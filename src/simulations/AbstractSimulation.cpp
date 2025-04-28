#include "Simulations/AbstractSimulation.hpp"
#include <gpsr/gpsr.h>

// To make code cleaner
using namespace ns3;

void AbstractSimulation::SetupNetwork() {
    m_nodes.Create(m_numNodes);

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211b);

    // Create a simpler channel with extended range
    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");

    // Use RangePropagationLossModel for simplicity with a shorter range
    // wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel",
    //                               "MaxRange", DoubleValue(150.0)); // Reduced range to 150m
    // Switch to Friis model to simplify and likely increase range
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel",
                                   "Frequency", ns3::DoubleValue(2.4e9)); // Assuming 2.4 GHz WiFi

    // Create and configure PHY with higher power
    YansWifiPhyHelper wifiPhy;
    wifiPhy.SetChannel(wifiChannel.Create());
    wifiPhy.Set("TxPowerStart", DoubleValue(20.0)); // Higher power in dBm
    wifiPhy.Set("TxPowerEnd", DoubleValue(20.0));   // Higher power in dBm
    wifiPhy.Set("RxGain", DoubleValue(5.0));       // Increased receiver gain

    wifiPhy.SetErrorRateModel("ns3::YansErrorRateModel");

    // Create and configure MAC
    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");

    // Install on nodes
    m_devices = wifi.Install(wifiPhy, wifiMac, m_nodes);

    AsciiTraceHelper ascii;
    // Ensure both Tx and Rx PHY traces are enabled
    wifiPhy.EnableAsciiAll(ascii.CreateFileStream("wifi-phy-trace.tr"));
    // wifiPhy.EnablePcapAll("simulation-pcap"); // Keep pcap enabled if desired
}

void AbstractSimulation::SetupDSDV() {
    DsdvHelper dsdv;
    InternetStackHelper internet;
    internet.SetRoutingHelper(dsdv);
    internet.Install(m_nodes);

    // Configure IP addressing
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    m_interfaces = ipv4.Assign(m_devices);
}

// Add stubs for the other protocol methods
void AbstractSimulation::SetupDSR() {
    // pass
}

void AbstractSimulation::SetupGPSR() {
    // Create a GPSR helper
    GpsrHelper gpsr;

    // Install the GPSR routing protocol on all nodes
    ns3::InternetStackHelper internet;
    internet.SetRoutingHelper(gpsr);
    internet.Install(m_nodes);

    // Configure IP addressing
    ns3::Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    m_interfaces = ipv4.Assign(m_devices);

    // Print node positions and GPSR neighbor tables
    std::cout << "\n*** GPSR Routing Tables & Node Info ***\n";
    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);

    for (uint32_t i = 0; i < m_nodes.GetN(); i++) {
        Ptr<Node> node = m_nodes.Get(i);
        std::cout << "\n--- Node " << i << " ---\nIP: ";
        if (node) {
            Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
            if (ipv4 && ipv4->GetNInterfaces() > 1) {
                // Assuming interface 1 is the WiFi interface
                std::cout << ipv4->GetAddress(1, 0).GetLocal();
            } else {
                std::cout << "unknown";
            }

            Ptr<MobilityModel> mob = node->GetObject<MobilityModel>();
            if (mob) {
                Vector pos = mob->GetPosition();
                std::cout << " | Position: (" << pos.x << ", " << pos.y << ")\n";
            } else {
                std::cout << " | Position: unknown\n";
            }

            // Get the GPSR routing protocol and print its table
            if (ipv4) {
                // We need to query the routing protocol list
                Ptr<Ipv4RoutingProtocol> rp = ipv4->GetRoutingProtocol ();
                // Assuming GPSR is the first protocol if list routing is used,
                // or the only one otherwise. A more robust way might iterate.
                Ptr<ns3::Gpsr> gpsrProto = DynamicCast<ns3::Gpsr>(rp);
                if (gpsrProto) {
                    gpsrProto->PrintRoutingTable(routingStream, Time::S);
                } else {
                    std::cout << "  Error: Could not get GPSR protocol instance.\n";
                }
            } else {
                 std::cout << "  Error: Could not get Ipv4 instance.\n";
            }

        } else {
            std::cout << "unknown | Position: unknown\n";
        }
    }
}