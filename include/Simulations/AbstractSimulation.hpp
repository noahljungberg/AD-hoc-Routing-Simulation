/**
* This class is meant to define an abstrac simulation class where will simulate our
* different routing protocols given different network topologies
*/

#ifndef SIMULATION_H
#define SIMULATION_H

#include <string>
#include <iostream>
#include <string>
#include "ns3/core-module.h"
#include "ns3/wifi-module.h"
#include "ns3/propagation-module.h"
#include "ns3/mobility-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/dsr-module.h"
#include "../gpsr/gpsr-helper.hpp"





class AbstractSimulation {
  public:
    virtual ~AbstractSimulation() {}
    void Run(){
      SetupNetwork();
      SetupTopology();
        SetupRoutingProtocol();
        ConfigureApplications();
        RunSimulation();
        CollectResults();
     }
protected:
  // Methods that must be implemented by derived classes
  void SetupNetwork();
 void SetupDSDV();         // configure DSDV routing protocol
 void SetupDSR();          // configure DSR routing protocol
 void SetupGPSR();         // configure GPSR routing protocol
  virtual void SetupTopology() = 0;         // e.g., node creation, mobility model
  virtual void SetupRoutingProtocol() = 0;      // configure DSDV, DSR, or GPSR on nodes
  virtual void ConfigureApplications() = 0;   // install applications, set up traffic flows
  virtual void RunSimulation() = 0;           // run the simulation (e.g., call Simulator::Run())
  virtual void CollectResults() = 0;          // gather and process simulation results


  ns3::NodeContainer m_nodes;
  int m_numNodes = 10;
  double m_simulationTime = 100.0;
  std::string m_routingProtocol;
  std::string m_topology;
  std::string m_applicationType;
  std::string m_resultsFile;

  // New member variables
  ns3::NetDeviceContainer m_devices;
  ns3::Ipv4InterfaceContainer m_interfaces;

  Ptr<FlowMonitor> m_flowMonitor;
  FlowMonitorHelper m_flowHelper;
};



#endif //SIMULATION_H
