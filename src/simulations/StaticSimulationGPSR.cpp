#include "Simulations/StaticSimulationGPSR.hpp"
#include "ns3/log.h"
#include "gpsr/gpsr-helper.hpp"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/udp-echo-helper.h"
#include "ns3/flow-monitor.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/string.h"
#include "ns3/names.h"
#include "ns3/trace-helper.h"
#include "ns3/packet-sink-helper.h"

NS_LOG_COMPONENT_DEFINE("StaticSimulationGPSR");

StaticSimulationGPSR::StaticSimulationGPSR(const int numNodes, const double simulationTime) {
    m_numNodes = numNodes;
    m_simulationTime = simulationTime;
    m_routingProtocol = "GPSR";
}

StaticSimulationGPSR::~StaticSimulationGPSR() {}

void StaticSimulationGPSR::SetupTopology() {
    // Use a grid layout for clearer routing paths
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

    // Log node positions
    for (uint32_t i = 0; i < m_nodes.GetN(); i++) {
        Ptr<MobilityModel> mob = m_nodes.Get(i)->GetObject<MobilityModel>();
        Vector pos = mob->GetPosition();
        NS_LOG_INFO("Node " << i << " position: x=" << pos.x << ", y=" << pos.y);

        // Give meaningful names to nodes for easier debugging
        std::ostringstream nodeName;
        nodeName << "Node-" << i;
        Names::Add(nodeName.str(), m_nodes.Get(i));
    }

    // Enable packet tracing to help debug
    AsciiTraceHelper ascii;
    Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream("gpsr-trace.tr");
    mobility.EnableAsciiAll(stream);
}

void StaticSimulationGPSR::SetupRoutingProtocol() {
    NS_LOG_INFO("Setting up GPSR routing protocol");

    // Create and configure GPSR helper
    GpsrHelper gpsr;

    // You can set specific GPSR parameters here if needed
    // Example: gpsr.Set("HelloInterval", TimeValue(Seconds(2.0)));

    // Install the internet stack with GPSR
    InternetStackHelper internet;
    internet.SetRoutingHelper(gpsr);
    internet.Install(m_nodes);

    // Configure IP addressing
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    m_interfaces = ipv4.Assign(m_devices);

    // Log IP addresses for easier debugging
    for (uint32_t i = 0; i < m_nodes.GetN(); i++) {
        NS_LOG_INFO("Node " << i << " has IP: " <<
                    m_interfaces.GetAddress(i));
    }

    // Setup flow monitoring
    m_flowMonitor = m_flowHelper.InstallAll();
    m_flowMonitor->SetAttribute("DelayBinWidth", DoubleValue(0.001));
    m_flowMonitor->SetAttribute("JitterBinWidth", DoubleValue(0.001));
    m_flowMonitor->SetAttribute("PacketSizeBinWidth", DoubleValue(20));
}

void StaticSimulationGPSR::ConfigureApplications() {
    NS_LOG_INFO("Setting up applications");

    // Create a more bandwidth-intensive application to better test routing
    uint16_t port = 9;

    // Set up a packet sink on the last node
    PacketSinkHelper sink("ns3::UdpSocketFactory",
                         InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApps = sink.Install(m_nodes.Get(m_numNodes - 1));
    sinkApps.Start(Seconds(1.0));
    sinkApps.Stop(Seconds(m_simulationTime));

    // Set up an OnOff source on the first node
    OnOffHelper onoff("ns3::UdpSocketFactory",
                     InetSocketAddress(m_interfaces.GetAddress(m_numNodes - 1), port));
    onoff.SetConstantRate(DataRate("10kb/s"));
    onoff.SetAttribute("PacketSize", UintegerValue(1024));
    onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
    onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));

    ApplicationContainer sourceApps = onoff.Install(m_nodes.Get(0));
    sourceApps.Start(Seconds(2.0));
    sourceApps.Stop(Seconds(m_simulationTime - 1.0));

    NS_LOG_INFO("Configured OnOff source on node 0 sending to node " << (m_numNodes - 1));

    // Add a second flow in the opposite direction
    OnOffHelper onoff2("ns3::UdpSocketFactory",
                      InetSocketAddress(m_interfaces.GetAddress(0), port + 1));
    onoff2.SetConstantRate(DataRate("5kb/s"));
    onoff2.SetAttribute("PacketSize", UintegerValue(512));

    PacketSinkHelper sink2("ns3::UdpSocketFactory",
                          InetSocketAddress(Ipv4Address::GetAny(), port + 1));
    ApplicationContainer sinkApps2 = sink2.Install(m_nodes.Get(0));
    ApplicationContainer sourceApps2 = onoff2.Install(m_nodes.Get(m_numNodes - 1));

    sinkApps2.Start(Seconds(1.0));
    sourceApps2.Start(Seconds(3.0));
    sinkApps2.Stop(Seconds(m_simulationTime));
    sourceApps2.Stop(Seconds(m_simulationTime - 1.0));

    NS_LOG_INFO("Configured second flow from node " << (m_numNodes - 1) << " to node 0");
}

