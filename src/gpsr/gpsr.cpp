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
#include "ns3/node-list.h"      // Added for NodeList access
#include "ns3/arp-cache.h"      // Added for ArpCache access
#include "ns3/loopback-net-device.h" // Added for LoopbackNetDevice type

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
  m_perimeterMode(true),
  m_uniformRandomVariable(CreateObject<UniformRandomVariable>())
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

    // Get the node pointer if not already available
    Ptr<Node> node = this->GetObject<Node>();
    if (node)
    {
        // Initialize random variable stream specific to this node/protocol instance
        m_uniformRandomVariable->SetStream(node->GetId() + this->GetProtocolNumber());
    }
    else
    {
        // Fallback if node context is not available yet (should ideally not happen here)
        m_uniformRandomVariable->SetStream(Simulator::GetContext());
    }


    // Schedule with a small random jitter to avoid synchronization
    Time helloJitter = MicroSeconds(m_uniformRandomVariable->GetInteger(0, 10000)); // 0-10 ms jitter
    Time queueJitter = MicroSeconds(m_uniformRandomVariable->GetInteger(0, 10000)); // 0-10 ms jitter

    m_helloTimer.Schedule(MilliSeconds(100) + helloJitter);
    m_queueTimer.Schedule(MilliSeconds(500) + queueJitter);
    NS_LOG_INFO("Scheduled initial Hello Timer with " << helloJitter.GetMicroSeconds() << "us jitter.");
    NS_LOG_INFO("Scheduled initial Queue Timer with " << queueJitter.GetMicroSeconds() << "us jitter.");

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

  // Ignore broadcast packets for routing table lookup/forwarding
  if (header.GetDestination().IsBroadcast()) {
      NS_LOG_LOGIC("RouteInput: Ignoring broadcast packet.");
      return false; // Let other potential handlers (like local delivery if applicable) deal with it
  }

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

    // Ignore broadcast packets for routing table lookup/forwarding
    if (dst.IsBroadcast()) {
        NS_LOG_LOGIC("RouteOutput: Ignoring broadcast packet.");
        // Cannot route broadcast, return null route.
        // Let link layer handle L2 broadcast if needed.
        sockerr = Socket::ERROR_NOROUTETOHOST; // Indicate why null route is returned
        return Ptr<Ipv4Route>(); // Return null Ptr
    }

    // --- Start Destination Position Lookup ---
    Vector dstPos;
    bool dstPosFound = false;
    Ptr<MobilityModel> dstMobility = nullptr;

    // Iterate through all nodes in the simulation to find the destination node
    for (uint32_t i = 0; i < NodeList::GetNNodes(); ++i)
    {
        Ptr<Node> node = NodeList::GetNode(i);
        Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
        if (ipv4)
        {
            // Check all interfaces of the node
            for (uint32_t j = 0; j < ipv4->GetNInterfaces(); ++j)
            {
                // Check all addresses on the interface
                for (uint32_t k = 0; k < ipv4->GetNAddresses(j); ++k)
                {
                    Ipv4InterfaceAddress ifaceAddr = ipv4->GetAddress(j, k);
                    if (ifaceAddr.GetLocal() == dst)
                    {
                        dstMobility = node->GetObject<MobilityModel>();
                        if (dstMobility)
                        {
                            dstPos = dstMobility->GetPosition();
                            dstPosFound = true;
                            NS_LOG_LOGIC("Found destination node " << node->GetId() << " at position " << dstPos << " for IP " << dst);
                        } else {
                            NS_LOG_WARN("Destination node " << node->GetId() << " found for IP " << dst << " but it has no mobility model.");
                        }
                        goto end_node_search_routeoutput; // Exit loops once found
                    }
                }
            }
        }
    }
    end_node_search_routeoutput:; // Label for goto

    if (!dstPosFound)
    {
        NS_LOG_WARN("Could not find position for destination IP " << dst << ". Deferring packet.");
        sockerr = Socket::ERROR_NOROUTETOHOST; // Indicate why loopback is returned
        return LoopbackRoute(header, oif);
    }
    // --- End Destination Position Lookup ---


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
    } else {
      // Handle missing mobility model if needed, maybe return LoopbackRoute?
      NS_LOG_WARN("RouteOutput: Node has no mobility model.");
      myPos = Vector(0,0,0); // Fallback position? Or error out?
      sockerr = Socket::ERROR_NOROUTETOHOST; 
      return LoopbackRoute(header, oif);
    }

    // Check if destination is a neighbor
    Ipv4Address nextHop;
    if (m_neighbors.IsNeighbor(dst)) {
      nextHop = dst;
    } else {
      nextHop = m_neighbors.BestNeighbor(dstPos, myPos);
    }

    if (nextHop != Ipv4Address::GetZero()) {
      // Find the correct output device for the next hop
      int32_t interfaceIndex = m_ipv4->GetInterfaceForAddress(nextHop);
      Ptr<NetDevice> outputDevice = nullptr;
      if (interfaceIndex < 0) {
          NS_LOG_WARN("Could not find interface for next hop address " << nextHop << ". Cannot create route.");
          sockerr = Socket::ERROR_NOROUTETOHOST;
          return LoopbackRoute(header, oif); // Cannot route without interface
      } else {
          outputDevice = m_ipv4->GetNetDevice(static_cast<uint32_t>(interfaceIndex));
          if (!outputDevice) {
               NS_LOG_ERROR("Could not get NetDevice for interface index " << interfaceIndex << ". Cannot create route.");
               sockerr = Socket::ERROR_NOROUTETOHOST;
               return LoopbackRoute(header, oif);
          }
      }

      // Fixed: Create route object for ucb
      route->SetDestination(dst);
      route->SetSource(header.GetSource()); // Source needed for the route object
      route->SetGateway(nextHop);
      route->SetOutputDevice(outputDevice); 

      NS_LOG_DEBUG("Found route to " << dst << " via " << nextHop << " on interface " << interfaceIndex);
      return route;
    } else {
      // No route found, defer for now
      NS_LOG_LOGIC("RouteOutput: No greedy route found for " << dst << ", deferring.");
      sockerr = Socket::ERROR_NOROUTETOHOST; // Indicate reason for loopback
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
  // Bind to Any Address on the specific port, don't bind to device explicitly
  socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), GPSR_PORT));
  socket->SetAllowBroadcast(true);
  socket->SetAttribute("IpTtl", UintegerValue(1));

  // Add the socket to the list of sockets for this interface
  m_socketAddresses.insert(std::make_pair(socket, iface));

  // Check if the device supports MAC level TX error callback
  // Removed: TxError handling is reverted for now
  /*
  Ptr<NetDevice> dev = l3->GetNetDevice(interface);
  Ptr<WifiNetDevice> wifiDevice = DynamicCast<WifiNetDevice>(dev);
  if (wifiDevice) {
    Ptr<WifiMac> mac = wifiDevice->GetMac();
    if (mac) {
      // Fixed: Removed call to non-existent GetTxErrorCallback
      // mac->TraceConnectWithoutContext("TxErrHeader", m_neighbors.GetTxErrorCallback());
    }
  }
  */
}

