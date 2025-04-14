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
  virtual void SetupTopology() = 0;         // e.g., node creation, mobility model
  virtual void SetupRoutingProtocol() = 0;      // configure DSDV, DSR, or GPSR on nodes
  virtual void ConfigureApplications() = 0;   // install applications, set up traffic flows
  virtual void RunSimulation() = 0;           // run the simulation (e.g., call Simulator::Run())
  virtual void CollectResults() = 0;          // gather and process simulation results


  ns3::NodeContainer m_nodes;
  int m_numNodes;
  double m_simulationTime;
  std::string m_routingProtocol;
  std::string m_topology;
  std::string m_applicationType;
  std::string m_resultsFile;
};



#endif //SIMULATION_H
