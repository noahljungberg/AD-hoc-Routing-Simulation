#include "gpsr/gpsr-rqueue.h"
#include <algorithm>
#include <functional>
#include "ns3/ipv4-route.h"
#include "ns3/socket.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("GpsrRqueue");

/***************************************************
 *            GpsrQueueEntry Implementation
 ***************************************************/

GpsrQueueEntry::GpsrQueueEntry(Ptr<const Packet> p, Ipv4Header const & h,
                               Ipv4RoutingProtocol::UnicastForwardCallback ucb,
                               Ipv4RoutingProtocol::ErrorCallback ecb,
                               Time exp) :
  m_packet(p),
  m_header(h),
  m_ucb(ucb),
  m_ecb(ecb),
  m_expire(exp + Simulator::Now())
{
}

bool
GpsrQueueEntry::operator==(GpsrQueueEntry const & o) const
{
  return m_packet == o.m_packet &&
         m_header.GetDestination() == o.m_header.GetDestination() &&
         m_expire == o.m_expire;
}

Ipv4RoutingProtocol::UnicastForwardCallback
GpsrQueueEntry::GetUnicastForwardCallback() const
{
  return m_ucb;
}

void
GpsrQueueEntry::SetUnicastForwardCallback(Ipv4RoutingProtocol::UnicastForwardCallback ucb)
{
  m_ucb = ucb;
}

Ipv4RoutingProtocol::ErrorCallback
GpsrQueueEntry::GetErrorCallback() const
{
  return m_ecb;
}

void
GpsrQueueEntry::SetErrorCallback(Ipv4RoutingProtocol::ErrorCallback ecb)
{
  m_ecb = ecb;
}

Ptr<const Packet>
GpsrQueueEntry::GetPacket() const
{
  return m_packet;
}

void
GpsrQueueEntry::SetPacket(Ptr<const Packet> p)
{
  m_packet = p;
}

Ipv4Header
GpsrQueueEntry::GetIpv4Header() const
{
  return m_header;
}

void
GpsrQueueEntry::SetIpv4Header(Ipv4Header h)
{
  m_header = h;
}

void
GpsrQueueEntry::SetExpireTime(Time exp)
{
  m_expire = exp + Simulator::Now();
}

Time
GpsrQueueEntry::GetExpireTime() const
{
  return m_expire - Simulator::Now();
}

/***************************************************
 *            GpsrRqueue Implementation
 ***************************************************/

GpsrRqueue::GpsrRqueue(uint32_t maxLen, Time timeout) :
  m_maxLen(maxLen),
  m_queueTimeout(timeout)
{
}

bool
GpsrRqueue::IsEmpty() const
{
  return m_queue.empty();
}

bool
GpsrRqueue::Enqueue(GpsrQueueEntry & entry)
{
  Purge();

  // Check if this packet is already in the queue (avoid duplicates)
  for (std::vector<GpsrQueueEntry>::const_iterator i = m_queue.begin(); i != m_queue.end(); ++i) {
    if (i->GetPacket()->GetUid() == entry.GetPacket()->GetUid() &&
        i->GetIpv4Header().GetDestination() == entry.GetIpv4Header().GetDestination()) {
      return false;
    }
  }

  // Set timeout for the entry
  entry.SetExpireTime(m_queueTimeout);

  // If queue full, drop the oldest packet
  if (m_queue.size() == m_maxLen) {
    Drop(m_queue.front(), "Drop the most aged packet");
    m_queue.erase(m_queue.begin());
  }

  // Add new packet to queue
  m_queue.push_back(entry);
  return true;
}

bool
GpsrRqueue::Dequeue(Ipv4Address dst, GpsrQueueEntry & entry)
{
  Purge();

  for (std::vector<GpsrQueueEntry>::iterator i = m_queue.begin(); i != m_queue.end(); ++i) {
    if (i->GetIpv4Header().GetDestination() == dst) {
      entry = *i;
      m_queue.erase(i);
      return true;
    }
  }

  return false;
}

bool
GpsrRqueue::Find(Ipv4Address dst)
{
  for (std::vector<GpsrQueueEntry>::const_iterator i = m_queue.begin(); i != m_queue.end(); ++i) {
    if (i->GetIpv4Header().GetDestination() == dst) {
      return true;
    }
  }

  return false;
}

uint32_t
GpsrRqueue::GetSize()
{
  Purge();
  return m_queue.size();
}

void
GpsrRqueue::DropPacketWithDst(Ipv4Address dst)
{
  NS_LOG_FUNCTION(this << dst);
  Purge();

  // Drop all packets with this destination
  for (std::vector<GpsrQueueEntry>::iterator i = m_queue.begin(); i != m_queue.end();) {
    if (i->GetIpv4Header().GetDestination() == dst) {
      Drop(*i, "DropPacketWithDst");
      i = m_queue.erase(i);
    } else {
      ++i;
    }
  }
}

void
GpsrRqueue::SetMaxQueueLen(uint32_t len)
{
  m_maxLen = len;
}

uint32_t
GpsrRqueue::GetMaxQueueLen() const
{
  return m_maxLen;
}

void
GpsrRqueue::SetQueueTimeout(Time t)
{
  m_queueTimeout = t;
}

Time
GpsrRqueue::GetQueueTimeout() const
{
  return m_queueTimeout;
}

void
GpsrRqueue::Purge()
{
  // Remove all expired entries
  for (std::vector<GpsrQueueEntry>::iterator i = m_queue.begin(); i != m_queue.end();) {
    if (i->GetExpireTime() < Seconds(0)) {
      Drop(*i, "Drop outdated packet");
      i = m_queue.erase(i);
    } else {
      ++i;
    }
  }
}

void
GpsrRqueue::Drop(GpsrQueueEntry en, std::string reason)
{
  NS_LOG_LOGIC(reason << en.GetPacket()->GetUid() << " " << en.GetIpv4Header().GetDestination());
  en.GetErrorCallback()(en.GetPacket(), en.GetIpv4Header(), Socket::ERROR_NOROUTETOHOST);
}

bool
GpsrRqueue::IsEqual(GpsrQueueEntry entry, const Ipv4Address dst)
{
  return entry.GetIpv4Header().GetDestination() == dst;
}

} // namespace ns3