void
Gpsr::NotifyInterfaceDown(uint32_t interface)
{
  NS_LOG_FUNCTION(this << interface);

  // Remove the socket associated with this interface
  Ptr<Socket> socket = FindSocketWithInterfaceAddress(m_ipv4->GetAddress(interface, 0));
  if (socket) {
    socket->Close();
    m_socketAddresses.erase(socket);
  }

  // Remove trace callbacks if necessary
  // Removed: TxError handling is reverted for now
  /*
  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();
  Ptr<NetDevice> dev = l3->GetNetDevice(interface);
  Ptr<WifiNetDevice> wifiDevice = DynamicCast<WifiNetDevice>(dev);
  if (wifiDevice) {
    Ptr<WifiMac> mac = wifiDevice->GetMac();
    if (mac) {
      // Fixed: Removed call to non-existent GetTxErrorCallback
      // mac->TraceDisconnectWithoutContext("TxErrHeader", m_neighbors.GetTxErrorCallback());
    }
  }
  */
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
      // Bind to Any Address on the specific port, don't bind to device explicitly
      socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), GPSR_PORT));
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
      // Bind to Any Address on the specific port, don't bind to device explicitly
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

  // Get the Ipv4L3Protocol object to call SetupLoopback
  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();
  NS_ASSERT(l3 != nullptr);

  // Create Loopback device
  // Removed private call: l3->SetupLoopback(); 
  // Assuming loopback device (index 0) is setup by InternetStackHelper
  m_lo = m_ipv4->GetNetDevice(0); 
  NS_ASSERT(m_lo != nullptr);
  // Verify it's actually loopback (requires include)
  if (m_lo->GetInstanceTypeId() != LoopbackNetDevice::GetTypeId()) {
      NS_LOG_WARN("Device at index 0 is not LoopbackNetDevice?");
  }

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

      GpsrHelloHeader helloHeader(myPos.x, myPos.y);

      Ptr<Packet> packet = Create<Packet>();
      GpsrTypeHeader typeHeader(GPSR_HELLO);
      packet->AddHeader(typeHeader);
      packet->AddHeader(helloHeader);

      // Send to broadcast address
      Ipv4Address destination;
      if (iface.GetMask() == Ipv4Mask::GetOnes()) {
        destination = Ipv4Address("255.255.255.255");
      } else {
        destination = iface.GetBroadcast();
      }

      // Removed excessive log message here
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
    InetSocketAddress inetSourceAddr = InetSocketAddress::ConvertFrom(sourceAddress);
    Ipv4Address sender = inetSourceAddr.GetIpv4();
    NS_LOG_INFO("RecvGpsr: Received packet of size " << packet->GetSize() << " from " << sender);

    GpsrTypeHeader typeHeader;
    packet->RemoveHeader(typeHeader);

    // Add specific logging right after removing the type header
    NS_LOG_INFO("RecvGpsr: After RemoveHeader(typeHeader): IsValid=" 
                << typeHeader.IsValid() << ", TypeValue=" << typeHeader.Get() 
                << " from " << sender);

    if (!typeHeader.IsValid()) {
      NS_LOG_DEBUG("RecvGpsr: Invalid GPSR packet received from " << sender << " (exiting RecvGpsr)");
      return;
    }

    NS_LOG_DEBUG("RecvGpsr: Packet type " << typeHeader.Get() << " from " << sender);

    // Add another log right before the check
    NS_LOG_INFO("RecvGpsr: Checking if TypeValue (" << typeHeader.Get() << ") == GPSR_HELLO (" << GPSR_HELLO << ")"); 

    // Simplify check: Store value locally first
    GpsrMessageType receivedType = typeHeader.Get();
    // Add explicit logging to check the received type value
    NS_LOG_INFO("RecvGpsr: Comparing receivedType (" << static_cast<int>(receivedType) 
                << ") with GPSR_HELLO (" << static_cast<int>(GPSR_HELLO) << ")");

    if (receivedType == GPSR_HELLO) { // Compare local variable
      NS_LOG_INFO("RecvGpsr: Entered HELLO processing block."); // Re-added info log
      GpsrHelloHeader helloHeader;
      packet->RemoveHeader(helloHeader);
      Vector senderPos;
      // Assuming coordinates are now double in the header
      senderPos.x = helloHeader.GetPositionX(); 
      senderPos.y = helloHeader.GetPositionY();
      senderPos.z = 0; // Assuming 2D

      // Log HELLO details
      NS_LOG_INFO("RecvGpsr: Processing HELLO from " << sender << " at position " << senderPos);

      // Update neighbor table
      UpdateRouteToNeighbor(sender, senderPos);

    } else {
      // Added log for non-HELLO case
      NS_LOG_INFO("RecvGpsr: Packet type was NOT HELLO. Type value: " << static_cast<int>(receivedType)); 
    }
  }

