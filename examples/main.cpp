// File: src/adHocRoutingComparison.cc

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/olsr-helper.h" // Example routing protocol

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("AdHocRoutingComparison");

int main (int argc, char *argv[])
{
  // Command-line arguments
  uint32_t numNodes = 10;
  double simulationTime = 50; // seconds
  
  CommandLine cmd;
  cmd.AddValue("numNodes", "Number of nodes", numNodes);
  cmd.AddValue("simulationTime", "Simulation duration in seconds", simulationTime);
  cmd.Parse(argc, argv);
  
  // Create nodes
  NodeContainer nodes;
  nodes.Create(numNodes);
  
  // WiFi Setup in ad-hoc mode
  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211b);
  
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
  wifiPhy.SetChannel(wifiChannel.Create());
  
  WifiMacHelper wifiMac;
  wifiMac.SetType("ns3::AdhocWifiMac");
  
  NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, nodes);
  
  // Internet stack and routing (using OLSR as an example)
  InternetStackHelper internet;
  OlsrHelper olsr;
  internet.SetRoutingHelper(olsr);
  internet.Install(nodes);
  
  // Mobility model setup
  MobilityHelper mobility;
  mobility.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
                                "X", DoubleValue(500.0),
                                "Y", DoubleValue(500.0));
  mobility.SetMobilityModel("ns3::RandomWaypointMobilityModel",
                            "Speed", StringValue("ns3::UniformRandomVariable[Min=1.0|Max=5.0]"),
                            "Pause", StringValue("ns3::ConstantRandomVariable[Constant=0.5]"));
  mobility.Install(nodes);
  
  // Application setup: UDP Echo server/client
  uint16_t port = 9;
  UdpEchoServerHelper echoServer(port);
  ApplicationContainer serverApps = echoServer.Install(nodes.Get(numNodes - 1));
  serverApps.Start(Seconds(1.0));
  serverApps.Stop(Seconds(simulationTime));
  
  UdpEchoClientHelper echoClient(nodes.Get(numNodes - 1)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), port);
  echoClient.SetAttribute("MaxPackets", UintegerValue(100));
  echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
  echoClient.SetAttribute("PacketSize", UintegerValue(1024));
  
  ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
  clientApps.Start(Seconds(2.0));
  clientApps.Stop(Seconds(simulationTime));
  
  NS_LOG_INFO("Starting Simulation...");
  Simulator::Stop(Seconds(simulationTime));
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Simulation complete.");
  return 0;
}
