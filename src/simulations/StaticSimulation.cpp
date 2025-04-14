#include "StaticSimulation.h"

StaticSimulation::StaticSimulation(const int numNodes, const double simulationTime)
    : m_numNodes(numNodes), m_simulationTime(simulationTime) {
    m_nodes.Create(numNodes);
}

StaticSimulation::~StaticSimulation() {}

void StaticSimulation::SetupTopology() {
  m_nodes.Create(m_numNodes);  // Create nodes


}

void SetupRoutingProtocol() {


  }