void
Gpsr::UpdateRouteToNeighbor(Ipv4Address neighbor, Vector position)
{
  NS_LOG_FUNCTION(this << neighbor << position);
  NS_LOG_INFO("UpdateRouteToNeighbor: Adding/Updating entry for " << neighbor << " at " << position);
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
    m_neighbors.Purge(); 

    GpsrQueueEntry queueEntry;
    if (!m_queue.Dequeue(dst, queueEntry)) {
      NS_LOG_DEBUG("SendPacketFromQueue: No packet for " << dst << " found in queue.");
      return false;
    }

    NS_LOG_DEBUG("SendPacketFromQueue: Dequeued packet UID " << queueEntry.GetPacket()->GetUid() << " for " << dst);

    // Get My Position
    Ptr<MobilityModel> mm = m_ipv4->GetObject<MobilityModel>();
    Vector myPos = mm ? mm->GetPosition() : Vector(0,0,0);
    if (!mm) {
      NS_LOG_WARN("SendPacketFromQueue: Node has no mobility model. Cannot route packet UID " << queueEntry.GetPacket()->GetUid() << ". Dropping.");
      DropPacketWithDst(dst, "No mobility model");
      return false;
    }

    // Get Destination Position (Oracle Lookup)
    Vector dstPos;
    bool dstPosFound = false;
    // Iterate through all nodes
    for (uint32_t i = 0; i < NodeList::GetNNodes(); ++i)
    {
        Ptr<Node> node = NodeList::GetNode(i);
        Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
        if (ipv4) {
            for (uint32_t j = 0; j < ipv4->GetNInterfaces(); ++j) {
                for (uint32_t k = 0; k < ipv4->GetNAddresses(j); ++k) {
                    if (ipv4->GetAddress(j, k).GetLocal() == dst) {
                        Ptr<MobilityModel> dstMobility = node->GetObject<MobilityModel>();
                        if (dstMobility) {
                            dstPos = dstMobility->GetPosition();
                            dstPosFound = true;
                            NS_LOG_LOGIC("SendPacketFromQueue: Found dstPos " << dstPos << " for " << dst);
                        } else {
                             NS_LOG_WARN("SendPacketFromQueue: Dest node " << node->GetId() << " for IP " << dst << " has no mobility model.");
                        }
                        goto end_node_search_sendfromqueue;
                    }
                }
            }
        }
    }
    end_node_search_sendfromqueue:;

    if (!dstPosFound) {
        NS_LOG_WARN("SendPacketFromQueue: Could not find position for destination IP " << dst << ". Dropping packets.");
        DropPacketWithDst(dst, "Destination position unknown"); // Drop all packets for this dest
        return false;
    }
    // --- End Destination Position Lookup ---

    // Find Best Neighbor (Greedy)
    Ipv4Address nextHop = m_neighbors.BestNeighbor(dstPos, myPos);

    if (nextHop == Ipv4Address::GetZero()) {
      // Greedy failed
      NS_LOG_DEBUG("SendPacketFromQueue: No greedy next hop found for " << dst << ", checking perimeter mode.");
      
      if (m_perimeterMode)
      {
          NS_LOG_LOGIC("SendPacketFromQueue: Entering RecoveryMode for packet UID " << queueEntry.GetPacket()->GetUid());
          // Packet from queue is assumed not to have a position header yet.
          // Create a copy to add the header.
          Ptr<Packet> packetCopy = ConstCast<Packet>(queueEntry.GetPacket())->Copy();
          
          GpsrPositionHeader recoveryHeader;
          // Populate the header for recovery mode initiation
          recoveryHeader.SetDstPositionX(dstPos.x);
          recoveryHeader.SetDstPositionY(dstPos.y);
          recoveryHeader.SetRecPositionX(myPos.x); // Position where recovery starts (here)
          recoveryHeader.SetRecPositionY(myPos.y);
          recoveryHeader.SetPrevPositionX(myPos.x); // First hop on perimeter is this node
          recoveryHeader.SetPrevPositionY(myPos.y);
          recoveryHeader.SetRecoveryFlag(true);
          packetCopy->AddHeader(recoveryHeader);

          // Call RecoveryMode with the packet COPY and info from queue entry
          RecoveryMode(dst, packetCopy, queueEntry.GetUnicastForwardCallback(), queueEntry.GetIpv4Header(), queueEntry.GetErrorCallback());
      } else {
          // Recovery needed but disabled, drop the packet
          NS_LOG_DEBUG("SendPacketFromQueue: Greedy failed, recovery disabled. Dropping packet UID " << queueEntry.GetPacket()->GetUid());
          queueEntry.GetErrorCallback()(queueEntry.GetPacket(), queueEntry.GetIpv4Header(), Socket::ERROR_NOROUTETOHOST);
          // We already dequeued it, so just return true (packet processed/dropped).
      }

    } else {
      // Greedy Succeeded
      NS_LOG_DEBUG("SendPacketFromQueue: Found greedy next hop " << nextHop << " for " << dst);
      // Send using the found greedy route
      int32_t interfaceIndex = m_ipv4->GetInterfaceForAddress(nextHop);
      Ptr<NetDevice> oif = nullptr;
      if (interfaceIndex < 0) {
          NS_LOG_WARN ("SendPacketFromQueue: Could not find Output Interface for next hop " << nextHop << ". Packet UID " << queueEntry.GetPacket()->GetUid() << " dropped.");
          queueEntry.GetErrorCallback()(queueEntry.GetPacket(), queueEntry.GetIpv4Header(), Socket::ERROR_NOROUTETOHOST);
          return false; // Return false as this specific packet failed
      } else {
          oif = m_ipv4->GetNetDevice(static_cast<uint32_t>(interfaceIndex));
           if (!oif) {
              NS_LOG_ERROR ("SendPacketFromQueue: Could not get Output NetDevice for interface index " << interfaceIndex << ". Packet UID " << queueEntry.GetPacket()->GetUid() << " dropped.");
              queueEntry.GetErrorCallback()(queueEntry.GetPacket(), queueEntry.GetIpv4Header(), Socket::ERROR_NOROUTETOHOST);
              return false; // Return false as this specific packet failed
          }
      }

      // Create route and call UCB
      Ptr<Ipv4Route> route = Create<Ipv4Route>();
      route->SetDestination(dst);
      route->SetSource(queueEntry.GetIpv4Header().GetSource());
      route->SetGateway(nextHop);
      route->SetOutputDevice(oif);
      NS_LOG_LOGIC("SendPacketFromQueue: Calling UCB for Dst=" << dst << " NextHop=" << nextHop);
      queueEntry.GetUnicastForwardCallback()(route, queueEntry.GetPacket(), queueEntry.GetIpv4Header());
    }

    // Check if there are more packets for this destination in the queue
    if (m_queue.Find(dst)) {
        NS_LOG_LOGIC("SendPacketFromQueue: More packets queued for " << dst << ". Scheduling CheckQueue.");
        m_queueTimer.Schedule(Time(0)); // Check queue again immediately
    }

    return true; // Indicate packet was processed (sent, recovered, or dropped)
  }

