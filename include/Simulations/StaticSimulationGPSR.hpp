#ifndef STATICSIMULATIONGPSR_HPP
#define STATICSIMULATIONGPSR_HPP

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include <iostream>
#include "AbstractSimulation.hpp"

/**
 * StaticSimulationGPSR class
 * This class is derived from AbstractSimulation and implements the methods
 * to set up a static simulation environment with GPSR routing.
 * It creates a grid of nodes, configures GPSR routing protocol,
 * sets up applications, and runs the simulation.
 */
class StaticSimulationGPSR : public AbstractSimulation {
public:
    /**
     * Constructor
     * @param numNodes Number of nodes in the simulation
     * @param simulationTime Duration of the simulation in seconds
     */
    StaticSimulationGPSR(const int numNodes, const double simulationTime);

    /**
     * Destructor
     */
    ~StaticSimulationGPSR() override;

protected:
    /**
     * Set up the network topology (grid of nodes)
     */
    void SetupTopology() override;

    /**
     * Configure GPSR routing protocol on all nodes
     */
    void SetupRoutingProtocol() override;

    /**
     * Set up applications (Echo server/client)
     */
    void ConfigureApplications() override;

    /**
     * Run the simulation
     */
    void RunSimulation() override;

    /**
     * Collect and display simulation results
     */
    void CollectResults() override;
};

#endif // STATICSIMULATIONGPSR_HPP