void StaticSimulationGPSR::RunSimulation() {
    NS_LOG_INFO("Starting GPSR simulation for " << m_simulationTime << " seconds");

    // Create tracers to monitor GPSR activity
    AsciiTraceHelper ascii;
    Ptr<OutputStreamWrapper> routingStream = ascii.CreateFileStream("gpsr-routing.tr");

    // Destroy simulator first to clear any previous state
    Simulator::Destroy();

    // Enable GPSR debug output
    LogComponentEnable("Gpsr", LOG_LEVEL_DEBUG);
    LogComponentEnable("GpsrHelper", LOG_LEVEL_INFO);  // This will work after adding the definition
    LogComponentEnable("GpsrPtable", LOG_LEVEL_INFO);
    LogComponentEnable("GpsrRqueue", LOG_LEVEL_INFO);
    LogComponentEnable("GpsrPacket", LOG_LEVEL_INFO);

    // Schedule packet capture operations at specific times
    Simulator::Schedule(Seconds(5.0), &StaticSimulationGPSR::CaptureBriefState, this);
    Simulator::Schedule(Seconds(m_simulationTime/2), &StaticSimulationGPSR::CaptureBriefState, this);

    // Run the simulation
    Simulator::Stop(Seconds(m_simulationTime));
    Simulator::Run();
    Simulator::Destroy();
}

void StaticSimulationGPSR::CaptureBriefState() {
    NS_LOG_INFO("---------------- Simulation State at " << Simulator::Now().GetSeconds() << "s ----------------");

    // Check the packet counters to see if packets are moving through the system
    FlowMonitorHelper flowMonHelper;
    Ptr<FlowMonitor> monitor = flowMonHelper.GetMonitor();

    monitor->CheckForLostPackets();
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

    if (stats.empty()) {
        NS_LOG_WARN("No flows detected yet!");
    } else {
        NS_LOG_INFO("Flow stats snapshot:");
        for (auto const& flow : stats) {
            NS_LOG_INFO("  Flow " << flow.first << ": "
                       << "Tx=" << flow.second.txPackets
                       << ", Rx=" << flow.second.rxPackets);
        }
    }
}

void StaticSimulationGPSR::CollectResults() {
    NS_LOG_INFO("Simulation completed. Collecting results...");

    // Get FlowMonitor statistics
    m_flowMonitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(m_flowHelper.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = m_flowMonitor->GetFlowStats();

    uint32_t totalTxPackets = 0;
    uint32_t totalRxPackets = 0;

    // Print flow statistics
    if (stats.empty()) {
        std::cout << "ERROR: No flows were detected in the simulation!\n";
        std::cout << "       This means GPSR routing is not working properly.\n";
    } else {
        std::cout << "\n*** GPSR Routing Results ***\n\n";

        for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin(); i != stats.end(); ++i) {
            Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
            totalTxPackets += i->second.txPackets;
            totalRxPackets += i->second.rxPackets;

            std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
            std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
            std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
            if (i->second.rxPackets > 0) {
                std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds()) / 1000 << " Kbps\n";
                std::cout << "  Mean Delay: " << i->second.delaySum.GetSeconds() / i->second.rxPackets << " seconds\n";
            }
            std::cout << "  Packet Loss: " << 100.0 * (i->second.txPackets - i->second.rxPackets) / i->second.txPackets << "%\n\n";
        }

        // Summary statistics
        std::cout << "*** Summary ***\n";
        std::cout << "Total Transmitted Packets: " << totalTxPackets << "\n";
        std::cout << "Total Received Packets: " << totalRxPackets << "\n";
        if (totalTxPackets > 0) {
            std::cout << "Overall Packet Delivery Ratio: " << 100.0 * totalRxPackets / totalTxPackets << "%\n";
        }
    }

    // Save detailed FlowMonitor results to XML
    m_flowMonitor->SerializeToXmlFile(m_routingProtocol + "-flow-stats.xml", true, true);

    // Add routing table state
    std::cout << "\n*** GPSR Routing Tables ***\n";
    PrintRoutingTables();
}

void StaticSimulationGPSR::PrintRoutingTables() {
    // This is a simplified version - in a real implementation you'd
    // access the actual GPSR tables from each node
    for (uint32_t i = 0; i < m_nodes.GetN(); i++) {
        Ptr<Ipv4> ipv4 = m_nodes.Get(i)->GetObject<Ipv4>();
        std::cout << "Node " << i << " IP: " << m_interfaces.GetAddress(i) << "\n";

        // In a complete implementation, you would access the GPSR position tables here
        // For now, just print node positions which are key to GPSR
        Ptr<MobilityModel> mob = m_nodes.Get(i)->GetObject<MobilityModel>();
        Vector pos = mob->GetPosition();
        std::cout << "  Position: (" << pos.x << ", " << pos.y << ")\n";
    }
}