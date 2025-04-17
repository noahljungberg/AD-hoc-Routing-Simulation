#include "ns3/core-module.h"
#include "Simulations/StaticSimulation.hpp"
#include "Simulations/StaticSimulationGPSR.hpp"

int main(int argc, char *argv[]) {
    // Parse command line
    ns3::CommandLine cmd;
    std::string protocol = "GPSR";  // Default protocol
    cmd.AddValue("protocol", "Routing protocol to use (DSDV, GPSR)", protocol);
    cmd.Parse(argc, argv);

    // Create and run the appropriate simulation based on protocol
    if (protocol == "GPSR") {
        StaticSimulationGPSR sim(10, 100.0);  // 10 nodes, 100 seconds
        LogComponentEnable("StaticSimulationGPSR", LOG_LEVEL_INFO);
        sim.Run();
    } else {
        // Default to DSDV
        StaticSimulation sim(10, 100.0, "DSDV");  // 10 nodes, 100 seconds
        LogComponentEnable("StaticSimulation", LOG_LEVEL_INFO);
        sim.Run();
    }

    return 0;
}