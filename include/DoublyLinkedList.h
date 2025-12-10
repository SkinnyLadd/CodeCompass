
#ifndef DOUBLYLINKEDLIST_H
#define DOUBLYLINKEDLIST_H

#include "Resource.h"

// Node for the Doubly Linked List.
// This is the fundamental element used by the LRUCache to maintain recency order.
struct Node {
    Resource* res;
    Node* prev;
    Node* next;

    // Constructor: resource pointer can be null for dummy head/tail nodes.
    Node(Resource* r) : res(r), prev(nullptr), next(nullptr) {}
};

#endif
