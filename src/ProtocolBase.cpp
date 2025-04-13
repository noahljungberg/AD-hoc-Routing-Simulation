#include <cmath>  // Include this for std::sqrt
#include "Node.hpp"

class ProtocolBase {
    int distanceTo(Node* A, Node* B) {
        double dx = A->pos.x - B->pos.x;
        double dy = A->pos.y - B->pos.y;
        return std::sqrt(dx * dx + dy * dy);
    }   
};
