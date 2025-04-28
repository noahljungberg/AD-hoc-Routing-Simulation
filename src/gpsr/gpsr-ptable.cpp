#include "gpsr/gpsr-ptable.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <algorithm>
#include <cmath>
#include <complex>
#include "ns3/node-list.h"
#include "ns3/node.h"
#include "ns3/mobility-model.h"
#include "ns3/double.h" // Needed for MakeTimeAccessor/Checker
#include "ns3/uinteger.h" // Needed for MakeTimeAccessor/Checker
#include "ns3/enum.h" // Needed for MakeTimeAccessor/Checker

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("GpsrPtable");
NS_OBJECT_ENSURE_REGISTERED(GpsrPtable); // Ensure registered for GetTypeId

// Add GetTypeId method to allow configuration
TypeId
GpsrPtable::GetTypeId(void)
{
    static TypeId tid = TypeId ("ns3::GpsrPtable")
        .SetParent<Object> ()
        .SetGroupName ("Routing")
        .AddConstructor<GpsrPtable> ()
        .AddAttribute ("EntryLifetime", "Time after which a neighbor entry is considered expired.",
                       TimeValue (Seconds (3.0)), // Default to 3 * default HelloInterval (1s)
                       MakeTimeAccessor (&GpsrPtable::m_entryLifetime),
                       MakeTimeChecker ());
    return tid;
}

GpsrPtable::GpsrPtable() :
  m_entryLifetime(Seconds(3.0)) // Default, will be overwritten by attribute if set
{
  NS_LOG_FUNCTION(this << m_entryLifetime);
}

Time
GpsrPtable::GetEntryUpdateTime(Ipv4Address id)
{
  if (id == Ipv4Address::GetZero()) {
    return Time(Seconds(0));
  }

  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i = m_table.find(id);
  if (i != m_table.end()) {
    return i->second.second;
  }

  return Time(Seconds(0));
}

  void
  GpsrPtable::AddEntry(Ipv4Address id, Vector position)
{
  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i = m_table.find(id);
  if (i != m_table.end()) {
    m_table.erase(id);
  }

  m_table.insert(std::make_pair(id, std::make_pair(position, Simulator::Now())));

  // Add debug message for neighbor discovery
  NS_LOG_DEBUG("Added neighbor " << id << " at position (" << position.x << "," << position.y
                << "), table size: " << m_table.size());
}

void
GpsrPtable::DeleteEntry(Ipv4Address id)
{
  m_table.erase(id);
}

  Vector
  GpsrPtable::GetPosition(Ipv4Address id)
{
  Purge(); // Purge expired entries before lookup
  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i = m_table.find(id);
  if (i != m_table.end()) {
    NS_LOG_LOGIC("GetPosition: Found position " << i->second.first << " for " << id << " in table.");
    return i->second.first;
  }

  /* Removed inefficient and potentially incorrect NodeList fallback
  // If not found in table, try to get it from node list
  NS_LOG_LOGIC("GetPosition: " << id << " not found in table, searching NodeList (INEFFICIENT). This should ideally not happen.");
  for (NodeList::Iterator nodeIter = NodeList::Begin(); nodeIter != NodeList::End(); nodeIter++) {
    Ptr<Node> node = *nodeIter;
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
    if (ipv4) {
        for (uint32_t ifaceIdx = 0; ifaceIdx < ipv4->GetNInterfaces(); ++ifaceIdx) {
            for (uint32_t addrIdx = 0; addrIdx < ipv4->GetNAddresses(ifaceIdx); ++addrIdx) {
                if (ipv4->GetAddress(ifaceIdx, addrIdx).GetLocal() == id) {
                    Ptr<MobilityModel> mobility = node->GetObject<MobilityModel>();
                    if (mobility) {
                         NS_LOG_WARN("GetPosition: Found position for " << id << " via NodeList fallback.");
                        return mobility->GetPosition();
                    } else {
                        NS_LOG_WARN("GetPosition: Found node for " << id << " via NodeList, but it has no mobility model.");
                        return GetInvalidPosition();
                    }
                }
            }
        }
    }
  }
  */

  NS_LOG_WARN("GetPosition: Could not find position for " << id << " in neighbor table.");
  return GetInvalidPosition();
}

  bool
  GpsrPtable::IsNeighbor(Ipv4Address id)
{
  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i = m_table.find(id);
  if (i != m_table.end()) {
    NS_LOG_DEBUG("Found " << id << " as a neighbor");
    return true;
  }

  NS_LOG_DEBUG("Address " << id << " is not a neighbor");
  return false;
}
  void
  GpsrPtable::Purge()
{
  if (m_table.empty()) {
    return;
  }

  std::list<Ipv4Address> toErase;

  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i = m_table.begin();
  for (; i != m_table.end(); ++i) {
    if (m_entryLifetime + i->second.second <= Simulator::Now()) {
      NS_LOG_DEBUG("Purging expired neighbor entry for " << i->first);
      toErase.push_back(i->first);
    }
  }

  for (std::list<Ipv4Address>::iterator it = toErase.begin(); it != toErase.end(); ++it) {
    m_table.erase(*it);
  }

  if (!toErase.empty()) {
    NS_LOG_DEBUG("Purged " << toErase.size() << " expired neighbors, table size now: " << m_table.size());
  }
}

