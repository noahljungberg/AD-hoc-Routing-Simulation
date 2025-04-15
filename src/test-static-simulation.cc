// src/test-static-simulation.cc
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "Simulations/StaticSimulation.hpp"

using namespace ns3;

int main(int argc, char *argv[]) {
    // Enable logging
    LogComponentEnable("StaticSimulation", LOG_LEVEL_INFO);

    // Parse command line arguments
    CommandLine cmd;
    uint32_t numNodes = 10;
    double simTime = 100.0;
    cmd.AddValue("numNodes", "Number of nodes", numNodes);
    cmd.AddValue("simTime", "Simulation time", simTime);
    cmd.Parse(argc, argv);

    // Create and run the simulation
    StaticSimulation sim(numNodes, simTime);
    sim.Run();

    return 0;
}