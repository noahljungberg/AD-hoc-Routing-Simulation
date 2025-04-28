#include "gpsr/gpsr-packet.h"
#include "ns3/address-utils.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include <cstring> // Added for memcpy

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
  NS_LOG_DEBUG("GpsrTypeHeader::Deserialize: Read byte = " << static_cast<int>(type));

  // Check the explicit numeric values based on the GpsrMessageType enum
  switch (type) {
    case 0: // Explicitly check for GPSR_HELLO value
      m_type = GPSR_HELLO;
      NS_LOG_DEBUG("GpsrTypeHeader::Deserialize: Interpreted as HELLO");
      break;
    case 1: // Explicitly check for GPSR_POSITION value
      m_type = GPSR_POSITION;
      NS_LOG_DEBUG("GpsrTypeHeader::Deserialize: Interpreted as POSITION");
      break;
    default:
      NS_LOG_WARN("GpsrTypeHeader::Deserialize: Unknown type byte " << static_cast<int>(type));
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

GpsrHelloHeader::GpsrHelloHeader(double x, double y) :
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
  return sizeof(double) * 2;
}

void
GpsrHelloHeader::Serialize(Buffer::Iterator i) const
{
  NS_LOG_DEBUG("Serialize X " << m_positionX << " Y " << m_positionY);
  i.Write((uint8_t*)&m_positionX, sizeof(double));
  i.Write((uint8_t*)&m_positionY, sizeof(double));
}

uint32_t
GpsrHelloHeader::Deserialize(Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  i.Read((uint8_t*)&m_positionX, sizeof(double));
  i.Read((uint8_t*)&m_positionY, sizeof(double));

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
GpsrHelloHeader::SetPositionX(double x)
{
  m_positionX = x;
}

double
GpsrHelloHeader::GetPositionX() const
{
  return m_positionX;
}

void
GpsrHelloHeader::SetPositionY(double y)
{
  m_positionY = y;
}

double
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

GpsrPositionHeader::GpsrPositionHeader(double dstX, double dstY, uint32_t updated,
                                     double recX, double recY, bool recoveryFlag,
                                     double prevX, double prevY) :
  m_dstPositionX(dstX),
  m_dstPositionY(dstY),
  m_updated(updated),
  m_recPositionX(recX),
  m_recPositionY(recY),
  m_recoveryFlag(static_cast<uint8_t>(recoveryFlag)),
  m_prevPositionX(prevX),
  m_prevPositionY(prevY)
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
  return sizeof(double) * 6 + sizeof(uint32_t) + sizeof(uint8_t);
}

void
GpsrPositionHeader::Serialize(Buffer::Iterator i) const
{
  NS_LOG_DEBUG("Serialize DstX " << m_dstPositionX << " DstY " << m_dstPositionY 
               << " RecX " << m_recPositionX << " RecY " << m_recPositionY 
               << " PrevX " << m_prevPositionX << " PrevY " << m_prevPositionY 
               << " Updated " << m_updated << " Recovery " << static_cast<bool>(m_recoveryFlag));

  i.Write((uint8_t*)&m_dstPositionX, sizeof(double));
  i.Write((uint8_t*)&m_dstPositionY, sizeof(double));
  i.Write((uint8_t*)&m_recPositionX, sizeof(double));
  i.Write((uint8_t*)&m_recPositionY, sizeof(double));
  i.Write((uint8_t*)&m_prevPositionX, sizeof(double));
  i.Write((uint8_t*)&m_prevPositionY, sizeof(double));

  i.WriteHtonU32(m_updated);
  i.WriteU8(m_recoveryFlag);
}

uint32_t
GpsrPositionHeader::Deserialize(Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  i.Read((uint8_t*)&m_dstPositionX, sizeof(double));
  i.Read((uint8_t*)&m_dstPositionY, sizeof(double));
  i.Read((uint8_t*)&m_recPositionX, sizeof(double));
  i.Read((uint8_t*)&m_recPositionY, sizeof(double));
  i.Read((uint8_t*)&m_prevPositionX, sizeof(double));
  i.Read((uint8_t*)&m_prevPositionY, sizeof(double));

  m_updated = i.ReadNtohU32();
  m_recoveryFlag = i.ReadU8();

  NS_LOG_DEBUG("Deserialize DstX " << m_dstPositionX << " DstY " << m_dstPositionY 
                 << " RecX " << m_recPositionX << " RecY " << m_recPositionY 
                 << " PrevX " << m_prevPositionX << " PrevY " << m_prevPositionY 
                 << " Updated " << m_updated << " Recovery " << static_cast<bool>(m_recoveryFlag));

  uint32_t dist = i.GetDistanceFrom(start);
  NS_ASSERT(dist == GetSerializedSize());
  return dist;
}

void
GpsrPositionHeader::Print(std::ostream &os) const
{
  os << " DstX: " << m_dstPositionX
     << " DstY: " << m_dstPositionY
     << " RecX: " << m_recPositionX
     << " RecY: " << m_recPositionY
     << " PrevX: " << m_prevPositionX
     << " PrevY: " << m_prevPositionY
     << " Updated: " << m_updated
     << " Recovery: " << (m_recoveryFlag ? "true" : "false");
}

void
GpsrPositionHeader::SetDstPositionX(double x)
{
  m_dstPositionX = x;
}

double
GpsrPositionHeader::GetDstPositionX() const
{
  return m_dstPositionX;
}

void
GpsrPositionHeader::SetDstPositionY(double y)
{
  m_dstPositionY = y;
}

double
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
GpsrPositionHeader::SetRecPositionX(double x)
{
  m_recPositionX = x;
}

double
GpsrPositionHeader::GetRecPositionX() const
{
  return m_recPositionX;
}

void
GpsrPositionHeader::SetRecPositionY(double y)
{
  m_recPositionY = y;
}

double
GpsrPositionHeader::GetRecPositionY() const
{
  return m_recPositionY;
}

void
GpsrPositionHeader::SetRecoveryFlag(bool flag)
{
  m_recoveryFlag = static_cast<uint8_t>(flag);
}

bool
GpsrPositionHeader::GetRecoveryFlag() const
{
  return static_cast<bool>(m_recoveryFlag);
}

void
GpsrPositionHeader::SetPrevPositionX(double x)
{
  m_prevPositionX = x;
}

double
GpsrPositionHeader::GetPrevPositionX() const
{
  return m_prevPositionX;
}

void
GpsrPositionHeader::SetPrevPositionY(double y)
{
  m_prevPositionY = y;
}

double
GpsrPositionHeader::GetPrevPositionY() const
{
  return m_prevPositionY;
}

bool
GpsrPositionHeader::operator==(GpsrPositionHeader const & o) const
{
  return m_dstPositionX == o.m_dstPositionX &&
         m_dstPositionY == o.m_dstPositionY &&
         m_updated == o.m_updated &&
         m_recPositionX == o.m_recPositionX &&
         m_recPositionY == o.m_recPositionY &&
         m_recoveryFlag == o.m_recoveryFlag &&
         m_prevPositionX == o.m_prevPositionX &&
         m_prevPositionY == o.m_prevPositionY;
}

std::ostream &
operator<<(std::ostream & os, GpsrPositionHeader const & h)
{
  h.Print(os);
  return os;
}

} // namespace ns3