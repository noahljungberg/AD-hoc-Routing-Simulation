/*
GPSR utilizes a greedy algorithm to determine the route from source to destination. Each node in the route determines which node (unvisited) is the shortest distance away from it and hops to it. 
*/

class GPSR: public ProtocolBase {
    public:
        Path findRoute(Node* src, Node* dst) override;
        Node* nextNeighbourOf(Node* A) override;
};