void
GpsrPtable::Clear()
{
  m_table.clear();
}

Ipv4Address
GpsrPtable::BestNeighbor(Vector position, Vector nodePos)
{
  Purge();

  // Calculate the distance from current node to destination
  double initialDistance = CalculateDistance(nodePos, position);

  if (m_table.empty()) {
    NS_LOG_DEBUG("BestNeighbor table is empty - no neighbors discovered yet");
    return Ipv4Address::GetZero();
  }

  // Find neighbor closest to destination
  Ipv4Address bestFoundId = m_table.begin()->first;
  double bestFoundDistance = CalculateDistance(m_table.begin()->second.first, position);

  NS_LOG_DEBUG("Looking for best neighbor to reach (" << position.x << "," << position.y
                << "), my position: (" << nodePos.x << "," << nodePos.y << ")");
  NS_LOG_DEBUG("My distance to destination: " << initialDistance);

  // Log all neighbors and their distances
  for (std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i = m_table.begin();
       i != m_table.end(); ++i) {
    double distance = CalculateDistance(i->second.first, position);
    NS_LOG_DEBUG("  Neighbor " << i->first << " at (" << i->second.first.x << ","
                 << i->second.first.y << "), distance: " << distance);

    if (bestFoundDistance > distance) {
      bestFoundId = i->first;
      bestFoundDistance = distance;
    }
       }

  // Only return neighbor if it's closer to destination than current node
  if (initialDistance > bestFoundDistance) {
    NS_LOG_DEBUG("Selected neighbor " << bestFoundId << " with distance " << bestFoundDistance);
    return bestFoundId;
  } else {
    // No neighbor closer - return empty address to trigger recovery
    NS_LOG_DEBUG("No neighbor is closer to destination than myself");
    return Ipv4Address::GetZero();
  }
}

