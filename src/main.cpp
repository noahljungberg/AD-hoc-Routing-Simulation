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

    // Set up logging with reduced verbosity
    if (debug) {
        // Only enable selective components if debug is requested
        ns3::LogComponentEnable("StaticSimulationGPSR", LOG_LEVEL_INFO);
        ns3::LogComponentEnable("Gpsr", LOG_LEVEL_DEBUG);
        ns3::LogComponentEnable("GpsrHelper", LOG_LEVEL_INFO);
        ns3::LogComponentEnable("GpsrPtable", LOG_LEVEL_DEBUG);
    } else {
        // Otherwise keep logging minimal
        ns3::LogComponentEnable("StaticSimulationGPSR", LOG_LEVEL_WARN);
        ns3::LogComponentEnable("Gpsr", LOG_LEVEL_WARN);
        ns3::LogComponentEnable("GpsrPtable", LOG_LEVEL_WARN);
    }

    try {
        // Create and run the appropriate simulation
        if (protocol == "GPSR") {
            std::cout << "Running GPSR routing simulation...\n";
            StaticSimulationGPSR sim(10, 30.0);  // 10 nodes, 30 seconds
            sim.Run();
        } else {
            std::cout << "Running " << protocol << " routing simulation...\n";
            StaticSimulation sim(10, 30.0, "DSDV");
            sim.Run();
        }
    } catch (const std::exception& e) {
        std::cerr << "Simulation failed with error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}