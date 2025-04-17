#include "Simulations/StaticSimulation.hpp"

NS_LOG_COMPONENT_DEFINE("StaticSimulation");
StaticSimulation::StaticSimulation(const int numNodes, const double simulationTime, const std::string& routingProtocol) {
    m_numNodes = numNodes;
    m_simulationTime = simulationTime;
    m_routingProtocol = routingProtocol;
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
    if (m_routingProtocol == "DSDV") SetupDSDV();
    else if  (m_routingProtocol == "DSR") SetupDSR();
    else if (m_routingProtocol == "GPSR")SetupGPSR();
    else NS_LOG_ERROR("Invalid routing protocol selected.");


    m_flowMonitor = m_flowHelper.InstallAll();
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
    NS_LOG_INFO("Simulation completed. Collecting results...");

    // Get FlowMonitor statistics
    m_flowMonitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(m_flowHelper.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = m_flowMonitor->GetFlowStats();

    // Print flow statistics
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin(); i != stats.end(); ++i) {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);

        std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
        std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
        std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
        std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds()) / 1000 << " Kbps\n";
        std::cout << "  Mean Delay: " << i->second.delaySum.GetSeconds() / i->second.rxPackets << " seconds\n";
        std::cout << "  Packet Loss: " << 100.0 * (i->second.txPackets - i->second.rxPackets) / i->second.txPackets << "%\n";
    }

    // Save detailed FlowMonitor results to XML
    m_flowMonitor->SerializeToXmlFile(m_routingProtocol + "-flow-stats.xml", true, true);
}