#include "gpsr/gpsr.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/random-variable-stream.h"
#include "ns3/inet-socket-address.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/wifi-net-device.h"
#include "ns3/double.h"
#include <algorithm>
#include <limits>
#include "ns3/wifi-mac.h"        // For WifiMac (was forward-declared but needs full definition)
#define NS_LOG_APPEND_CONTEXT \
  if (m_ipv4) { std::clog << "[node " << m_ipv4->GetObject<Node> ()->GetId () << "] "; }

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Gpsr");

// UDP Port for GPSR control traffic
const uint32_t Gpsr::GPSR_PORT = 666;

// Maximum allowed jitter for hello messages
#define GPSR_MAX_JITTER (m_helloInterval.GetSeconds() / 2)

/**
 * \brief Tag for deferred route requests
 */
class GpsrDeferredRouteTag : public Tag
{
public:
  GpsrDeferredRouteTag() : Tag(), m_isCallFromL3(0) {}

  static TypeId GetTypeId() {
    static TypeId tid = TypeId("ns3::GpsrDeferredRouteTag").SetParent<Tag>();
    return tid;
  }

  TypeId GetInstanceTypeId() const { return GetTypeId(); }

  uint32_t GetSerializedSize() const { return sizeof(uint32_t); }

  void Serialize(TagBuffer i) const { i.WriteU32(m_isCallFromL3); }

  void Deserialize(TagBuffer i) { m_isCallFromL3 = i.ReadU32(); }

  void Print(std::ostream &os) const {
    os << "GpsrDeferredRouteTag: m_isCallFromL3 = " << m_isCallFromL3;
  }

  uint32_t m_isCallFromL3;
};

NS_OBJECT_ENSURE_REGISTERED(Gpsr);

  TypeId
  Gpsr::GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::Gpsr")
      .SetParent<Ipv4RoutingProtocol>()
      .SetGroupName("Routing")
      .AddConstructor<Gpsr>()
      .AddAttribute("HelloInterval", "HELLO messages emission interval",
                   TimeValue(Seconds(1)),
                   MakeTimeAccessor(&Gpsr::m_helloInterval),
                   MakeTimeChecker())
      .AddAttribute("MaxQueueLen", "Maximum length of the packet queue",
                   UintegerValue(64),
                   MakeUintegerAccessor(&Gpsr::m_maxQueueLen),
                   MakeUintegerChecker<uint32_t>())
      .AddAttribute("MaxQueueTime", "Maximum time a packet can stay in the queue",
                   TimeValue(Seconds(30)),
                   MakeTimeAccessor(&Gpsr::m_maxQueueTime),
                   MakeTimeChecker())
      .AddAttribute("PerimeterMode", "Enable perimeter mode for recovery",
                   BooleanValue(true),
                   MakeBooleanAccessor(&Gpsr::m_perimeterMode),
                   MakeBooleanChecker());
    return tid;
  }

Gpsr::Gpsr() :
  m_helloInterval(Seconds(1)),
  m_maxQueueLen(64),
  m_maxQueueTime(Seconds(30)),
  m_queue(m_maxQueueLen, m_maxQueueTime),
  m_perimeterMode(true)
{
    // Initialize but don't schedule yet
    m_helloTimer.SetFunction(&Gpsr::SendHello, this);
    m_queueTimer.SetFunction(&Gpsr::CheckQueue, this);
}

Gpsr::~Gpsr()
{
}

void
Gpsr::DoDispose()
{
  m_ipv4 = 0;
  Ipv4RoutingProtocol::DoDispose();
}

void
Gpsr::DoInitialize()
{
    m_helloTimer.Cancel();
    m_queueTimer.Cancel();

    // Schedule with a small delay to ensure the simulator is ready
    m_helloTimer.Schedule(MilliSeconds(100));
    m_queueTimer.Schedule(MilliSeconds(500));

    Ipv4RoutingProtocol::DoInitialize();
}

