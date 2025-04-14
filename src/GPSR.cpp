
#include "ProtocolBase.hpp"
#include <vector>
#include "Node.hpp"
#include "GPSR.hpp"
#include <cmath> // For sqrt and pow

    Path findRoute(Node* src, Node* dst) {
        Node* curr = src;
        Node* next;

        while (curr != dst) {
            path.path.push_back(curr);
            next = nextNeighbourOf(curr);
            path.hopCount++;
            curr = next;
        }

        path.path.push_back(dst);
        return path;
    }

    Node* nextNeighbourOf(Node* A) {
        int currentShortest;
        int tempDistance;
        Node* closestNode;

        for (auto i = neighborTable.begin(); i < neighborTable.end(); i++) {
            tempDistance = distanceTo(A, *i);
            if (tempDistance < currentShortest) {
                currentShortest = tempDistance;
                closestNode = *i;
            }
        }

        path.timeElapsed += tempDistance;
        return closestNode;
    }
