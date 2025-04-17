#include "Simulations/StaticSimulationGPSR.hpp"
#include "gpsr/gpsr-helper.hpp"  // Include GPSR helper

NS_LOG_COMPONENT_DEFINE("StaticSimulationGPSR");

StaticSimulationGPSR::StaticSimulationGPSR(const int numNodes, const double simulationTime) {
    m_numNodes = numNodes;
    m_simulationTime = simulationTime;
    m_routingProtocol = "GPSR";  // Set protocol name for logging
}

StaticSimulationGPSR::~StaticSimulationGPSR() {}

void StaticSimulationGPSR::SetupTopology() {
    // Grid position allocator - same as in StaticSimulation
    ns3::MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                "MinX", ns3::DoubleValue(0.0),
                                "MinY", ns3::DoubleValue(0.0),
                                "DeltaX", ns3::DoubleValue(100.0),
                                "DeltaY", ns3::DoubleValue(100.0),
                                "GridWidth", ns3::UintegerValue(5),
                                "LayoutType", ns3::StringValue("RowFirst"));

    // Set static mobility model
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(m_nodes);

    // Log node positions for debugging GPSR
    for (uint32_t i = 0; i < m_nodes.GetN(); i++) {
        Ptr<MobilityModel> mob = m_nodes.Get(i)->GetObject<MobilityModel>();
        NS_LOG_INFO("Node " << i << " position: x=" <<
                    mob->GetPosition().x << ", y=" << mob->GetPosition().y);
    }
}

void StaticSimulationGPSR::SetupRoutingProtocol() {
    NS_LOG_INFO("Setting up GPSR routing protocol");

    // Handle potential GPSR setup issues gracefully

        // Create a GPSR helper
        GpsrHelper gpsr;

        // Optional: Configure GPSR parameters if needed
        // gpsr.Set("HelloInterval", TimeValue(Seconds(1.0)));

        // Install the GPSR routing protocol
        ns3::InternetStackHelper internet;
        internet.SetRoutingHelper(gpsr);
        internet.Install(m_nodes);

        // Configure IP addressing
        ns3::Ipv4AddressHelper ipv4;
        ipv4.SetBase("10.1.1.0", "255.255.255.0");
        m_interfaces = ipv4.Assign(m_devices);



    // Install FlowMonitor
    m_flowMonitor = m_flowHelper.InstallAll();
}

void StaticSimulationGPSR::ConfigureApplications() {
    // Basic echo client/server setup - same as StaticSimulation
    uint16_t port = 9;

    // Create a server on the last node
    ns3::UdpEchoServerHelper echoServer(port);
    ns3::ApplicationContainer serverApps = echoServer.Install(m_nodes.Get(m_numNodes - 1));
    serverApps.Start(ns3::Seconds(1.0));
    serverApps.Stop(ns3::Seconds(m_simulationTime));

    // Create a client on the first node
    ns3::UdpEchoClientHelper echoClient(m_interfaces.GetAddress(m_numNodes - 1), port);
    echoClient.SetAttribute("MaxPackets", ns3::UintegerValue(10));
    echoClient.SetAttribute("Interval", ns3::TimeValue(ns3::Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", ns3::UintegerValue(1024));

    ns3::ApplicationContainer clientApps = echoClient.Install(m_nodes.Get(0));
    clientApps.Start(ns3::Seconds(2.0));
    clientApps.Stop(ns3::Seconds(m_simulationTime));
}

void StaticSimulationGPSR::RunSimulation() {
    NS_LOG_INFO("Starting GPSR simulation for " << m_simulationTime << " seconds");

    // Make sure to handle any existing timer issues
    ns3::Simulator::Destroy();

    ns3::Simulator::Stop(ns3::Seconds(m_simulationTime));
    ns3::Simulator::Run();
    ns3::Simulator::Destroy();
}

void StaticSimulationGPSR::CollectResults() {
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