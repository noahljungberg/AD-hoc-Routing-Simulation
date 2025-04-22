#include "Simulations/StaticSimulationGPSR.hpp"

#include <gpsr/gpsr.h>

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
    // Use a grid layout for clearer routing paths but with closer spacing
    // to ensure nodes can discover each other
    ns3::MobilityHelper mobility;

    // IMPORTANT FIX: Reduce node spacing to ensure nodes are within radio range
    // Default WiFi range is around 100-150m, so node spacing should be < 100m
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                               "MinX", ns3::DoubleValue(0.0),
                               "MinY", ns3::DoubleValue(0.0),
                               "DeltaX", ns3::DoubleValue(75.0),  // Changed from 100.0 to 75.0
                               "DeltaY", ns3::DoubleValue(75.0),  // Changed from 100.0 to 75.0
                               "GridWidth", ns3::UintegerValue(5),
                               "LayoutType", ns3::StringValue("RowFirst"));

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(m_nodes);

    // Log node positions but with less verbosity
    NS_LOG_INFO("Node positions:");
    for (uint32_t i = 0; i < m_nodes.GetN(); i++) {
        Ptr<MobilityModel> mob = m_nodes.Get(i)->GetObject<MobilityModel>();
        Vector pos = mob->GetPosition();
        NS_LOG_INFO("Node " << i << " at (" << pos.x << "," << pos.y << ")");

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

    // IMPORTANT: Set GPSR parameters
    // Increase hello interval for faster neighbor discovery
    gpsr.Set("HelloInterval", TimeValue(Seconds(1.0)));

    // Install internet stack with GPSR routing
    InternetStackHelper internet;
    internet.SetRoutingHelper(gpsr);
    internet.Install(m_nodes);

    // Configure IP addressing
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    m_interfaces = ipv4.Assign(m_devices);

    // Log IP addresses with less verbosity
    NS_LOG_INFO("Node IP addresses:");
    for (uint32_t i = 0; i < m_nodes.GetN(); i++) {
        NS_LOG_INFO("Node " << i << ": " << m_interfaces.GetAddress(i));
    }

    // Install flow monitoring
    m_flowMonitor = m_flowHelper.InstallAll();

    // Critical: Install GPSR
    gpsr.Install();
}

static void UdpEchoTxTrace(Ptr<OutputStreamWrapper> stream, Ptr<const Packet> p) {
    *stream->GetStream() << "TX: " << p->GetSize() << " bytes at " << Simulator::Now().GetSeconds() << " s\n";
}

static void UdpEchoRxTrace(Ptr<OutputStreamWrapper> stream, Ptr<const Packet> p) {
    *stream->GetStream() << "RX: " << p->GetSize() << " bytes at " << Simulator::Now().GetSeconds() << " s\n";
}
void StaticSimulationGPSR::ConfigureApplications() {
    NS_LOG_INFO("Setting up applications");

    // Use a simpler UDP echo server/client application
    uint16_t port = 9;

    // Create server on last node (node 9)
    UdpEchoServerHelper echoServer(port);
    ApplicationContainer serverApps = echoServer.Install(m_nodes.Get(m_numNodes - 1));
    serverApps.Start(Seconds(5.0));  // Start later to allow neighbor discovery
    serverApps.Stop(Seconds(m_simulationTime));

    // Create client on first node (node 0)
    UdpEchoClientHelper echoClient(m_interfaces.GetAddress(m_numNodes - 1), port);
    echoClient.SetAttribute("MaxPackets", UintegerValue(200));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0))); // Send slower
    echoClient.SetAttribute("PacketSize", UintegerValue(512));

    ApplicationContainer clientApps = echoClient.Install(m_nodes.Get(0));
    clientApps.Start(Seconds(10.0));  // Start even later to allow neighbor discovery
    clientApps.Stop(Seconds(m_simulationTime - 1.0));

    NS_LOG_INFO("Configured UDP Echo client on node 0 sending to node " << (m_numNodes - 1));
}


void StaticSimulationGPSR::RunSimulation() {
    NS_LOG_INFO("Starting GPSR simulation for " << m_simulationTime << " seconds");

    // Enable GPSR debug output but at a lower level
    LogComponentEnable("Gpsr", LOG_LEVEL_DEBUG);  // Changed from LOG_LEVEL_DEBUG to LOG_LEVEL_INFO
    LogComponentEnable("GpsrPtable", LOG_LEVEL_DEBUG);  // Changed from LOG_LEVEL_INFO to LOG_LEVEL_DEBUG

    // Schedule state capture at specific times
    Simulator::Schedule(Seconds(5.0), &StaticSimulationGPSR::CaptureBriefState, this);
    Simulator::Schedule(Seconds(15.0), &StaticSimulationGPSR::CaptureBriefState, this);
    Simulator::Schedule(Seconds(m_simulationTime/2), &StaticSimulationGPSR::CaptureBriefState, this);

    // Run the simulation
    Simulator::Stop(Seconds(m_simulationTime));
    Simulator::Run();
}

