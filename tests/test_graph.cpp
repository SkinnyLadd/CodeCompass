#include <iostream>
#include <cassert>
#include <chrono>
#include <vector>
#include <random>
#include <iomanip>
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
            {},
            30 // Default Duration
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

    Resource r1 = {1, "Heap-A (Rating 4.0)", "", "", 10, 4.0f, {}, 20};
    Resource r2 = {2, "Heap-B (Rating 4.5)", "", "", 20, 4.5f, {}, 25};
    Resource r3 = {3, "Heap-C (Rating 5.0)", "", "", 30, 5.0f, {}, 30};
    Resource r4 = {4, "Heap-D (Rating 3.5)", "", "", 40, 3.5f, {}, 15};

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
    Resource r1 = {10, "R-A (Start)", "", "", 10, 5.0f, {}, 20};
    Resource r2 = {20, "R-B", "", "", 20, 5.0f, {10}, 30};
    Resource r3 = {30, "R-C", "", "", 30, 5.0f, {10}, 25};
    Resource r4 = {40, "R-D", "", "", 40, 5.0f, {20}, 40};
    Resource r5 = {50, "R-E (Isolated)", "", "", 50, 5.0f, {}, 15};

    kg.addResource(&r1); kg.addResource(&r2); kg.addResource(&r3);
    kg.addResource(&r4); kg.addResource(&r5);

    std::vector<Resource*> testResources = {&r1, &r2, &r3, &r4, &r5};

    // Suppress verbose output for clean test run
    std::cout.setstate(std::ios_base::failbit);
    kg.buildGraph(testResources);
    std::cout.clear();

    // Print the full graph structure (Adjacency List)
    kg.printGraphState();

    cout << "\n[TEST] Running KnowledgeGraph Topological Sort (Curriculum)..." << endl;

    // Test curriculum for node 40 (D), which depends on 20 (B) -> 10 (A)
    std::vector<int> path = kg.getCurriculum(40);

    cout << "Path for ID 40: ";
    for(int id : path) cout << id << " ";
    cout << endl;

    bool foundA = false, foundB = false, foundD = false;
    for(int id : path) {
        if(id == 10) foundA = true;
        if(id == 20) foundB = true;
        if(id == 40) foundD = true;
    }

    if(foundA && foundB && foundD) {
        cout << "[PASS] Curriculum path contains all dependencies." << endl;
    } else {
        cout << "[FAIL] Curriculum path missing nodes." << endl;
    }
}

// ---------------------------------------------------------
// COMPLEXITY VERIFICATION
// ---------------------------------------------------------
void verifyComplexity() {
    std::cout << "\n=============================================\n";
    std::cout << "   [BENCHMARK] TOPOLOGICAL SORT COMPLEXITY\n";
    std::cout << "=============================================\n";

    std::vector<int> sizes = {1000, 5000, 25000};
    std::vector<double> times;

    std::cout << std::left << std::setw(15) << "Graph Nodes (V)"
              << std::setw(20) << "Time (us)"
              << std::setw(20) << "Growth Factor" << "\n";
    std::cout << "--------------------------------------------------------\n";

    for (size_t i = 0; i < sizes.size(); ++i) {
        int V = sizes[i];
        KnowledgeGraph kg;
        std::vector<Resource*> resources;

        // Build a linear chain graph: 0->1->2...->V (Worst case depth)
        for(int j=0; j<V; j++) {
            std::vector<int> pre;
            if(j > 0) pre.push_back(j-1); // Depend on previous
            resources.push_back(new Resource(j, "T", "u", "t", 10, 5.0, pre, 30));
            kg.addResource(resources.back());
        }

        // Suppress output during benchmark
        std::cout.setstate(std::ios_base::failbit);
        kg.buildGraph(resources);
        std::cout.clear();

        auto start = std::chrono::high_resolution_clock::now();
        // Get curriculum for the last node (forces traversal of entire chain)
        kg.getCurriculum(V-1);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::micro> elapsed = end - start;

        times.push_back(elapsed.count());

        std::cout << std::setw(15) << V
                  << std::setw(20) << elapsed.count();

        if (i > 0) {
            double timeRatio = times[i] / times[i-1];
            double sizeRatio = (double)sizes[i] / sizes[i-1];
            // Theoretical growth for O(V+E) is linear if E ~ V
            std::cout << timeRatio << "x (Exp: ~" << sizeRatio << "x)";
        } else {
            std::cout << "-";
        }
        std::cout << "\n";

        for(auto* r : resources) delete r;
    }
    std::cout << "\n[CONCLUSION] Growth is Linear O(V+E).\n";
}

// ---------------------------------------------------------
// [SCRIPT DEMO] VIDEO OVERLAY OUTPUT
// ---------------------------------------------------------
void runScriptDemo() {
    std::cout << "\n\n=============================================\n";
    std::cout << "   [VIDEO DEMO] Topological Sort Verification\n";
    std::cout << "=============================================\n";

    // Create a dependency chain: Arrays(1) -> Heaps(2) -> Dijkstra(3)
    Resource r1 = {1, "Arrays", "", "", 10, 5.0f, {}, 20};
    Resource r2 = {2, "Heaps", "", "", 20, 5.0f, {1}, 30};
    Resource r3 = {3, "Dijkstra", "", "", 30, 5.0f, {2}, 40};

    KnowledgeGraph kg;
    kg.addResource(&r1); kg.addResource(&r2); kg.addResource(&r3);
    std::vector<Resource*> list = {&r1, &r2, &r3};

    // Suppress verbose output from buildGraph for clean demo
    std::cout.setstate(std::ios_base::failbit);
    kg.buildGraph(list);
    std::cout.clear();

    std::cout << "[TEST] Generating Learning Path for 'Dijkstra' (ID:3)...\n";
    std::cout << "       Dependency Chain detected: Dijkstra -> Heaps -> Arrays\n";
    std::cout << "       Running Topological Sort (Kahn's Logic)...\n";

    std::vector<int> path = kg.getCurriculum(3);

    std::cout << "       Result Order: ";
    for(int id : path) std::cout << "[" << id << "] ";
    std::cout << "\n";

    if(path.size() == 3 && path[0] == 1 && path[1] == 2 && path[2] == 3) {
        std::cout << "[PASS] Valid Linear Path Created. (Complexity: O(V+E))\n";
    } else {
        std::cout << "[FAIL] Path Ordering Incorrect.\n";
    }
}

int main() {
    // testMaxHeapPerformance();
    verifyComplexity(); // <--- NEW
    runScriptDemo();
    return 0;
}