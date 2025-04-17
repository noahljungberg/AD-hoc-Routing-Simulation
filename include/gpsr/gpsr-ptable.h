//
// Created by Noah Ljungberg on 2025-04-17.
//
#ifndef GPSR_PTABLE_H
#define GPSR_PTABLE_H

#include "ns3/ipv4.h"
#include "ns3/timer.h"
#include "ns3/vector.h"
#include "ns3/mobility-model.h"
#include "ns3/wifi-mac-header.h"
#include <map>

namespace ns3 {

/**
 *  Position table used by GPSR
 *
 * This class manages the position information of neighboring nodes
 * and provides methods to find the best next hop for GPSR routing.
 */
class GpsrPtable
{
public:
  GpsrPtable();

  /**
   *  Gets the time when an entry was last updated
   *  id The IPv4 address of the node
   *  The time of the last update
   */
  Time GetEntryUpdateTime(Ipv4Address id);

  /**
   *  Adds an entry to the position table
   *  id The IPv4 address of the node
   *  position The position of the node
   */
  void AddEntry(Ipv4Address id, Vector position);

  /**
   *  Deletes an entry from the position table
   *  id The IPv4 address of the node to delete
   */
  void DeleteEntry(Ipv4Address id);

  /**
   *  Gets the position of a node
   *  id The IPv4 address of the node
   *  The position of the node
   */
  Vector GetPosition(Ipv4Address id);

  /**
   *  Checks if a node is a neighbor
   *  id The IPv4 address of the node
   *  True if the node is a neighbor, false otherwise
   */
  bool IsNeighbor(Ipv4Address id);

  /**
   *  Removes entries with expired lifetime
   */
  void Purge();

  /**
   *  Clears all entries
   */
  void Clear();

  /**
   *  Gets the TX error callback
   *  The TX error callback
   */
  Callback<void, WifiMacHeader const &> GetTxErrorCallback() const;

  /**
   *  Finds the best next hop using greedy forwarding
   *  position The position of the destination
   *  nodePos The position of the current node
   *  The IP address of the best next hop, or zero if none found
   */
  Ipv4Address BestNeighbor(Vector position, Vector nodePos);

  /**
   *  Finds the best next hop in perimeter mode using right-hand rule
   *  prevHop The position of the previous hop
   *  nodePos The position of the current node
   *  The IP address of the best next hop
   */
  Ipv4Address BestAngle(Vector prevHop, Vector nodePos);

  /**
   *  Calculates the angle between three points
   *  center The center point
   *  refPos The reference position
   *  node The position to calculate angle for
   *  The angle in degrees (counterclockwise)
   */
  double GetAngle(Vector center, Vector refPos, Vector node);

  /**
   *  Returns an invalid position vector
   *  Vector(-1, -1, 0)
   */
  static Vector GetInvalidPosition();

private:
  Time m_entryLifetime; // Lifetime of a position table entry
  std::map<Ipv4Address, std::pair<Vector, Time>> m_table; // Position table
  Callback<void, WifiMacHeader const &> m_txErrorCallback; // TX error callback

  // Process TX error notification
  void ProcessTxError(WifiMacHeader const &);
};

}

#endif // GPSR_PTABLE_H
