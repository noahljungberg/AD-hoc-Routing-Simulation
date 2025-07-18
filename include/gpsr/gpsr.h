//
// Created by Noah Ljungberg on 2025-04-17.
//


#ifndef GPSR_H
#define GPSR_H

#include "gpsr-ptable.h"
#include "gpsr-packet.h"
#include "gpsr-rqueue.h"
#include "ns3/node.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ip-l4-protocol.h"
#include "ns3/mobility-model.h"
#include "ns3/vector.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

/**
 *  GPSR routing protocol
 *
 * This class implements the Greedy Perimeter Stateless Routing protocol
 * for ad-hoc networks as described in the paper by Brad Karp and H. T. Kung.
 */
class Gpsr : public Ipv4RoutingProtocol
{
public:
  /**
   *  Get the type ID.
   *  The object TypeId
   */
  static TypeId GetTypeId (void);
  static const uint32_t GPSR_PORT;

  Gpsr();
  virtual ~Gpsr();
  virtual void DoDispose();

  // From Ipv4RoutingProtocol
  Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);
  virtual bool RouteInput(Ptr<const Packet> p, const Ipv4Header &header,
                      Ptr<const NetDevice> idev,
                      const UnicastForwardCallback &ucb,
                      const MulticastForwardCallback &mcb,
                      const LocalDeliverCallback &lcb,
                      const ErrorCallback &ecb);
  virtual void NotifyInterfaceUp(uint32_t interface);
  virtual void NotifyInterfaceDown(uint32_t interface);
  virtual void NotifyAddAddress(uint32_t interface, Ipv4InterfaceAddress address);
  virtual void NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address);
  virtual void SetIpv4(Ptr<Ipv4> ipv4);
  virtual void PrintRoutingTable(Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S) const;
  virtual void AddHeaders (Ptr<Packet> p, Ipv4Address source, Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route);
    virtual int GetProtocolNumber(void) const;
    virtual void SetDownTarget(IpL4Protocol::DownTargetCallback callback);
    virtual IpL4Protocol::DownTargetCallback GetDownTarget(void) const;


  // GPSR specific methods
  void RecvGpsr(Ptr<Socket> socket);
  void SendHello();
  void UpdateRouteToNeighbor(Ipv4Address neighbor, Vector position);
  bool IsMyOwnAddress(Ipv4Address addr);

private:
  // Start protocol operation
  void Start();

  // Handle deferred route output
  void DeferredRouteOutput(Ptr<const Packet> p, const Ipv4Header & header, UnicastForwardCallback ucb, ErrorCallback ecb);

  // Helper to drop queued packets for a destination
  void DropPacketWithDst(Ipv4Address dst, std::string reason);

  // Forward packet using greedy forwarding
  bool ForwardingGreedy(Ptr<const Packet> p, const Ipv4Header & header, UnicastForwardCallback ucb, ErrorCallback ecb);

  // Enter recovery mode when greedy forwarding fails
  void RecoveryMode(Ipv4Address dst, Ptr<Packet> p, UnicastForwardCallback ucb, Ipv4Header header, const ErrorCallback &ecb);

  // Loopback route for self-addressed packets
  Ptr<Ipv4Route> LoopbackRoute(const Ipv4Header & header, Ptr<NetDevice> oif);

  // Find socket with local interface address
  Ptr<Socket> FindSocketWithInterfaceAddress(Ipv4InterfaceAddress iface) const;

  // Send packet from queue
  bool SendPacketFromQueue(Ipv4Address dst);

  // Check the packet queue
  void CheckQueue();

  // Protocol parameters
  Time m_helloInterval;
  uint32_t m_maxQueueLen;
  Time m_maxQueueTime;

  // Internal state
  Ptr<Ipv4> m_ipv4;
  std::map<Ptr<Socket>, Ipv4InterfaceAddress> m_socketAddresses;
  Ptr<NetDevice> m_lo;
  GpsrPtable m_neighbors; // Position table for neighbors
  GpsrRqueue m_queue; // Request queue for packets
  bool m_perimeterMode; // Flag for perimeter mode
  std::list<Ipv4Address> m_queuedAddresses;
  Ptr<UniformRandomVariable> m_uniformRandomVariable;

  // Timers
  Timer m_helloTimer;
  Timer m_queueTimer;

  // Callbacks
  IpL4Protocol::DownTargetCallback m_downTarget;

  // New methods for position management and recovery mode
  Vector GetNodePosition(Ptr<Node> node);
  Vector GetDestinationPosition(Ipv4Address dst);
  void UpdatePositionTable();
  bool IsCloserToDestination(Vector nextHopPos, Vector myPos, Vector dstPos);



protected:
  virtual void DoInitialize(void);
};

}

#endif // GPSR_H