void StaticSimulationGPSR::CaptureBriefState() {
    NS_LOG_INFO("-------- Simulation State at " << Simulator::Now().GetSeconds() << "s --------");

    // Check for nodes and their neighbors
    for (uint32_t i = 0; i < m_nodes.GetN(); i++) {
        Ptr<Node> node = m_nodes.Get(i);
        Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
        NS_LOG_INFO("Node " << i << " at " << ipv4->GetAddress(1, 0).GetLocal());

        // We can't easily access neighbor tables directly in this function,
        // but we can print node positions to help debug
        Ptr<MobilityModel> mobility = node->GetObject<MobilityModel>();
        if (mobility) {
            Vector pos = mobility->GetPosition();
            NS_LOG_INFO("  Position: (" << pos.x << "," << pos.y << ")");
        }
    }

    // Check packet stats
    FlowMonitorHelper flowMonHelper;
    Ptr<FlowMonitor> monitor = m_flowMonitor;

    monitor->CheckForLostPackets();
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

    if (stats.empty()) {
        NS_LOG_INFO("No flows detected yet");
    } else {
        for (auto const& flow : stats) {
            NS_LOG_INFO("Flow " << flow.first << ": "
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

    // Print flow statistics with less verbosity
    std::cout << "\n*** GPSR Routing Results ***\n";

    if (stats.empty()) {
        std::cout << "ERROR: No flows detected in the simulation!\n";
    } else {
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
            std::cout << "  Packet Loss: " << 100.0 * (i->second.txPackets - i->second.rxPackets) / i->second.txPackets << "%\n";
        }

        // Summary statistics
        std::cout << "*** Summary ***\n";
        std::cout << "Total Transmitted Packets: " << totalTxPackets << "\n";
        std::cout << "Total Received Packets: " << totalRxPackets << "\n";
        if (totalTxPackets > 0) {
            std::cout << "Overall Packet Delivery Ratio: " << 100.0 * totalRxPackets / totalTxPackets << "%\n";
        }
    }

    // Print node positions in a more compact way
    std::cout << "\n*** GPSR Routing Tables ***\n";
    std::cout << "Node positions and addresses:\n";
    for (uint32_t i = 0; i < m_nodes.GetN(); i++) {
        Ptr<Node> node = m_nodes.Get(i);
        std::cout << "Node " << i << " IP: ";
        if (node) {
            Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
            if (ipv4 && ipv4->GetNInterfaces() > 1) {
                std::cout << ipv4->GetAddress(1, 0).GetLocal();
            } else {
                std::cout << "unknown";
            }

            Ptr<MobilityModel> mob = node->GetObject<MobilityModel>();
            if (mob) {
                Vector pos = mob->GetPosition();
                std::cout << "  Position: (" << pos.x << ", " << pos.y << ")\n";
            } else {
                std::cout << "  Position: unknown\n";
            }
        } else {
            std::cout << "unknown  Position: unknown\n";
        }
    }

    Simulator::Destroy();
}

void StaticSimulationGPSR::PrintRoutingTables() {
    // Safely print routing information without causing null pointer exceptions
    std::cout << "Node positions and addresses:\n";

    for (uint32_t i = 0; i < m_nodes.GetN(); i++) {
        Ptr<Node> node = m_nodes.Get(i);
        std::cout << "Node " << i;

        // Get IP address directly from the node
        if (node) {
            Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
            if (ipv4 && ipv4->GetNInterfaces() > 1) { // Interface 0 is loopback
                std::cout << " IP: " << ipv4->GetAddress(1, 0).GetLocal();
            } else {
                std::cout << " IP: unknown (no IPv4)";
            }

            // Get position
            Ptr<MobilityModel> mob = node->GetObject<MobilityModel>();
            if (mob) {
                Vector pos = mob->GetPosition();
                std::cout << "  Position: (" << pos.x << ", " << pos.y << ")\n";
            } else {
                std::cout << "  Position: unknown (no mobility model)\n";
            }
        } else {
            std::cout << " IP: unknown (no node)  Position: unknown\n";
        }
    }
}