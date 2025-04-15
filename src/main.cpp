#include "ns3/core-module.h"
#include "Simulations/StaticSimulation.hpp"

int main(int argc, char *argv[]) {
    // Parse command line
    ns3::CommandLine cmd;
    cmd.Parse(argc, argv);

    // Create and run the simulation
    StaticSimulation sim(10, 10.0);  // 10 nodes, 10 seconds
    LogComponentEnable("StaticSimulation", LOG_LEVEL_INFO);

    sim.Run();

    return 0;
}