#include "AbstractSimulation.hpp"



void AbstractSimulation::SetupNetwork() {
    WifiHelper wifi;
    wifi.SetStandard(WIFI_PHY_STANDARD_80211b);

    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
    wifiPhy.SetChannel(wifiChannel.Create());

    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");  // Ensuring ad hoc mode

    // 'nodes' should be a member defined in AbstractSimulation, or set up by derived classes.
    NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, nodes);
}

void AbstractSimulation::SetupDSDV() {
    DsdvHelper dsdv;
    InternetStackHelper internet;
    internet.SetRoutingHelper(dsdv);
    internet.Install(m_nodes);

    // Configure IP addressing
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    m_interfaces = ipv4.Assign(m_devices);  // Store in a member variable
}

void AbstractSimulation::SetupDSR()
{
    // pass
}

 void AbstractSimulation::SetupGPSR()
{
    // pass
}