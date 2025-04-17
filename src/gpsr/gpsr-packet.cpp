#include "gpsr/gpsr-packet.h"
#include "ns3/address-utils.h"
#include "ns3/packet.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("GpsrPacket");

/***************************************************
 *            Type Header Implementation
 ***************************************************/

NS_OBJECT_ENSURE_REGISTERED(GpsrTypeHeader);

GpsrTypeHeader::GpsrTypeHeader(GpsrMessageType t) :
  m_type(t),
  m_valid(true)
{
}

TypeId
GpsrTypeHeader::GetTypeId()
{
  static TypeId tid = TypeId("ns3::GpsrTypeHeader")
    .SetParent<Header>()
    .SetGroupName("Gpsr")
    .AddConstructor<GpsrTypeHeader>();
  return tid;
}

TypeId
GpsrTypeHeader::GetInstanceTypeId() const
{
  return GetTypeId();
}

uint32_t
GpsrTypeHeader::GetSerializedSize() const
{
  return 1;
}

void
GpsrTypeHeader::Serialize(Buffer::Iterator i) const
{
  i.WriteU8((uint8_t)m_type);
}

uint32_t
GpsrTypeHeader::Deserialize(Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint8_t type = i.ReadU8();
  m_valid = true;

  switch (type) {
    case GPSR_HELLO:
    case GPSR_POSITION:
      m_type = (GpsrMessageType)type;
      break;
    default:
      m_valid = false;
  }

  uint32_t dist = i.GetDistanceFrom(start);
  NS_ASSERT(dist == GetSerializedSize());
  return dist;
}

void
GpsrTypeHeader::Print(std::ostream &os) const
{
  switch (m_type) {
    case GPSR_HELLO:
      os << "HELLO";
      break;
    case GPSR_POSITION:
      os << "POSITION";
      break;
    default:
      os << "UNKNOWN_TYPE";
  }
}

GpsrMessageType
GpsrTypeHeader::Get() const
{
  return m_type;
}

bool
GpsrTypeHeader::IsValid() const
{
  return m_valid;
}

bool
GpsrTypeHeader::operator==(GpsrTypeHeader const & o) const
{
  return m_type == o.m_type && m_valid == o.m_valid;
}

std::ostream &
operator<<(std::ostream & os, GpsrTypeHeader const & h)
{
  h.Print(os);
  return os;
}

/***************************************************
 *            Hello Header Implementation
 ***************************************************/

NS_OBJECT_ENSURE_REGISTERED(GpsrHelloHeader);

GpsrHelloHeader::GpsrHelloHeader(uint64_t x, uint64_t y) :
  m_positionX(x),
  m_positionY(y)
{
}

TypeId
GpsrHelloHeader::GetTypeId()
{
  static TypeId tid = TypeId("ns3::GpsrHelloHeader")
    .SetParent<Header>()
    .SetGroupName("Gpsr")
    .AddConstructor<GpsrHelloHeader>();
  return tid;
}

TypeId
GpsrHelloHeader::GetInstanceTypeId() const
{
  return GetTypeId();
}

uint32_t
GpsrHelloHeader::GetSerializedSize() const
{
  return 16; // 8 bytes for each position coordinate
}

void
GpsrHelloHeader::Serialize(Buffer::Iterator i) const
{
  NS_LOG_DEBUG("Serialize X " << m_positionX << " Y " << m_positionY);

  i.WriteHtonU64(m_positionX);
  i.WriteHtonU64(m_positionY);
}

uint32_t
GpsrHelloHeader::Deserialize(Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_positionX = i.ReadNtohU64();
  m_positionY = i.ReadNtohU64();

  NS_LOG_DEBUG("Deserialize X " << m_positionX << " Y " << m_positionY);

  uint32_t dist = i.GetDistanceFrom(start);
  NS_ASSERT(dist == GetSerializedSize());
  return dist;
}

void
GpsrHelloHeader::Print(std::ostream &os) const
{
  os << " PositionX: " << m_positionX
     << " PositionY: " << m_positionY;
}

void
GpsrHelloHeader::SetPositionX(uint64_t x)
{
  m_positionX = x;
}

uint64_t
GpsrHelloHeader::GetPositionX() const
{
  return m_positionX;
}

void
GpsrHelloHeader::SetPositionY(uint64_t y)
{
  m_positionY = y;
}

uint64_t
GpsrHelloHeader::GetPositionY() const
{
  return m_positionY;
}

bool
GpsrHelloHeader::operator==(GpsrHelloHeader const & o) const
{
  return m_positionX == o.m_positionX && m_positionY == o.m_positionY;
}

std::ostream &
operator<<(std::ostream & os, GpsrHelloHeader const & h)
{
  h.Print(os);
  return os;
}

/***************************************************
 *            Position Header Implementation
 ***************************************************/

NS_OBJECT_ENSURE_REGISTERED(GpsrPositionHeader);

