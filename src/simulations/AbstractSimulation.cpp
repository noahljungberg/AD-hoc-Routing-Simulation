#include "Simulations/AbstractSimulation.hpp"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"

// To make code cleaner
using namespace ns3;

void AbstractSimulation::SetupNetwork() {
    m_nodes.Create(m_numNodes);

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211b);

    // Create and configure channel with propagation models
    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");

    // Create and configure PHY
    YansWifiPhyHelper wifiPhy;
    wifiPhy.SetChannel(wifiChannel.Create());
    // Optional: Configure other PHY parameters
    wifiPhy.SetErrorRateModel("ns3::YansErrorRateModel");

    // Create and configure MAC
    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");

    // Install on nodes
    m_devices = wifi.Install(wifiPhy, wifiMac, m_nodes);
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
    // Implementation will come later
}

void AbstractSimulation::SetupGPSR() {
    // Implementation will come later
}