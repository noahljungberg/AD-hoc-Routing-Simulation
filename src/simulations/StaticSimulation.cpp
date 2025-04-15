#include "Simulations/StaticSimulation.hpp"  // Not StaticSimulation.h

StaticSimulation::StaticSimulation(const int numNodes, const double simulationTime) {
    m_numNodes = numNodes;
    m_simulationTime = simulationTime;
}

StaticSimulation::~StaticSimulation() {}

void StaticSimulation::SetupTopology() {
    // Add your grid position allocator code here
    ns3::MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                               "MinX", ns3::DoubleValue(0.0),
                               "MinY", ns3::DoubleValue(0.0),
                               "DeltaX", ns3::DoubleValue(100.0),
                               "DeltaY", ns3::DoubleValue(100.0),
                               "GridWidth", ns3::UintegerValue(5),
                               "LayoutType", ns3::StringValue("RowFirst"));

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(m_nodes);
}

void StaticSimulation::SetupRoutingProtocol() {
    // Choose which protocol to use
    SetupDSDV();  // For now, just use DSDV
}

void StaticSimulation::ConfigureApplications() {
    // Basic echo client/server setup
    uint16_t port = 9;

    // Create a server on the last node
    ns3::UdpEchoServerHelper echoServer(port);
    ns3::ApplicationContainer serverApps = echoServer.Install(m_nodes.Get(m_numNodes - 1));
    serverApps.Start(ns3::Seconds(1.0));
    serverApps.Stop(ns3::Seconds(m_simulationTime));

    // Create a client on the first node
    ns3::UdpEchoClientHelper echoClient(m_interfaces.GetAddress(m_numNodes - 1), port);
    echoClient.SetAttribute("MaxPackets", ns3::UintegerValue(1));
    echoClient.SetAttribute("Interval", ns3::TimeValue(ns3::Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", ns3::UintegerValue(1024));

    ns3::ApplicationContainer clientApps = echoClient.Install(m_nodes.Get(0));
    clientApps.Start(ns3::Seconds(2.0));
    clientApps.Stop(ns3::Seconds(m_simulationTime));
}

void StaticSimulation::RunSimulation() {
    ns3::Simulator::Stop(ns3::Seconds(m_simulationTime));
    ns3::Simulator::Run();
    ns3::Simulator::Destroy();
}

void StaticSimulation::CollectResults() {
    // You can add result collection logic here
    std::cout << "Simulation completed successfully." << std::endl;
}