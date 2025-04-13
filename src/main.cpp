#include <iostream>
#include "GPSR.hpp"
#include "Node.hpp"
#include "ProtocolBase.hpp"

using namespace std;

int main() {
    // Create nodes
    Node nodeA(1, {0, 0});
    Node nodeB(2, {5, 0});
    Node nodeC(3, {10, 0});
    Node nodeD(4, {15, 0});    

    // Simulate neighbor relationships
    std::vector<Node*> neighbors = {&nodeB, &nodeC};  // Node A can see B and C

    // Create GPSR instance
    GPSR gpsr;
    
    // Manually assign the neighbor table
    gpsr.neighborTable = neighbors;

    // Call findRoute from A to D
    Path result = gpsr.findRoute(&nodeA, &nodeD);

    // Print results
    std::cout << "Route found:\n";
    for (Node* node : result.path) {
        std::cout << "Node ID: " << node->id << " ("
                  << node->pos.x << ", " << node->pos.y << ")\n";
    }
    std::cout << "Total hops: " << result.hopCount << "\n";
    std::cout << "Time elapsed: " << result.timeElapsed << "\n";

    return 0;
}
