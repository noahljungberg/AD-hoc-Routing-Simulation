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

    // Test the GPSR modules
    NS_LOG_INFO("Testing GPSR module functionality");
    Ptr<ns3::Gpsr> gpsrInstance = gpsr.Create(m_nodes.Get(0));// Create a test instance

    if (gpsrInstance) {
        NS_LOG_INFO("✓ Successfully created GPSR instance");
    } else {
        NS_LOG_ERROR("✗ Failed to create GPSR instance");
    }

    // Continue with installation
    InternetStackHelper internet;
    internet.SetRoutingHelper(gpsr);
    internet.Install(m_nodes);

    // Configure IP addressing
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    m_interfaces = ipv4.Assign(m_devices);

    // Log IP addresses
    for (uint32_t i = 0; i < m_nodes.GetN(); i++) {
        NS_LOG_INFO("Node " << i << " has IP: " << m_interfaces.GetAddress(i));
    }

    // Setup flow monitoring
    m_flowMonitor = m_flowHelper.InstallAll();
}

static void UdpEchoTxTrace(Ptr<OutputStreamWrapper> stream, Ptr<const Packet> p) {
    *stream->GetStream() << "TX: " << p->GetSize() << " bytes at " << Simulator::Now().GetSeconds() << " s\n";
}

static void UdpEchoRxTrace(Ptr<OutputStreamWrapper> stream, Ptr<const Packet> p, const Address &address) {
    *stream->GetStream() << "RX: " << p->GetSize() << " bytes at " << Simulator::Now().GetSeconds() << " s\n";
}
void StaticSimulationGPSR::ConfigureApplications() {
    NS_LOG_INFO("Setting up applications");

    // Use a simpler UDP echo server/client application
    uint16_t port = 9;

    // Create server on last node
    UdpEchoServerHelper echoServer(port);
    ApplicationContainer serverApps = echoServer.Install(m_nodes.Get(m_numNodes - 1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(m_simulationTime));

    // Create client on first node
    UdpEchoClientHelper echoClient(m_interfaces.GetAddress(m_numNodes - 1), port);
    echoClient.SetAttribute("MaxPackets", UintegerValue(200));  // Send more packets
    echoClient.SetAttribute("Interval", TimeValue(Seconds(0.5)));  // Send faster
    echoClient.SetAttribute("PacketSize", UintegerValue(512));

    ApplicationContainer clientApps = echoClient.Install(m_nodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(m_simulationTime - 1.0));

    NS_LOG_INFO("Configured UDP Echo client on node 0 sending to node " << (m_numNodes - 1));

    // Enable tracing for debugging
    AsciiTraceHelper ascii;
    Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream("gpsr-application.tr");
    clientApps.Get(0)->TraceConnectWithoutContext("Tx", MakeBoundCallback(&UdpEchoTxTrace, stream));
    serverApps.Get(0)->TraceConnectWithoutContext("Rx", MakeBoundCallback(&UdpEchoRxTrace, stream));
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
    // Safely print routing information without causing null pointer exceptions
    std::cout << "Node positions and addresses:\n";

    for (uint32_t i = 0; i < m_nodes.GetN(); i++) {
        std::cout << "Node " << i;

        // Safely get the IP address
        if (i < m_interfaces.GetN()) {
            std::cout << " IP: " << m_interfaces.GetAddress(i);
        } else {
            std::cout << " IP: unknown";
        }

        // Safely get the position
        Ptr<MobilityModel> mob = m_nodes.Get(i)->GetObject<MobilityModel>();
        if (mob) {
            Vector pos = mob->GetPosition();
            std::cout << "  Position: (" << pos.x << ", " << pos.y << ")\n";
        } else {
            std::cout << "  Position: unknown\n";
        }
    }
}