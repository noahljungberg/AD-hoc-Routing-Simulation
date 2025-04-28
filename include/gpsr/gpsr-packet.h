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
  GPSR_HELLO = 0,       // Hello message
  GPSR_POSITION = 1,    // Position message
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
   *  Constructor (using double)
   *  x X coordinate
   *  y Y coordinate
   */
  GpsrHelloHeader(double x = 0.0, double y = 0.0);

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
   *  Set the X coordinate (using double)
   *  x The X coordinate
   */
  void SetPositionX(double x);

  /**
   *  Get the X coordinate (using double)
   *  The X coordinate
   */
  double GetPositionX() const;

  /**
   *  Set the Y coordinate (using double)
   *  y The Y coordinate
   */
  void SetPositionY(double y);

  /**
   *  Get the Y coordinate (using double)
   *  The Y coordinate
   */
  double GetPositionY() const;

  /**
   *  Comparison operator
   *  o The header to compare with
   *  True if equal, false otherwise
   */
  bool operator==(GpsrHelloHeader const & o) const;

private:
  double m_positionX;
  double m_positionY;
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
   *  Using double for coordinates now.
   *  Renamed lastX/Y to prevX/Y.
   *  Renamed inRec to recoveryFlag.
   */
  GpsrPositionHeader(double dstX = 0.0, double dstY = 0.0, uint32_t updated = 0,
                   double recX = 0.0, double recY = 0.0, bool recoveryFlag = false,
                   double prevX = 0.0, double prevY = 0.0);

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
  void SetDstPositionX(double x);
  double GetDstPositionX() const;

  void SetDstPositionY(double y);
  double GetDstPositionY() const;

  void SetUpdated(uint32_t updated);
  uint32_t GetUpdated() const;

  void SetRecPositionX(double x);
  double GetRecPositionX() const;

  void SetRecPositionY(double y);
  double GetRecPositionY() const;

  // Renamed from Set/GetInRecovery
  void SetRecoveryFlag(bool flag);
  bool GetRecoveryFlag() const;

  // Renamed from Set/GetLastPositionX/Y
  void SetPrevPositionX(double x);
  double GetPrevPositionX() const;

  void SetPrevPositionY(double y);
  double GetPrevPositionY() const;

  /**
   *  Comparison operator
   *  o The header to compare with
   *  True if equal, false otherwise
   */
  bool operator==(GpsrPositionHeader const & o) const;

private:
  // Using double for coordinates
  double m_dstPositionX;   // Destination X coordinate
  double m_dstPositionY;   // Destination Y coordinate
  uint32_t m_updated;        // Time of last update (purpose still unclear)
  double m_recPositionX;   // Recovery position X coordinate
  double m_recPositionY;   // Recovery position Y coordinate
  uint8_t m_recoveryFlag;    // Flag for recovery mode (using uint8_t for storage, API uses bool)
  // Renamed from m_lastPositionX/Y
  double m_prevPositionX;  // Previous hop X coordinate in recovery
  double m_prevPositionY;  // Previous hop Y coordinate in recovery
};

/**
 *  Stream insertion operator
 */
std::ostream & operator<<(std::ostream & os, GpsrPositionHeader const & h);

} // namespace ns3

#endif // GPSR_PACKET_H
