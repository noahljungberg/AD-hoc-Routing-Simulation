#include "gpsr/gpsr-ptable.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <algorithm>
#include <cmath>
#include <complex>
#include "ns3/node-list.h"
#include "ns3/node.h"
#include "ns3/mobility-model.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("GpsrPtable");

GpsrPtable::GpsrPtable() :
  m_entryLifetime(Seconds(2)) // FIXME: Make this parametrizable based on hello interval
{
  m_txErrorCallback = MakeCallback(&GpsrPtable::ProcessTxError, this);
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
  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i = m_table.find(id);
  if (i != m_table.end()) {
    return i->second.first;
  }

  // If not found in table, try to get it from node list
  for (NodeList::Iterator i = NodeList::Begin(); i != NodeList::End(); i++) {
    Ptr<Node> node = *i;
    if (node->GetObject<Ipv4>() &&
        node->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() == id) {
      Ptr<MobilityModel> mobility = node->GetObject<MobilityModel>();
      if (mobility) {
        return mobility->GetPosition();
      }
        }
  }

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

Callback<void, WifiMacHeader const &>
GpsrPtable::GetTxErrorCallback() const
{
  return m_txErrorCallback;
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
GpsrPtable::BestAngle(Vector prevHop, Vector nodePos)
{
  Purge();

  if (m_table.empty()) {
    NS_LOG_DEBUG("BestAngle table is empty; Position: " << nodePos);
    return Ipv4Address::GetZero();
  }

  double tmpAngle;
  Ipv4Address bestFoundId = Ipv4Address::GetZero();
  double bestFoundAngle = 360;

  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i;
  for (i = m_table.begin(); i != m_table.end(); ++i) {
    tmpAngle = GetAngle(nodePos, prevHop, i->second.first);
    if (bestFoundAngle > tmpAngle && tmpAngle != 0) {
      bestFoundId = i->first;
      bestFoundAngle = tmpAngle;
    }
  }

  if (bestFoundId == Ipv4Address::GetZero() && !m_table.empty()) {
    // If no suitable neighbor found, just use the first one
    bestFoundId = m_table.begin()->first;
  }

  return bestFoundId;
}

  double
  GpsrPtable::GetAngle(Vector center, Vector refPos, Vector node)
{
  const double PI = 3.14159265358979323846;

  // Convert to complex numbers for easier calculation
  std::complex<double> A(center.x, center.y);
  std::complex<double> B(node.x, node.y);
  std::complex<double> C(refPos.x, refPos.y);

  std::complex<double> AB = B - A;
  std::complex<double> AC = C - A;

  // Normalize vectors
  AB = std::complex<double>(std::real(AB)/std::abs(AB), std::imag(AB)/std::abs(AB));
  AC = std::complex<double>(std::real(AC)/std::abs(AC), std::imag(AC)/std::abs(AC));

  // Calculate angle
  std::complex<double> tmp = std::log(AC/AB);
  std::complex<double> Angle = tmp * std::complex<double>(0.0, -1.0);
  Angle *= (180/PI);

  double angle = std::real(Angle);
  if (angle < 0) {
    angle = 360 + angle;
  }

  return angle;
}

Vector
GpsrPtable::GetInvalidPosition()
{
  return Vector(-1, -1, 0);
}

void
GpsrPtable::ProcessTxError(WifiMacHeader const & hdr)
{
  // This could be used to handle link failures detected at the MAC layer
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