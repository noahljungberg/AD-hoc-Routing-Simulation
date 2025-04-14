#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"


int main(int argc, char *argv[])
{
    NodeContainer nodes;
    nodes.Create(10);  // Create 10 nodes

    // Set up WiFi: This creates a WiFi helper and sets it to use the 802.11b standard.
    WifiHelper wifi;
    wifi.SetStandard(WIFI_PHY_STANDARD_80211b);

    // configure the physical layer (PHY) for wifi
    YansWifiPhyHelpeer wifiPhy;
    // set the type of data link used in the packet capture (for tracing packets)
    wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
    // create a default wifi channel ( simulates the radio env)
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
    wifiPhy.SetChannel(wifiChannel.Create());

    // configure mac layer for wifi in an ad hoc mode
    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");


    //install wifi devices on the nodes with the above PHy and MAC settings
    NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, nodes);

    // Set up mobility: This arranges the nodes in a static grid layout.
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
    "MinX", DoubleValue(0.0),           // Starting X coordinate
    "MinY", DoubleValue(0.0),           // Starting Y coordinate
    "DeltaX", DoubleValue(100.0),       // Distance between nodes in X-direction
    "DeltaY", DoubleValue(100.0),       // Distance between nodes in Y-direction
    "GridWidth", UintegerValue(5),      // Number of nodes per row
    "LayoutType", StringValue("RowFirst")); // Fill rows before columns

    // set a mobility model that keeps nodes in constant position
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    // install internet stack: this adds  a networking protocols to the nodes (like IP)
    InternetStackHelper internet;
    DsdvHelper dsdv;
    internet.SetRoutingHelper(dsdv); // Tell the Internet stack to use DSDV for routing
    internet.Install(nodes);

    // Assign IP addresses to the nodes
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0"); // define the base network and subnet mask
    Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

    // create an application: set up a udp echo client on node 9 that sends a packet to the server
    UdpEchoClientHelper echoClient(interfaces.GetAddress(0), 9); // Target server at node 0, port 9
    echoClient.SetAttribute("MaxPackets", UintegerValue(1)); // send only one packet
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0))); // 1 second interval
    echoClient.SetAttribute("PacketSize", UintegerValue(1024)); // packet size of 1024 bytes

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(9)); // install the client on node 9
    clientApps.Start(Seconds(2.0)); // start the client at 2 seconds into the simulation
    clientApps.Stop(Seconds(10.0)); // stop the client at 10 seconds into the simulation

    // enable tracing: this writes packet-level information to a pcap file
    wifiPhy.EnablePcap("dsdv-simulation", devices); // enable pcap tracing for the devices
    // Run simulation: Start the simulation and keep it running until all scheduled events are processed.
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