Ipv4Address
GpsrPtable::BestAngle(Vector dstPos, Vector recPos, Vector myPos, Vector prevPos)
{
  Purge();
  NS_LOG_FUNCTION(this << " Dst:" << dstPos << " Rec:" << recPos << " My:" << myPos << " Prev:" << prevPos);

  if (m_table.empty()) {
    NS_LOG_DEBUG("BestAngle: Neighbor table empty at " << myPos);
    return Ipv4Address::GetZero();
  }

  // Debug: Log neighbor table contents
  NS_LOG_DEBUG("BestAngle: Current Neighbors at " << myPos << " (Table size: " << m_table.size() << "):");
  for (std::map<Ipv4Address, std::pair<Vector, Time> >::const_iterator dbg_it = m_table.begin(); 
       dbg_it != m_table.end(); ++dbg_it) {
      NS_LOG_DEBUG("  -> " << dbg_it->first << " at " << dbg_it->second.first 
                   << " (Updated: " << dbg_it->second.second.GetSeconds() << "s)");
  }
  // End Debug Log

  if (prevPos == GetInvalidPosition()) {
      NS_LOG_WARN("BestAngle: Previous hop position is invalid. Cannot apply right-hand rule.");
      return Ipv4Address::GetZero();
  }


  Ipv4Address bestFoundId = Ipv4Address::GetZero();
  double smallestAngle = 361.0; // Initialize with value > 360

  NS_LOG_DEBUG("BestAngle: Evaluating neighbors relative to edge " << prevPos << " -> " << myPos);

  for (std::map<Ipv4Address, std::pair<Vector, Time> >::const_iterator i = m_table.begin();
       i != m_table.end(); ++i) 
  {
    Vector neighborPos = i->second.first;
    Ipv4Address neighborId = i->first;

    if (neighborPos == prevPos) {
        NS_LOG_LOGIC("BestAngle: Skipping neighbor " << neighborId << " at previous hop position " << prevPos);
        continue;
    }

    double angle = GetAngle(myPos, prevPos, neighborPos);
    // Debug: Log calculated angle
    NS_LOG_DEBUG("BestAngle:  Neighbor " << neighborId << " at " << neighborPos << ", Angle = " << angle);

    // TODO: Implement check against recPos-dstPos line intersection here? 

    if (angle < smallestAngle) {
      // Debug: Log potential new best angle
      NS_LOG_DEBUG("BestAngle:   New smallest angle found: " << angle << " for neighbor " << neighborId);
      smallestAngle = angle;
      bestFoundId = neighborId;
    }
  }

  if (bestFoundId == Ipv4Address::GetZero()){
      NS_LOG_DEBUG("BestAngle: No suitable neighbor found according to right-hand rule.");
  } else {
      NS_LOG_DEBUG("BestAngle: Selected neighbor " << bestFoundId << " with angle " << smallestAngle);
  }

  return bestFoundId;
}

// GetAngle calculates the counter-clockwise angle (0-360) from vector Center->RefPos
// to vector Center->NodePos using atan2
double
GpsrPtable::GetAngle(Vector center, Vector refPos, Vector nodePos)
{
    const double PI = M_PI; // Use M_PI from cmath for better precision

    // Calculate vectors relative to the center point
    double refVecX = refPos.x - center.x;
    double refVecY = refPos.y - center.y;
    double nodeVecX = nodePos.x - center.x;
    double nodeVecY = nodePos.y - center.y;

    // Calculate the angle of each vector relative to the positive X-axis
    double angleRef = atan2(refVecY, refVecX);
    double angleNode = atan2(nodeVecY, nodeVecX);

    // Calculate the difference in angles
    double angleDiff = angleNode - angleRef;

    // Convert angle difference to degrees (0-360 range)
    angleDiff = angleDiff * 180.0 / PI;
    if (angleDiff < 0)
    {
        angleDiff += 360.0;
    }

    // Ensure the angle is not exactly zero if vectors are different
    // (avoids issues if refPos == nodePos, though BestAngle should skip prevHop)
    if (angleDiff == 0 && (refVecX != nodeVecX || refVecY != nodeVecY)) {
         // Return 360 instead of 0 for non-identical vectors pointing in the same direction
         // This ensures it's not selected as the minimum angle unless it's the only option.
         // Or handle this in BestAngle? Let's return 360 for now.
         return 360.0; 
    }

    // Return a very small value if the angle is extremely close to 0 but not exactly 0? No, return calculated angle.
    return angleDiff;
}

Vector
GpsrPtable::GetInvalidPosition()
{
  return Vector(-1, -1, 0);
}

// Helper function to calculate distance between two points
double
CalculateDistance(Vector a, Vector b)
{
  double dx = b.x - a.x;
  double dy = b.y - a.y;
  double distanceSq = dx * dx + dy * dy;
  return std::sqrt(distanceSq);
}

} // namespace ns3
