#include "Simulations/AbstractSimulation.hpp"


// To make code cleaner
using namespace ns3;

void AbstractSimulation::SetupNetwork() {
    m_nodes.Create(m_numNodes);

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211b);

    // Create a simpler channel with extended range
    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");

    // Use RangePropagationLossModel for simplicity with extended range
    wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel",
                                  "MaxRange", DoubleValue(250.0)); // Increase range to 250m

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
    wifiPhy.EnableAsciiAll(ascii.CreateFileStream("wireless-trace.tr"));
    wifiPhy.EnablePcapAll("simulation-pcap");
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
}