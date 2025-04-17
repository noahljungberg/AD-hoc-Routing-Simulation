//
// Created by Noah Ljungberg on 2025-04-17.
//

#ifndef GPSR_RQUEUE_H
#define GPSR_RQUEUE_H

#include "ns3/ipv4-routing-protocol.h"
#include "ns3/simulator.h"
#include <vector>

namespace ns3 {

/**
 * GPSR Queue Entry class
 */
class GpsrQueueEntry
{
public:
  /**
   *  Constructor
   *  p The packet
   *  h The IPv4 header
   *  ucb Unicast forward callback
   *  ecb Error callback
   *  exp Expiration time
   */
  GpsrQueueEntry(Ptr<const Packet> p = 0, Ipv4Header const & h = Ipv4Header(),
               Ipv4RoutingProtocol::UnicastForwardCallback ucb = Ipv4RoutingProtocol::UnicastForwardCallback(),
               Ipv4RoutingProtocol::ErrorCallback ecb = Ipv4RoutingProtocol::ErrorCallback(),
               Time exp = Simulator::Now());

  /**
   *  Comparison operator
   *  o The entry to compare with
   *  True if equal, false otherwise
   */
  bool operator==(GpsrQueueEntry const & o) const;

  // Accessor methods
  Ipv4RoutingProtocol::UnicastForwardCallback GetUnicastForwardCallback() const;
  void SetUnicastForwardCallback(Ipv4RoutingProtocol::UnicastForwardCallback ucb);

  Ipv4RoutingProtocol::ErrorCallback GetErrorCallback() const;
  void SetErrorCallback(Ipv4RoutingProtocol::ErrorCallback ecb);

  Ptr<const Packet> GetPacket() const;
  void SetPacket(Ptr<const Packet> p);

  Ipv4Header GetIpv4Header() const;
  void SetIpv4Header(Ipv4Header h);

  void SetExpireTime(Time exp);
  Time GetExpireTime() const;

private:
  Ptr<const Packet> m_packet;                            // Data packet
  Ipv4Header m_header;                                  // IP header
  Ipv4RoutingProtocol::UnicastForwardCallback m_ucb;    // Unicast forward callback
  Ipv4RoutingProtocol::ErrorCallback m_ecb;             // Error callback
  Time m_expire;                                        // Expire time
};

/**
 *  GPSR Request Queue
 */
class GpsrRqueue
{
public:
  /**
   *  Constructor
   *  maxLen Maximum queue length
   *  timeout Queue timeout
   */
  GpsrRqueue(uint32_t maxLen, Time timeout);

  /**
   *  Check if the queue is empty
   *  True if empty, false otherwise
   */
  bool IsEmpty() const;

  /**
   *  Enqueue a packet
   *  entry The queue entry
   *  True if successful, false otherwise
   */
  bool Enqueue(GpsrQueueEntry & entry);

  /**
   *  Dequeue first entry for a destination
   *  dst The destination IP address
   *  entry The queue entry to dequeue
   *  True if successful, false otherwise
   */
  bool Dequeue(Ipv4Address dst, GpsrQueueEntry & entry);

  /**
   *  Find an entry for a destination
   *  dst The destination IP address
   *  True if found, false otherwise
   */
  bool Find(Ipv4Address dst);

  /**
   *  Get the queue size
   *  The queue size
   */
  uint32_t GetSize();

  /**
   *  Drop packets for a destination
   *  dst The destination IP address
   */
  void DropPacketWithDst(Ipv4Address dst);

  /**
   *  Set the maximum queue length
   *  len The maximum queue length
   */
  void SetMaxQueueLen(uint32_t len);

  /**
   *  Get the maximum queue length
   *  The maximum queue length
   */
  uint32_t GetMaxQueueLen() const;

  /**
   *  Set the queue timeout
   *  t The queue timeout
   */
  void SetQueueTimeout(Time t);

  /**
   *  Get the queue timeout
   *  The queue timeout
   */
  Time GetQueueTimeout() const;

private:
  std::vector<GpsrQueueEntry> m_queue; // Request queue
  uint32_t m_maxLen;                   // Maximum queue length
  Time m_queueTimeout;                 // Queue timeout

  /**
   *  Remove all expired entries
   */
  void Purge();

  /**
   *  Notify that packet is dropped from queue by timeout
   *  entry The dropped entry
   *  reason The reason for dropping
   */
  void Drop(GpsrQueueEntry entry, std::string reason);

  /**
   *  Check if an entry matches a destination
   *  entry The queue entry
   *  dst The destination IP address
   *  True if matches, false otherwise
   */
  static bool IsEqual(GpsrQueueEntry entry, const Ipv4Address dst);
};

} // namespace ns3

#endif // GPSR_RQUEUE_H