void
Gpsr::Start()
{
  m_queuedAddresses.clear();
  m_neighbors.Clear();
}

  bool
  Gpsr::RouteInput(Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                  const UnicastForwardCallback &ucb, const MulticastForwardCallback &mcb,
                  const LocalDeliverCallback &lcb, const ErrorCallback &ecb)
{
  NS_LOG_FUNCTION(this << p->GetUid() << header.GetDestination() << idev->GetAddress());

  if (m_socketAddresses.empty()) {
    NS_LOG_LOGIC("No GPSR interfaces");
    return false;
  }

  NS_ASSERT(m_ipv4 != nullptr);
  NS_ASSERT(p != nullptr);

  // Check if input device supports IP
  NS_ASSERT(m_ipv4->GetInterfaceForDevice(idev) >= 0);
  int32_t iif = m_ipv4->GetInterfaceForDevice(idev);
  Ipv4Address dst = header.GetDestination();
  Ipv4Address origin = header.GetSource();

  // Check for deferred routing
  GpsrDeferredRouteTag tag;
  if (p->PeekPacketTag(tag) && IsMyOwnAddress(origin)) {
    Ptr<Packet> packet = p->Copy();
    packet->RemovePacketTag(tag);
    DeferredRouteOutput(packet, header, ucb, ecb);
    return true;
  }

  // If this packet is for us, deliver it
  if (m_ipv4->IsDestinationAddress(dst, iif)) {
    NS_LOG_LOGIC("Local delivery to " << dst);
    lcb(p, header, iif);
    return true;
  }

  // Forward packet using greedy algorithm
  return ForwardingGreedy(p, header, ucb, ecb);
}

Ptr<Ipv4Route>
Gpsr::RouteOutput(Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{
  NS_LOG_FUNCTION(this << header << (oif ? oif->GetIfIndex() : 0));

  if (!p) {
    return LoopbackRoute(header, oif);
  }

  if (m_socketAddresses.empty()) {
    sockerr = Socket::ERROR_NOROUTETOHOST;
    NS_LOG_LOGIC("No GPSR interfaces");
    Ptr<Ipv4Route> route;
    return route;
  }

  sockerr = Socket::ERROR_NOTERROR;
  Ptr<Ipv4Route> route = Create<Ipv4Route>();
  Ipv4Address dst = header.GetDestination();

  Vector dstPos = Vector(1, 0, 0);
  // Find destination position

  // Create a tag
  GpsrDeferredRouteTag tag;
  if (!p->PeekPacketTag(tag)) {
    p->AddPacketTag(tag);
  }

  // Get my position
  Vector myPos;
  Ptr<MobilityModel> MM = m_ipv4->GetObject<MobilityModel>();
  if (MM) {
    myPos.x = MM->GetPosition().x;
    myPos.y = MM->GetPosition().y;
  }

  // Check if destination is a neighbor
  Ipv4Address nextHop;
  if (m_neighbors.IsNeighbor(dst)) {
    nextHop = dst;
  } else {
    nextHop = m_neighbors.BestNeighbor(dstPos, myPos);
  }

  if (nextHop != Ipv4Address::GetZero()) {
    route->SetDestination(dst);
    route->SetSource(header.GetSource());
    route->SetGateway(nextHop);

    // FIXME: This only works for a single interface
    route->SetOutputDevice(m_ipv4->GetNetDevice(1));

    NS_LOG_DEBUG("Found route to " << dst << " via " << nextHop);
    return route;
  } else {
    // No route found, defer for now
    return LoopbackRoute(header, oif);
  }
}

void
Gpsr::NotifyInterfaceUp(uint32_t interface)
{
  NS_LOG_FUNCTION(this << m_ipv4->GetAddress(interface, 0).GetLocal());

  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();
  if (l3->GetNAddresses(interface) > 1) {
    NS_LOG_WARN("GPSR does not work with more than one address per interface.");
  }

  Ipv4InterfaceAddress iface = l3->GetAddress(interface, 0);
  if (iface.GetLocal() == Ipv4Address("127.0.0.1")) {
    return;
  }

  // Create a socket to listen only on this interface
  Ptr<Socket> socket = Socket::CreateSocket(GetObject<Node>(),
                                         UdpSocketFactory::GetTypeId());
  NS_ASSERT(socket != nullptr);
  socket->SetRecvCallback(MakeCallback(&Gpsr::RecvGpsr, this));
  socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), GPSR_PORT));
  socket->BindToNetDevice(l3->GetNetDevice(interface));
  socket->SetAllowBroadcast(true);
  socket->SetAttribute("IpTtl", UintegerValue(1));
  m_socketAddresses.insert(std::make_pair(socket, iface));

  // Monitor for link failures using layer 2 feedback if possible
  Ptr<NetDevice> dev = m_ipv4->GetNetDevice(interface);
  Ptr<WifiNetDevice> wifi = dev->GetObject<WifiNetDevice>();
  if (wifi) {
    Ptr<WifiMac> mac = wifi->GetMac();
    if (mac) {
      mac->TraceConnectWithoutContext("TxErrHeader", m_neighbors.GetTxErrorCallback());
    }
  }
}

