// In ProtocolBase.hpp
#pragma once
#include <vector>

// Forward declarations
struct Pos;
struct Path;
class Node;

struct Pos {
    int x;
    int y;
};

struct Path {
    std::vector<Node*> path;
    int hopCount;
    int timeElapsed;
};

class ProtocolBase {
public:
    ProtocolBase();
    std::vector<Node*> neighborTable;
    Pos pos;
    Path path;
    int hopCount;
    int distanceTo(Node* A, Node* B);
    virtual Node* nextNeighbourOf(Node* A);
    Node* nextNode;
    int timeElapsed;
    virtual Path findRoute(Node* src, Node* dst) = 0;
};