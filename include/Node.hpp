#pragma once
#include "ProtocolBase.hpp"  // Now this is safe

struct Node {
    Node* next;
    Pos pos;
    int id;
    Node();
    Node(int id, Pos pos);
};