GpsrPositionHeader::GpsrPositionHeader(uint64_t dstX, uint64_t dstY, uint32_t updated,
                                     uint64_t recX, uint64_t recY, uint8_t inRec,
                                     uint64_t lastX, uint64_t lastY) :
  m_dstPositionX(dstX),
  m_dstPositionY(dstY),
  m_updated(updated),
  m_recPositionX(recX),
  m_recPositionY(recY),
  m_inRecovery(inRec),
  m_lastPositionX(lastX),
  m_lastPositionY(lastY)
{
}

TypeId
GpsrPositionHeader::GetTypeId()
{
  static TypeId tid = TypeId("ns3::GpsrPositionHeader")
    .SetParent<Header>()
    .SetGroupName("Gpsr")
    .AddConstructor<GpsrPositionHeader>();
  return tid;
}

TypeId
GpsrPositionHeader::GetInstanceTypeId() const
{
  return GetTypeId();
}

uint32_t
GpsrPositionHeader::GetSerializedSize() const
{
  return 53; // 8*6 + 4 + 1 bytes
}

void
GpsrPositionHeader::Serialize(Buffer::Iterator i) const
{
  i.WriteU64(m_dstPositionX);
  i.WriteU64(m_dstPositionY);
  i.WriteU32(m_updated);
  i.WriteU64(m_recPositionX);
  i.WriteU64(m_recPositionY);
  i.WriteU8(m_inRecovery);
  i.WriteU64(m_lastPositionX);
  i.WriteU64(m_lastPositionY);
}

uint32_t
GpsrPositionHeader::Deserialize(Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_dstPositionX = i.ReadU64();
  m_dstPositionY = i.ReadU64();
  m_updated = i.ReadU32();
  m_recPositionX = i.ReadU64();
  m_recPositionY = i.ReadU64();
  m_inRecovery = i.ReadU8();
  m_lastPositionX = i.ReadU64();
  m_lastPositionY = i.ReadU64();

  uint32_t dist = i.GetDistanceFrom(start);
  NS_ASSERT(dist == GetSerializedSize());
  return dist;
}

void
GpsrPositionHeader::Print(std::ostream &os) const
{
  os << " DestinationX: " << m_dstPositionX
     << " DestinationY: " << m_dstPositionY
     << " Updated: " << m_updated
     << " RecoveryX: " << m_recPositionX
     << " RecoveryY: " << m_recPositionY
     << " InRecovery: " << (uint32_t)m_inRecovery
     << " LastX: " << m_lastPositionX
     << " LastY: " << m_lastPositionY;
}

void
GpsrPositionHeader::SetDstPositionX(uint64_t x)
{
  m_dstPositionX = x;
}

uint64_t
GpsrPositionHeader::GetDstPositionX() const
{
  return m_dstPositionX;
}

void
GpsrPositionHeader::SetDstPositionY(uint64_t y)
{
  m_dstPositionY = y;
}

uint64_t
GpsrPositionHeader::GetDstPositionY() const
{
  return m_dstPositionY;
}

void
GpsrPositionHeader::SetUpdated(uint32_t updated)
{
  m_updated = updated;
}

uint32_t
GpsrPositionHeader::GetUpdated() const
{
  return m_updated;
}

void
GpsrPositionHeader::SetRecPositionX(uint64_t x)
{
  m_recPositionX = x;
}

uint64_t
GpsrPositionHeader::GetRecPositionX() const
{
  return m_recPositionX;
}

void
GpsrPositionHeader::SetRecPositionY(uint64_t y)
{
  m_recPositionY = y;
}

uint64_t
GpsrPositionHeader::GetRecPositionY() const
{
  return m_recPositionY;
}

void
GpsrPositionHeader::SetInRecovery(uint8_t inRec)
{
  m_inRecovery = inRec;
}

uint8_t
GpsrPositionHeader::GetInRecovery() const
{
  return m_inRecovery;
}

void
GpsrPositionHeader::SetLastPositionX(uint64_t x)
{
  m_lastPositionX = x;
}

uint64_t
GpsrPositionHeader::GetLastPositionX() const
{
  return m_lastPositionX;
}

void
GpsrPositionHeader::SetLastPositionY(uint64_t y)
{
  m_lastPositionY = y;
}

uint64_t
GpsrPositionHeader::GetLastPositionY() const
{
  return m_lastPositionY;
}

bool
GpsrPositionHeader::operator==(GpsrPositionHeader const & o) const
{
  return m_dstPositionX == o.m_dstPositionX &&
         m_dstPositionY == o.m_dstPositionY &&
         m_updated == o.m_updated &&
         m_recPositionX == o.m_recPositionX &&
         m_recPositionY == o.m_recPositionY &&
         m_inRecovery == o.m_inRecovery &&
         m_lastPositionX == o.m_lastPositionX &&
         m_lastPositionY == o.m_lastPositionY;
}

std::ostream &
operator<<(std::ostream & os, GpsrPositionHeader const & h)
{
  h.Print(os);
  return os;
}

} // namespace ns3