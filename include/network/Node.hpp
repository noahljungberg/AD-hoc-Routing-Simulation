#include <string>
#include "ProtocolBase.hpp"  // gives access to 'pos'

struct Node {
    Node* next;      
    struct Pos pos;
    int id;
    Node();
    Node(int id, Pos pos);
};
