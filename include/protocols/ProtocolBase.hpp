#pragma once
#include <vector>
#include "Node.hpp"
#include "Packet.hpp"
using namespace std;

class ProtocolBase {
public:
    virtual void initialize() = 0;
    virtual void handlePacket(Packet* packet) = 0;
    virtual void sendPacket(Packet* packet, Node* destination) = 0;
    virtual void periodicUpdate() = 0; // For protocols that need periodic updates

    // Metrics collection
    virtual int getRouteOverhead() = 0;
    virtual int getAverageHopCount() = 0;
    virtual double getRouteDiscoveryLatency() = 0;

protected:
    // Common functionality
    std::vector<Node*> neighbors;
    Node* self;
};
