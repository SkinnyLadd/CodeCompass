// DUMMY TEST CODE

#include "KnowledgeGraph.h"
#include "MaxHeap.h"
#include <iostream>

int main() {
    KnowledgeGraph graph;

    // 1. Setup Graph (0 -> 1 -> 2)
    graph.addEdge(0, 1, 10); // weight 10
    graph.addEdge(1, 2, 5);  // weight 5

    // 2. Test Dijkstra
    std::cout << "Path from 0 to 2:\n";
    graph.dijkstra(0, 2);
    // Expected Output: Distance 15

    return 0;
}
