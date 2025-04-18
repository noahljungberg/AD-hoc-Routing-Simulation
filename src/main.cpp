#include "ns3/core-module.h"
#include "Simulations/StaticSimulation.hpp"
#include "Simulations/StaticSimulationGPSR.hpp"

int main(int argc, char *argv[]) {
    // Parse command line
    ns3::CommandLine cmd;
    std::string protocol = "GPSR";  // Default to GPSR
    bool debug = false;

    cmd.AddValue("protocol", "Routing protocol to use (DSDV, GPSR)", protocol);
    cmd.AddValue("debug", "Enable debug mode with verbose logging", debug);
    cmd.Parse(argc, argv);

    // Set up additional logging if debug is enabled
    if (debug) {
        ns3::LogComponentEnableAll(ns3::LOG_PREFIX_ALL);
        ns3::LogComponentEnableAll(ns3::LOG_LEVEL_INFO);
    }

    try {
        // Create and run the appropriate simulation
        if (protocol == "GPSR") {
            LogComponentEnable("StaticSimulationGPSR", LOG_LEVEL_INFO);
            LogComponentEnable("Gpsr", LOG_LEVEL_DEBUG);
            LogComponentEnable("GpsrHelper", LOG_LEVEL_INFO);

            StaticSimulationGPSR sim(10, 30.0);  // 10 nodes, 30 seconds
            sim.Run();
        } else {
            LogComponentEnable("StaticSimulation", LOG_LEVEL_INFO);
            StaticSimulation sim(10, 30.0, "DSDV");
            sim.Run();
        }
    } catch (const std::exception& e) {
        std::cerr << "Simulation failed with error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}