// Helper function to drop all packets for a destination and call error callback
void
Gpsr::DropPacketWithDst(Ipv4Address dst, std::string reason)
{
    NS_LOG_FUNCTION(this << dst << reason);
    GpsrQueueEntry queueEntry;
    while (m_queue.Dequeue(dst, queueEntry))
    {
        NS_LOG_LOGIC("Dropping queued packet UID " << queueEntry.GetPacket()->GetUid() << " for " << dst << " Reason: " << reason);
        queueEntry.GetErrorCallback()(queueEntry.GetPacket(), queueEntry.GetIpv4Header(), Socket::ERROR_NOROUTETOHOST);
    }
}

bool
Gpsr::ForwardingGreedy(Ptr<const Packet> p, const Ipv4Header &header,
                       UnicastForwardCallback ucb, ErrorCallback ecb)
{
  NS_LOG_FUNCTION(this << p->GetUid() << header.GetDestination());
  Ipv4Address dst = header.GetDestination();
  Ptr<MobilityModel> mm = m_ipv4->GetObject<MobilityModel>();
  Vector myPos = mm ? mm->GetPosition() : Vector(0,0,0);

  if (!mm) {
      NS_LOG_WARN("ForwardingGreedy: Node has no mobility model, cannot perform greedy routing.");
      ecb(p, header, Socket::ERROR_NOROUTETOHOST);
      return false;
  }

  // Get destination position from header OR use Oracle lookup
  Vector dstPos;
  GpsrPositionHeader gpsrHeader;
  bool hasPositionHeader = p->PeekHeader(gpsrHeader);

  if (hasPositionHeader) {
      // If header exists, use position from it (might have been added by source or previous hop)
      dstPos = Vector(gpsrHeader.GetDstPositionX(), gpsrHeader.GetDstPositionY(), 0); 
      // TODO: Ensure GetDstPositionX/Y exist.
      NS_LOG_LOGIC("ForwardingGreedy: Using dstPos from existing header: " << dstPos);
  } else {
      // If no header, perform Oracle lookup (consistent with RouteOutput)
      NS_LOG_LOGIC("ForwardingGreedy: No position header, performing Oracle lookup for " << dst);
      bool dstPosFound = false;
      Ptr<MobilityModel> dstMobility = nullptr;
      // Simplified lookup - assumes destination node exists and has mobility
      // A full lookup loop like in RouteOutput might be needed for robustness
      for (uint32_t i = 0; i < NodeList::GetNNodes(); ++i) {
          Ptr<Node> node = NodeList::GetNode(i);
          Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
          if (ipv4) {
              for (uint32_t j = 0; j < ipv4->GetNInterfaces(); ++j) {
                  for (uint32_t k = 0; k < ipv4->GetNAddresses(j); ++k) {
                      if (ipv4->GetAddress(j, k).GetLocal() == dst) {
                          dstMobility = node->GetObject<MobilityModel>();
                          if (dstMobility) {
                              dstPos = dstMobility->GetPosition();
                              dstPosFound = true;
                              NS_LOG_LOGIC("Found destination node " << node->GetId() << " at position " << dstPos);
                          } else {
                               NS_LOG_WARN("Dest node " << node->GetId() << " for IP " << dst << " has no mobility model.");
                          }
                          goto end_node_search_forwarding; 
                      }
                  }
              }
          }
      }
      end_node_search_forwarding:;
      if (!dstPosFound) {
          NS_LOG_WARN("Could not find position for destination IP " << dst << ". Packet dropped.");
          ecb(p, header, Socket::ERROR_NOROUTETOHOST);
          return false;
      }
  }

  // Find best neighbor using greedy approach
  Ipv4Address nextHop = m_neighbors.BestNeighbor(dstPos, myPos);

  if (nextHop != Ipv4Address::GetZero()) {
    NS_LOG_DEBUG("ForwardingGreedy: Found next hop " << nextHop << " for dst " << dst);
    int32_t interfaceIndex = m_ipv4->GetInterfaceForAddress(nextHop);
    Ptr<NetDevice> oif = nullptr;
    if (interfaceIndex < 0) {
        NS_LOG_WARN ("ForwardingGreedy: Could not find Output Interface for next hop " << nextHop);
        ecb(p, header, Socket::ERROR_NOROUTETOHOST);
        return false;
    } else {
        oif = m_ipv4->GetNetDevice(static_cast<uint32_t>(interfaceIndex));
         if (!oif) {
            NS_LOG_ERROR ("ForwardingGreedy: Could not get Output NetDevice for interface index " << interfaceIndex);
            ecb(p, header, Socket::ERROR_NOROUTETOHOST);
            return false;
        }
    }

    // Fixed: Create Ipv4Route object and call ucb correctly
    Ptr<Ipv4Route> route = Create<Ipv4Route>();
    route->SetDestination(dst);
    route->SetSource(header.GetSource());
    route->SetGateway(nextHop);
    route->SetOutputDevice(oif);
    NS_LOG_LOGIC("ForwardingGreedy: Calling UCB for Dst=" << dst << " NextHop=" << nextHop);
    ucb(route, p, header);
    return true;

  } else if (m_perimeterMode) {
    // Greedy failed, enter Recovery / Perimeter mode if not already in it
    NS_LOG_DEBUG("ForwardingGreedy: No closer neighbor found for " << dst << ". Checking recovery status.");

    GpsrPositionHeader existingHeader;
    bool alreadyInRecovery = p->PeekHeader(existingHeader) && existingHeader.GetRecoveryFlag();

    if (alreadyInRecovery) {
        // Packet is already in recovery mode, just forward using RecoveryMode logic
        NS_LOG_DEBUG("ForwardingGreedy: Packet already in recovery. Passing to RecoveryMode.");
        // Need to pass the *original* packet (p), not a copy, as header should exist.
        // RecoveryMode itself will make copies if needed for header updates.
        RecoveryMode(dst, ConstCast<Packet>(p), ucb, header, ecb); 
    } else {
        // Packet is not in recovery, initiate recovery mode
        NS_LOG_DEBUG("ForwardingGreedy: Initiating recovery mode.");
        // Create a *mutable* copy of the packet to add the header
        Ptr<Packet> packetCopy = p->Copy();

        // Remove any potentially stale/incorrect position header before adding the new one.
        // This is safer if the packet somehow had a non-recovery position header.
        GpsrPositionHeader removedHeader; 
        packetCopy->RemoveHeader(removedHeader); 

        // Populate the header for recovery mode initiation
        GpsrPositionHeader recoveryHeader;
        recoveryHeader.SetDstPositionX(dstPos.x);
        recoveryHeader.SetDstPositionY(dstPos.y);
        recoveryHeader.SetRecPositionX(myPos.x); // Position where recovery starts
        recoveryHeader.SetRecPositionY(myPos.y);
        recoveryHeader.SetPrevPositionX(myPos.x); // First hop on perimeter is this node
        recoveryHeader.SetPrevPositionY(myPos.y);
        recoveryHeader.SetRecoveryFlag(true);
        // TODO: Ensure GpsrPositionHeader has these fields and Setters.

        packetCopy->AddHeader(recoveryHeader);

        // Call RecoveryMode function (passing the *copy* with the header)
        RecoveryMode(dst, packetCopy, ucb, header, ecb); // Note: ecb is now passed to RecoveryMode
    }
    return true; // Packet is handled by RecoveryMode (either forwarded or dropped)

  } else {
    // Greedy failed and Perimeter mode is disabled
    NS_LOG_DEBUG("ForwardingGreedy: No closer neighbor and no recovery mode for dst " << dst << ". Packet dropped.");
    ecb(p, header, Socket::ERROR_NOROUTETOHOST);
    return false;
  }
}

