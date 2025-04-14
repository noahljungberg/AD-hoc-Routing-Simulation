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