void
Gpsr::NotifyInterfaceDown(uint32_t interface)
{
  NS_LOG_FUNCTION(this << m_ipv4->GetAddress(interface, 0).GetLocal());

  // Disable layer 2 link state monitoring (if possible)
  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();
  Ptr<NetDevice> dev = l3->GetNetDevice(interface);
  Ptr<WifiNetDevice> wifi = dev->GetObject<WifiNetDevice>();
  if (wifi) {
    Ptr<WifiMac> mac = wifi->GetMac();
    if (mac) {
      mac->TraceDisconnectWithoutContext("TxErrHeader", m_neighbors.GetTxErrorCallback());
    }
  }

  // Close socket
  Ptr<Socket> socket = FindSocketWithInterfaceAddress(m_ipv4->GetAddress(interface, 0));
  NS_ASSERT(socket);
  socket->Close();
  m_socketAddresses.erase(socket);

  if (m_socketAddresses.empty()) {
    NS_LOG_LOGIC("No GPSR interfaces");
    m_neighbors.Clear();
    return;
  }
}

void
Gpsr::NotifyAddAddress(uint32_t interface, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION(this << " interface " << interface << " address " << address);

  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();
  if (!l3->IsUp(interface)) {
    return;
  }

  if (l3->GetNAddresses(interface) == 1) {
    Ipv4InterfaceAddress iface = l3->GetAddress(interface, 0);
    Ptr<Socket> socket = FindSocketWithInterfaceAddress(iface);
    if (!socket) {
      if (iface.GetLocal() == Ipv4Address("127.0.0.1")) {
        return;
      }

      // Create a socket to listen only on this interface
      Ptr<Socket> socket = Socket::CreateSocket(GetObject<Node>(),
                                             UdpSocketFactory::GetTypeId());
      NS_ASSERT(socket != nullptr);
      socket->SetRecvCallback(MakeCallback(&Gpsr::RecvGpsr, this));
      socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), GPSR_PORT));
      socket->BindToNetDevice(l3->GetNetDevice(interface));
      socket->SetAllowBroadcast(true);
      socket->SetAttribute("IpTtl", UintegerValue(1));
      m_socketAddresses.insert(std::make_pair(socket, iface));
    }
  } else {
    NS_LOG_LOGIC("GPSR does not work with more than one address per interface");
  }
}

