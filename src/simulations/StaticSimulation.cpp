#include "StaticSimulation.hpp"

StaticSimulation::StaticSimulation(const int numNodes, const double simulationTime)
    : m_numNodes(numNodes), m_simulationTime(simulationTime) {
    m_nodes.Create(numNodes);
}

StaticSimulation::~StaticSimulation() {}

void StaticSimulation::SetupTopology() {
    m_nodes.Create(m_numNodes);  // Create nodes

    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                        "MinX", DoubleValue(0.0),           // Starting X coordinate
                        "MinY", DoubleValue(0.0),           // Starting Y coordinate
                        "DeltaX", DoubleValue(100.0),       // Distance between nodes in X-direction
                        "DeltaY", DoubleValue(100.0),       // Distance between nodes in Y-direction
                        "GridWidth", UintegerValue(5),      // Number of nodes per row
                        "LayoutType", StringValue("RowFirst")); // Fill rows before columns

    // set a mobility model that keeps nodes in constant position
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);
}

void SetupRoutingProtocol() {
    setupDSDV();  // Configure DSDV routing protocol
}

void StaticSimulation::ConfigureApplications()
{


    // create an application: set up a udp echo client on node 9 that sends a packet to the server
    UdpEchoClientHelper echoClient(interfaces.GetAddress(0), 9); // Target server at node 0, port 9
    echoClient.SetAttribute("MaxPackets", UintegerValue(1)); // send only one packet
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0))); // 1 second interval
    echoClient.SetAttribute("PacketSize", UintegerValue(1024)); // packet size of 1024 bytes

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(9)); // install the client on node 9

    clientApps.Start(Seconds(2.0)); // start the client at 2 seconds into the simulation
    clientApps.Stop(Seconds(10.0)); // stop the client at 10 seconds into the simulation

    // enable tracing: this writes packet-level information to a pcap file
    wifiPhy.EnablePcap("dsdv-simulation", devices); // enable pcap tracing for the devices
}

void StaticSimulation::RunSimulation() {
    Simulator::Run();  // Start the simulation
    Simulator::Destroy();  // Clean up after the simulation
}

void StaticSimulation::CollectResults() {
    // Collect and process simulation results
    // This could include analyzing packet delivery ratios, delays, etc.
    // For now, we will just print a message
    std::cout << "Simulation completed. Results collected." << std::endl;
}