//
// Created by Noah Ljungberg on 2025-04-17.
//

#ifndef GPSR_PACKET_H
#define GPSR_PACKET_H

#include "ns3/header.h"
#include "ns3/enum.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/vector.h"

namespace ns3 {

/**
 *  GPSR message types
 */
enum GpsrMessageType
{
  GPSR_HELLO = 1,       // Hello message
  GPSR_POSITION = 2,    // Position message
};

/**
 *  GPSR message type header
 */
class GpsrTypeHeader : public Header
{
public:
  /**
   *  Constructor
   *  t The message type
   */
  GpsrTypeHeader(GpsrMessageType t = GPSR_HELLO);

  /**
   *  Get the type ID
   *  The object TypeId
   */
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const;

  // Header serialization
  uint32_t GetSerializedSize() const;
  void Serialize(Buffer::Iterator start) const;
  uint32_t Deserialize(Buffer::Iterator start);
  void Print(std::ostream &os) const;

  /**
   *  Get the message type
   *  The message type
   */
  GpsrMessageType Get() const;

  /**
   *  Check if the header is valid
   *  True if valid, false otherwise
   */
  bool IsValid() const;

  /**
   *  Comparison operator
   *  o The header to compare with
   *  True if equal, false otherwise
   */
  bool operator==(GpsrTypeHeader const & o) const;

private:
  GpsrMessageType m_type; // Message type
  bool m_valid; // Header validity flag
};

/**
 *  Stream insertion operator
 */
std::ostream & operator<<(std::ostream & os, GpsrTypeHeader const & h);

/**
 *  GPSR Hello header
 *
 * Contains position information for neighbor discovery
 */
class GpsrHelloHeader : public Header
{
public:
  /**
   *  Constructor
   *  x X coordinate
   *  y Y coordinate
   */
  GpsrHelloHeader(uint64_t x = 0, uint64_t y = 0);

  /**
   *  Get the type ID
   *  The object TypeId
   */
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const;

  // Header serialization
  uint32_t GetSerializedSize() const;
  void Serialize(Buffer::Iterator start) const;
  uint32_t Deserialize(Buffer::Iterator start);
  void Print(std::ostream &os) const;

  /**
   *  Set the X coordinate
   *  x The X coordinate
   */
  void SetPositionX(uint64_t x);

  /**
   *  Get the X coordinate
   *  The X coordinate
   */
  uint64_t GetPositionX() const;

  /**
   *  Set the Y coordinate
   *  y The Y coordinate
   */
  void SetPositionY(uint64_t y);

  /**
   *  Get the Y coordinate
   *  The Y coordinate
   */
  uint64_t GetPositionY() const;

  /**
   *  Comparison operator
   *  o The header to compare with
   *  True if equal, false otherwise
   */
  bool operator==(GpsrHelloHeader const & o) const;

private:
  uint64_t m_positionX; // X coordinate
  uint64_t m_positionY; // Y coordinate
};

/**
 *  Stream insertion operator
 */
std::ostream & operator<<(std::ostream & os, GpsrHelloHeader const & h);

/**
 *  GPSR Position header
 *
 * Contains information for routing packets in GPSR
 */
class GpsrPositionHeader : public Header
{
public:
  /**
   *  Constructor
   *  dstX Destination X coordinate
   *  dstY Destination Y coordinate
   *  updated Time of last update
   *  recX Recovery position X coordinate
   *  recY Recovery position Y coordinate
   *  inRec Flag for recovery mode
   *  lastX Last position X coordinate
   *  lastY Last position Y coordinate
   */
  GpsrPositionHeader(uint64_t dstX = 0, uint64_t dstY = 0, uint32_t updated = 0,
                   uint64_t recX = 0, uint64_t recY = 0, uint8_t inRec = 0,
                   uint64_t lastX = 0, uint64_t lastY = 0);

  /**
   *  Get the type ID
   *  The object TypeId
   */
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const;

  // Header serialization
  uint32_t GetSerializedSize() const;
  void Serialize(Buffer::Iterator start) const;
  uint32_t Deserialize(Buffer::Iterator start);
  void Print(std::ostream &os) const;

  // Accessor methods for all fields
  void SetDstPositionX(uint64_t x);
  uint64_t GetDstPositionX() const;

  void SetDstPositionY(uint64_t y);
  uint64_t GetDstPositionY() const;

  void SetUpdated(uint32_t updated);
  uint32_t GetUpdated() const;

  void SetRecPositionX(uint64_t x);
  uint64_t GetRecPositionX() const;

  void SetRecPositionY(uint64_t y);
  uint64_t GetRecPositionY() const;

  void SetInRecovery(uint8_t inRec);
  uint8_t GetInRecovery() const;

  void SetLastPositionX(uint64_t x);
  uint64_t GetLastPositionX() const;

  void SetLastPositionY(uint64_t y);
  uint64_t GetLastPositionY() const;

  /**
   *  Comparison operator
   *  o The header to compare with
   *  True if equal, false otherwise
   */
  bool operator==(GpsrPositionHeader const & o) const;

private:
  uint64_t m_dstPositionX;   // Destination X coordinate
  uint64_t m_dstPositionY;   // Destination Y coordinate
  uint32_t m_updated;        // Time of last update
  uint64_t m_recPositionX;   // Recovery position X coordinate
  uint64_t m_recPositionY;   // Recovery position Y coordinate
  uint8_t m_inRecovery;      // Flag for recovery mode
  uint64_t m_lastPositionX;  // Last position X coordinate
  uint64_t m_lastPositionY;  // Last position Y coordinate
};

/**
 *  Stream insertion operator
 */
std::ostream & operator<<(std::ostream & os, GpsrPositionHeader const & h);

} // namespace ns3

#endif // GPSR_PACKET_H