void Gpsr::RecoveryMode(Ipv4Address dst, Ptr<Packet> p, UnicastForwardCallback ucb, Ipv4Header header, const ErrorCallback &ecb)
{
    NS_LOG_FUNCTION(this << dst << p->GetUid());

    // Get my position
    Ptr<MobilityModel> MM = m_ipv4->GetObject<MobilityModel>();
    Vector myPos = MM ? MM->GetPosition() : Vector(0,0,0);
    if (!MM) {
        NS_LOG_WARN("RecoveryMode: Node has no mobility model, cannot perform perimeter routing.");
        // Fixed: Use correct error code
        ecb(p, header, Socket::ERROR_NOROUTETOHOST);
        return;
    }

    // Get Header info
    GpsrPositionHeader gpsrHeader;
    if (!p->PeekHeader(gpsrHeader)) {
        NS_LOG_ERROR("RecoveryMode: Packet is missing GpsrPositionHeader!");
        // Fixed: Use correct error code
        ecb(p, header, Socket::ERROR_NOROUTETOHOST); 
        return;
    }
    // Extract positions from header
    Vector prevPos = Vector(gpsrHeader.GetPrevPositionX(), gpsrHeader.GetPrevPositionY(), 0);
    Vector dstPos = Vector(gpsrHeader.GetDstPositionX(), gpsrHeader.GetDstPositionY(), 0);
    // Fixed: Extract recPos needed for BestAngle
    Vector recPos = Vector(gpsrHeader.GetRecPositionX(), gpsrHeader.GetRecPositionY(), 0);

    NS_LOG_DEBUG("RecoveryMode: MyPos=" << myPos << " DstPos=" << dstPos << " PrevPos=" << prevPos << " RecPos=" << recPos);

    // Use the right-hand rule to find the next hop
    // Fixed: Pass recPos to BestAngle
    Ipv4Address nextHop = m_neighbors.BestAngle(dstPos, recPos, myPos, prevPos);

    if (nextHop != Ipv4Address::GetZero()) {
        NS_LOG_DEBUG("RecoveryMode: Found next hop " << nextHop << " using right-hand rule.");

        // Create a copy of the packet to modify header for the next hop
        Ptr<Packet> packetCopy = p->Copy();
        GpsrPositionHeader oldHeader;
        packetCopy->RemoveHeader(oldHeader); // Remove the old header

        // Update the header: Previous hop position becomes current node's position
        GpsrPositionHeader newHeader = oldHeader; 
        newHeader.SetPrevPositionX(myPos.x);
        newHeader.SetPrevPositionY(myPos.y);
        packetCopy->AddHeader(newHeader); // Add the updated header

        // Send the packet
        int32_t interfaceIndex = m_ipv4->GetInterfaceForAddress(nextHop);
        Ptr<NetDevice> oif = nullptr;
        if (interfaceIndex < 0) {
            NS_LOG_WARN ("RecoveryMode: Could not find Output Interface for next hop " << nextHop);
            ecb(packetCopy, header, Socket::ERROR_NOROUTETOHOST);
            return;
        } else {
            oif = m_ipv4->GetNetDevice(static_cast<uint32_t>(interfaceIndex));
            if (!oif) {
                NS_LOG_ERROR ("RecoveryMode: Could not get Output NetDevice for interface index " << interfaceIndex);
                ecb(packetCopy, header, Socket::ERROR_NOROUTETOHOST);
                return;
            }
        }
        
        // Fixed: Create Ipv4Route object and call ucb correctly
        Ptr<Ipv4Route> route = Create<Ipv4Route>();
        route->SetDestination(dst);
        route->SetSource(header.GetSource()); // Use original header source
        route->SetGateway(nextHop);
        route->SetOutputDevice(oif);
        NS_LOG_LOGIC("RecoveryMode: Calling UCB for Dst=" << dst << " NextHop=" << nextHop);
        ucb(route, packetCopy, header);

    } else {
        NS_LOG_WARN("RecoveryMode: No next hop found using right-hand rule for dst " << dst << ". Packet dropped.");
        ecb(p, header, Socket::ERROR_NOROUTETOHOST); // Notify caller
    }
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
    NS_LOG_FUNCTION(this << p << source << destination);

    // Get destination position using Oracle lookup
    Vector dstPos;
    bool dstPosFound = false;
    Ptr<MobilityModel> dstMobility = nullptr;

    // Iterate through all nodes (same logic as RouteOutput/ForwardingGreedy)
    for (uint32_t i = 0; i < NodeList::GetNNodes(); ++i)
    {
        Ptr<Node> node = NodeList::GetNode(i);
        Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
        if (ipv4) {
            for (uint32_t j = 0; j < ipv4->GetNInterfaces(); ++j) {
                for (uint32_t k = 0; k < ipv4->GetNAddresses(j); ++k) {
                    if (ipv4->GetAddress(j, k).GetLocal() == destination) {
                        dstMobility = node->GetObject<MobilityModel>();
                        if (dstMobility) {
                            dstPos = dstMobility->GetPosition();
                            dstPosFound = true;
                            NS_LOG_LOGIC("AddHeaders: Found destination node " << node->GetId() << " at position " << dstPos << " for IP " << destination);
                        } else {
                             NS_LOG_WARN("AddHeaders: Dest node " << node->GetId() << " for IP " << destination << " has no mobility model.");
                        }
                        goto end_node_search_addheaders;
                    }
                }
            }
        }
    }
    end_node_search_addheaders:;

    if (!dstPosFound) {
        NS_LOG_ERROR("AddHeaders: Could not find position for destination IP " << destination << ". Header not added/incomplete.");
        // Decide how to handle this - maybe don't add header? Or add with invalid pos?
        // For now, let's proceed but log the error. The routing might fail later.
        // Setting dstPos to (0,0,0) as a fallback.
        dstPos = Vector(0,0,0);
    }

    // Add GpsrPositionHeader
    GpsrPositionHeader gpsrHeader;
    gpsrHeader.SetDstPositionX(dstPos.x);
    gpsrHeader.SetDstPositionY(dstPos.y);
    // Initialize other fields for a packet originating from source (not yet in recovery)
    gpsrHeader.SetRecPositionX(0); // Not in recovery yet
    gpsrHeader.SetRecPositionY(0);
    gpsrHeader.SetPrevPositionX(0); // No previous hop yet
    gpsrHeader.SetPrevPositionY(0);
    gpsrHeader.SetRecoveryFlag(false); // Not in recovery
    // TODO: Ensure GpsrPositionHeader has these fields and setters.

    p->AddHeader(gpsrHeader);
}

} // namespace ns3