void
Gpsr::NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION(this);

  Ptr<Socket> socket = FindSocketWithInterfaceAddress(address);
  if (socket) {
    m_socketAddresses.erase(socket);

    Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();
    if (l3->GetNAddresses(interface)) {
      Ipv4InterfaceAddress iface = l3->GetAddress(interface, 0);

      // Create a socket to listen only on this interface
      Ptr<Socket> socket = Socket::CreateSocket(GetObject<Node>(),
                                             UdpSocketFactory::GetTypeId());
      NS_ASSERT(socket != nullptr);
      socket->SetRecvCallback(MakeCallback(&Gpsr::RecvGpsr, this));
      socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), GPSR_PORT));
      socket->SetAllowBroadcast(true);
      socket->SetAttribute("IpTtl", UintegerValue(1));
      m_socketAddresses.insert(std::make_pair(socket, iface));
    }

    if (m_socketAddresses.empty()) {
      NS_LOG_LOGIC("No GPSR interfaces");
      m_neighbors.Clear();
      return;
    }
  }
}

void Gpsr::SetIpv4(Ptr<Ipv4> ipv4)
{
  NS_ASSERT(ipv4 != nullptr);
  NS_ASSERT(m_ipv4 == nullptr);

  m_ipv4 = ipv4;

  // Create lo device
  m_lo = m_ipv4->GetNetDevice(0);

  // Setup hello timer with jitter to avoid synchronization
  double min = 0.0;
  double max = GPSR_MAX_JITTER;
  Ptr<UniformRandomVariable> jitter = CreateObject<UniformRandomVariable>();
  jitter->SetAttribute("Min", DoubleValue(min));
  jitter->SetAttribute("Max", DoubleValue(max));

  m_helloTimer.Schedule(Seconds(jitter->GetValue(min, max)));

  Simulator::ScheduleNow(&Gpsr::Start, this);
}

void Gpsr::SendHello()
{
  NS_LOG_FUNCTION(this);

  // Get my position from mobility model
  Vector myPos;
  Ptr<MobilityModel> MM = m_ipv4->GetObject<MobilityModel>();

  if (MM) {
    myPos.x = MM->GetPosition().x;
    myPos.y = MM->GetPosition().y;
  } else {
    NS_LOG_WARN("No mobility model available");
    return;
  }

  // Create hello packet with my position
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin();
        j != m_socketAddresses.end(); ++j) {
    Ptr<Socket> socket = j->first;
    Ipv4InterfaceAddress iface = j->second;

    GpsrHelloHeader helloHeader((uint64_t)myPos.x, (uint64_t)myPos.y);

    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(helloHeader);
    GpsrTypeHeader typeHeader(GPSR_HELLO);
    packet->AddHeader(typeHeader);

    // Send to broadcast address
    Ipv4Address destination;
    if (iface.GetMask() == Ipv4Mask::GetOnes()) {
      destination = Ipv4Address("255.255.255.255");
    } else {
      destination = iface.GetBroadcast();
    }

    socket->SendTo(packet, 0, InetSocketAddress(destination, GPSR_PORT));
  }

  // Schedule next hello message with jitter
  double min = -1 * GPSR_MAX_JITTER;
  double max = GPSR_MAX_JITTER;
  Ptr<UniformRandomVariable> jitter = CreateObject<UniformRandomVariable>();
  jitter->SetAttribute("Min", DoubleValue(min));
  jitter->SetAttribute("Max", DoubleValue(max));

  m_helloTimer.Schedule(m_helloInterval + Seconds(jitter->GetValue(min, max)));
}

void
Gpsr::RecvGpsr(Ptr<Socket> socket)
{
  NS_LOG_FUNCTION(this << socket);

  Address sourceAddress;
  Ptr<Packet> packet = socket->RecvFrom(sourceAddress);

  GpsrTypeHeader typeHeader;
  packet->RemoveHeader(typeHeader);
  if (!typeHeader.IsValid()) {
    NS_LOG_DEBUG("GPSR message " << packet->GetUid() << " with unknown type received: "
                << typeHeader.Get() << ". Drop");
    return;
  }

  switch (typeHeader.Get()) {
    case GPSR_HELLO:
    {
      GpsrHelloHeader helloHeader;
      packet->RemoveHeader(helloHeader);

      Vector position;
      position.x = helloHeader.GetPositionX();
      position.y = helloHeader.GetPositionY();

      InetSocketAddress inetSourceAddr = InetSocketAddress::ConvertFrom(sourceAddress);
      Ipv4Address sender = inetSourceAddr.GetIpv4();

      // Update neighbor position
      UpdateRouteToNeighbor(sender, position);
      break;
    }
    default:
      NS_LOG_DEBUG("Unknown GPSR packet type: " << typeHeader.Get());
      break;
  }
}

