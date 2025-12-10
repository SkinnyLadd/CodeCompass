#include <iostream>
#include <cassert>
#include <chrono> // Added for performance timing
#include <vector>
#include <random>
#include "../include/KnowledgeGraph.h"
#include "../include/MaxHeap.h"
#include "../include/Resource.h"

using namespace std;

// Function to generate unique random resources for bulk testing
std::vector<Resource> generateRandomResources(int count, int start_id) {
    std::vector<Resource> resources;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distrib(1.0, 5.0);

    for (int i = 0; i < count; ++i) {
        Resource r = {
            start_id + i,
            "Random Resource " + std::to_string(start_id + i),
            "",
            "Type",
            rand() % 1000,
            (float)distrib(gen), // Random rating between 1.0 and 5.0
            {}
        };
        resources.push_back(r);
    }
    return resources;
}

void testMaxHeapPerformance() {
    cout << "\n[TEST] Running MaxHeap Performance Test..." << endl;

    const int NUM_RESOURCES = 50000;
    std::vector<Resource> resources = generateRandomResources(NUM_RESOURCES, 10000);
    MaxHeap heap;

    // --- Insertion Timing (O(N log N)) ---
    auto start_insert = chrono::high_resolution_clock::now();
    for (Resource& r : resources) {
        // PASSING 'false' TO DISABLE PRINTING during bulk test
        heap.insert(&r, false);
    }
    auto end_insert = chrono::high_resolution_clock::now();
    auto duration_insert = chrono::duration_cast<chrono::microseconds>(end_insert - start_insert);

    // --- Extraction Timing (O(K log N)) ---
    const int NUM_EXTRACT = NUM_RESOURCES / 5; // Extract 10,000 elements
    auto start_extract = chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_EXTRACT; ++i) {
        // PASSING 'false' TO DISABLE PRINTING
        if (heap.extractMax(false) == nullptr) break;
    }
    auto end_extract = chrono::high_resolution_clock::now();
    auto duration_extract = chrono::duration_cast<chrono::microseconds>(end_extract - start_extract);

    cout << "[PERFORMANCE RESULT] MaxHeap Insertion (" << NUM_RESOURCES << " elements): "
         << duration_insert.count() << " us" << endl;
    cout << "[PERFORMANCE RESULT] MaxHeap Extraction (" << NUM_EXTRACT << " elements): "
         << duration_extract.count() << " us" << endl;

    // Cleanup for next test
    // PASSING 'false' TO DISABLE PRINTING during cleanup
    int cleanup_count = 0;
    const int MAX_CLEANUP_ITERATIONS = NUM_RESOURCES * 2;
    while (heap.extractMax(false) != nullptr) {
        cleanup_count++;
        if (cleanup_count > MAX_CLEANUP_ITERATIONS) {
            cerr << "[ERROR] MaxHeap Cleanup failed to terminate. Potential infinite loop in extractMax()." << endl;
            break;
        }
    }
}


void testMaxHeapVisualization() {
    cout << "\n[TEST] Running MaxHeap Visualization Test (Correctness Check)..." << endl;

    Resource r1 = {1, "Heap-A (Rating 4.0)", "", "", 10, 4.0f, {}};
    Resource r2 = {2, "Heap-B (Rating 4.5)", "", "", 20, 4.5f, {}};
    Resource r3 = {3, "Heap-C (Rating 5.0)", "", "", 30, 5.0f, {}};
    Resource r4 = {4, "Heap-D (Rating 3.5)", "", "", 40, 3.5f, {}};

    MaxHeap heap;

    // Default call uses doPrint=true
    heap.insert(&r3);
    heap.insert(&r2);
    heap.insert(&r1);
    heap.insert(&r4);

    Resource* max1 = heap.extractMax();
    assert(max1 != nullptr && max1->id == 3);

    Resource* max2 = heap.extractMax();
    assert(max2 != nullptr && max2->id == 2);

    // Clean up remaining elements - passing 'false' to avoid clutter
    while (heap.extractMax(false) != nullptr);

    cout << "[PASS] MaxHeap Visualization Tests Passed." << endl;
}

void testGraphAdjacencyListVisualization() {
    cout << "\n[TEST] Running KnowledgeGraph Adjacency List & BFS Visualization Test..." << endl;
    KnowledgeGraph kg;

    // Setup the small branching graph (V=5, E=3)
    Resource r1 = {10, "R-A (Start)", "", "", 10, 5.0f, {}};
    Resource r2 = {20, "R-B", "", "", 20, 5.0f, {10}};
    Resource r3 = {30, "R-C", "", "", 30, 5.0f, {10}};
    Resource r4 = {40, "R-D", "", "", 40, 5.0f, {20}};
    Resource r5 = {50, "R-E (Isolated)", "", "", 50, 5.0f, {}};

    kg.addResource(&r1); kg.addResource(&r2); kg.addResource(&r3);
    kg.addResource(&r4); kg.addResource(&r5);

    // Dependencies: A -> B, A -> C, B -> D
    kg.addDependency(10, 20);
    kg.addDependency(10, 30);
    kg.addDependency(20, 40);

    // Print the full graph structure (Adjacency List)
    kg.printGraphState();

    cout << "\n[TEST] Running KnowledgeGraph BFS Traversal..." << endl;

    // --- BFS Timing (O(V + E)) ---
    auto start_bfs = chrono::high_resolution_clock::now();
    // Assuming KnowledgeGraph::bfs(10) uses a visited set and prints minimally
    kg.bfs(10); // Run BFS, which prints the layered traversal
    auto end_bfs = chrono::high_resolution_clock::now();
    auto duration_bfs = chrono::duration_cast<chrono::microseconds>(end_bfs - start_bfs);

    cout << "[PERFORMANCE RESULT] BFS Traversal (Starting from ID 10): "
         << duration_bfs.count() << " us" << endl;
    cout << "[PASS] Graph Visualization Tests completed." << endl;
}


int main() {
    testMaxHeapVisualization();
    testMaxHeapPerformance();
    testGraphAdjacencyListVisualization(); 
    
    return 0;
}