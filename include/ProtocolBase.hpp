#pragma once
#include <vector>
#include "Node.hpp"
using namespace std;

struct Pos
{
    int x;
    int y;
};

class ProtocolBase {
    public:
        ProtocolBase();
        //virtual ~ProtocolBase();
        vector<Node*> neighborTable;

        struct Pos pos;
        struct Path path;

        int hopCount;
        int distanceTo(Node* A, Node* B);

        virtual Node* nextNeighbourOf(Node* A);
        
        Node* nextNode;
    
        int timeElapsed;
        virtual Path findRoute(Node* src, Node* dst) = 0;
        
        protected:

    };

struct Neighbor {
    int id;
    struct Pos position;
    double lastSeen;
};

struct Path {
    vector<Node*> path;
    int hopCount;
    int timeElapsed;
};