void
Gpsr::UpdateRouteToNeighbor(Ipv4Address neighbor, Vector position)
{
  m_neighbors.AddEntry(neighbor, position);
}

bool
Gpsr::IsMyOwnAddress(Ipv4Address addr)
{
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin();
        j != m_socketAddresses.end(); ++j) {
    Ipv4InterfaceAddress iface = j->second;
    if (addr == iface.GetLocal()) {
      return true;
    }
  }
  return false;
}

void
Gpsr::DeferredRouteOutput(Ptr<const Packet> p, const Ipv4Header &header,
                          UnicastForwardCallback ucb, ErrorCallback ecb)
{
  NS_LOG_FUNCTION(this << p << header);

  // Check queue size
  if (m_queue.GetSize() == 0) {
    m_queueTimer.Cancel();
    m_queueTimer.Schedule(Seconds(0.5));
  }

  // Queue the packet
  GpsrQueueEntry newEntry(p, header, ucb, ecb);
  bool result = m_queue.Enqueue(newEntry);

  if (result) {
    m_queuedAddresses.insert(m_queuedAddresses.begin(), header.GetDestination());
    m_queuedAddresses.unique();
    NS_LOG_LOGIC("Add packet " << p->GetUid() << " to queue. Protocol " << (uint16_t)header.GetProtocol());
  }
}

void
Gpsr::CheckQueue()
{
  m_queueTimer.Cancel();

  std::list<Ipv4Address> toRemove;

  // Check if packets can be sent now
  for (std::list<Ipv4Address>::iterator i = m_queuedAddresses.begin(); i != m_queuedAddresses.end(); ++i) {
    if (SendPacketFromQueue(*i)) {
      toRemove.insert(toRemove.begin(), *i);
    }
  }

  // Remove addresses for packets that have been sent
  for (std::list<Ipv4Address>::iterator i = toRemove.begin(); i != toRemove.end(); ++i) {
    m_queuedAddresses.remove(*i);
  }

  // Reschedule timer if queue is not empty
  if (!m_queuedAddresses.empty()) {
    m_queueTimer.Schedule(Seconds(0.5));
  }
}

bool
Gpsr::SendPacketFromQueue(Ipv4Address dst)
{
  NS_LOG_FUNCTION(this << dst);

  GpsrQueueEntry queueEntry;
  bool recovery = false;

  // Get my position
  Vector myPos;
  Ptr<MobilityModel> MM = m_ipv4->GetObject<MobilityModel>();
  if (MM) {
    myPos.x = MM->GetPosition().x;
    myPos.y = MM->GetPosition().y;
  }

  // Check if destination is a neighbor
  Ipv4Address nextHop;
  if (m_neighbors.IsNeighbor(dst)) {
    nextHop = dst;
  } else {
    // Find destination position and then best neighbor
    Vector dstPos = Vector(1, 0, 0);
    nextHop = m_neighbors.BestNeighbor(dstPos, myPos);

    if (nextHop == Ipv4Address::GetZero()) {
      NS_LOG_LOGIC("No next hop found, going to recovery mode");
      recovery = true;
    }
  }

  if (recovery && m_perimeterMode) {
    // Handle recovery mode
    while (m_queue.Dequeue(dst, queueEntry)) {
      RecoveryMode(dst, ConstCast<Packet>(queueEntry.GetPacket()), queueEntry.GetUnicastForwardCallback(), queueEntry.GetIpv4Header());
    }
    return true;
  } else if (nextHop != Ipv4Address::GetZero()) {
    // Create route
    Ptr<Ipv4Route> route = Create<Ipv4Route>();
    route->SetDestination(dst);
    route->SetGateway(nextHop);
    route->SetOutputDevice(m_ipv4->GetNetDevice(1));

    // Send all packets for this destination
    while (m_queue.Dequeue(dst, queueEntry)) {
      Ptr<Packet> p = ConstCast<Packet>(queueEntry.GetPacket());
      UnicastForwardCallback ucb = queueEntry.GetUnicastForwardCallback();
      Ipv4Header header = queueEntry.GetIpv4Header();

      route->SetSource(header.GetSource());
      ucb(route, p, header);
    }
    return true;
  }

  return false;
}

