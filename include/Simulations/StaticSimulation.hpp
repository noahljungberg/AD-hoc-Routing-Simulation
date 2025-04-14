
#ifndef STATICSIMULATION_HPP
#define STATICSIMULATION_HPP

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include <iostream>
#include "AbstractSimulation.hpp"

class StaticSimulation : public AbstractSimulation {
  public:
    StaticSimulation(const int numNodes, const double simulationTime);
    ~StaticSimulation() override;

  protected:
    virtual void SetupTopology() override;
    virtual void SetupRoutingProtocol() override;
    virtual void ConfigureApplications() override;
    virtual void RunSimulation() override;
    virtual void CollectResults() override;

  };


#endif //STATICSIMULATION_HPP