bool
Gpsr::ForwardingGreedy(Ptr<const Packet> p, const Ipv4Header &header,
                       UnicastForwardCallback ucb, ErrorCallback ecb)
{
  NS_LOG_FUNCTION(this);

  Ipv4Address dst = header.GetDestination();

  // Get my position
  Vector myPos;
  Ptr<MobilityModel> MM = m_ipv4->GetObject<MobilityModel>();
  if (MM) {
    myPos.x = MM->GetPosition().x;
    myPos.y = MM->GetPosition().y;
  }

  // Update position table
  m_neighbors.Purge();

  // Check if destination is a neighbor
  Ipv4Address nextHop;
  if (m_neighbors.IsNeighbor(dst)) {
    nextHop = dst;
  } else {
    // Try greedy forwarding
    Vector dstPos = Vector(1, 0, 0);
    nextHop = m_neighbors.BestNeighbor(dstPos, myPos);
  }

  if (nextHop != Ipv4Address::GetZero()) {
    // Create route
    Ptr<Ipv4Route> route = Create<Ipv4Route>();
    route->SetDestination(dst);
    route->SetGateway(nextHop);
    route->SetSource(header.GetSource());
    route->SetOutputDevice(m_ipv4->GetNetDevice(1));

    // Forward packet
    NS_LOG_LOGIC(route->GetOutputDevice() << " forwarding to " << dst << " from "
                << header.GetSource() << " through " << route->GetGateway() << " packet " << p->GetUid());

    ucb(route, p, header);
    return true;
  } else if (m_perimeterMode) {
    // Enter recovery mode
    NS_LOG_LOGIC("Entering recovery mode for packet to " << dst);
    Ptr<Packet> packet = p->Copy();
    RecoveryMode(dst, packet, ucb, header);
    return true;
  }

  return false;
}

void
Gpsr::RecoveryMode(Ipv4Address dst, Ptr<Packet> p, UnicastForwardCallback ucb, Ipv4Header header)
{
  NS_LOG_FUNCTION(this << dst);

  // Get my position
  Vector myPos;
  Ptr<MobilityModel> MM = m_ipv4->GetObject<MobilityModel>();
  if (MM) {
    myPos.x = MM->GetPosition().x;
    myPos.y = MM->GetPosition().y;
  }

  // Add position header in recovery mode
  GpsrPositionHeader posHeader;
  posHeader.SetDstPositionX(1); // FIXME: Set real destination position
  posHeader.SetDstPositionY(1);
  posHeader.SetUpdated(0);
  posHeader.SetRecPositionX(myPos.x);
  posHeader.SetRecPositionY(myPos.y);
  posHeader.SetInRecovery(1);
  posHeader.SetLastPositionX(myPos.x);
  posHeader.SetLastPositionY(myPos.y);

  p->AddHeader(posHeader);

  // Use right-hand rule to find next hop
  Vector prevPos = Vector(0, 0, 0); // FIXME: Should be previous hop position
  Ipv4Address nextHop = m_neighbors.BestAngle(prevPos, myPos);

  if (nextHop == Ipv4Address::GetZero()) {
    NS_LOG_LOGIC("No neighbors found in recovery mode, dropping packet");
    return;
  }

  Ptr<Ipv4Route> route = Create<Ipv4Route>();
  route->SetDestination(dst);
  route->SetGateway(nextHop);
  route->SetSource(header.GetSource());
  route->SetOutputDevice(m_ipv4->GetNetDevice(1));

  NS_LOG_LOGIC("Recovery mode forwarding to " << dst << " via " << nextHop);
  ucb(route, p, header);
}

Ptr<Ipv4Route>
Gpsr::LoopbackRoute(const Ipv4Header &header, Ptr<NetDevice> oif)
{
  NS_LOG_FUNCTION(this << header);

  Ptr<Ipv4Route> route = Create<Ipv4Route>();
  route->SetDestination(header.GetDestination());

  // Get first available interface address to use as source
  Ipv4Address src = Ipv4Address::GetZero();
  std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin();

  if (oif) {
    // Find address on the requested output interface
    for (; j != m_socketAddresses.end(); ++j) {
      Ipv4Address addr = j->second.GetLocal();
      int32_t interface = m_ipv4->GetInterfaceForAddress(addr);
      if (oif == m_ipv4->GetNetDevice(static_cast<uint32_t>(interface))) {
        src = addr;
        break;
      }
    }
  } else {
    // Use first available address
    if (j != m_socketAddresses.end()) {
      src = j->second.GetLocal();
    }
  }

  NS_ASSERT_MSG(src != Ipv4Address::GetZero(), "No available GPSR interfaces");

  route->SetSource(src);
  route->SetGateway(Ipv4Address("127.0.0.1"));
  route->SetOutputDevice(m_lo);

  return route;
}

Ptr<Socket>
Gpsr::FindSocketWithInterfaceAddress(Ipv4InterfaceAddress addr) const
{
  NS_LOG_FUNCTION(this << addr);

  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin();
        j != m_socketAddresses.end(); ++j) {
    Ptr<Socket> socket = j->first;
    Ipv4InterfaceAddress iface = j->second;
    if (iface == addr) {
      return socket;
    }
  }

  Ptr<Socket> socket;
  return socket;
}

void
Gpsr::PrintRoutingTable(Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{
  std::ostream* os = stream->GetStream();
  *os << "Node: " << m_ipv4->GetObject<Node>()->GetId()
      << ", Time: " << Now().As(unit)
      << ", Local time: " << GetObject<Node>()->GetLocalTime().As(unit)
      << ", GPSR Routing table" << std::endl;

  *os << "Destination\tGateway\tInterface\tFlag\tExpire" << std::endl;

  // Print neighbors from position table
  // This is a placeholder - actual implementation would print neighbor info

  *os << std::endl;
}

int
Gpsr::GetProtocolNumber(void) const
{
  return GPSR_PORT;
}

void
Gpsr::SetDownTarget(IpL4Protocol::DownTargetCallback callback)
{
  m_downTarget = callback;
}

IpL4Protocol::DownTargetCallback
Gpsr::GetDownTarget(void) const
{
  return m_downTarget;
}

  void
  Gpsr::AddHeaders(Ptr<Packet> p, Ipv4Address source, Ipv4Address destination,
                  uint8_t protocol, Ptr<Ipv4Route> route)
{
  NS_LOG_FUNCTION(this << p << source << destination << (uint32_t)protocol);

  // Get my position from mobility model
  Vector myPos;
  Ptr<MobilityModel> MM = m_ipv4->GetObject<MobilityModel>();

  if (MM) {
    myPos.x = MM->GetPosition().x;
    myPos.y = MM->GetPosition().y;
  } else {
    NS_LOG_WARN("No mobility model attached to the node");
    return;
  }

  // Add position header for GPSR routing
  GpsrPositionHeader posHeader;
  posHeader.SetDstPositionX(1); // Placeholder
  posHeader.SetDstPositionY(1); // Placeholder
  posHeader.SetLastPositionX(myPos.x);
  posHeader.SetLastPositionY(myPos.y);

  p->AddHeader(posHeader);

  // Call the original down target with the modified packet
  if (!m_downTarget.IsNull()) {
    m_downTarget(p, source, destination, protocol, route);
  }
}

} // namespace ns3