#include <iostream>
#include <cassert>
#include <chrono> // Added for performance timing
#include <vector>
#include <random>
#include "../include/LRUCache.h"
#include "../include/Stack.h"
#include "../include/Resource.h"

using namespace std;

// Function to generate unique random resources for bulk testing (redefined for this file)
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
            (float)distrib(gen),
            {}
        };
        resources.push_back(r);
    }
    return resources;
}

// --- Stack Performance Test ---
void testStackPerformance() {
    cout << "\n[TEST] Running Stack Performance Test..." << endl;
    const int NUM_OPERATIONS = 100000;
    std::vector<Resource> resources = generateRandomResources(NUM_OPERATIONS, 20000);
    Stack s;

    // --- Push Timing (O(1)) ---
    auto start_push = chrono::high_resolution_clock::now();
    for (Resource& r : resources) {
        // Suppress print output for performance test
        s.push(&r, false);
    }
    auto end_push = chrono::high_resolution_clock::now();
    auto duration_push = chrono::duration_cast<chrono::microseconds>(end_push - start_push);

    // --- Pop Timing (O(1)) ---
    auto start_pop = chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_OPERATIONS; ++i) {
        // Suppress print output for performance test
        s.pop(false);
    }
    auto end_pop = chrono::high_resolution_clock::now();
    auto duration_pop = chrono::duration_cast<chrono::microseconds>(end_pop - start_pop);

    cout << "[PERFORMANCE RESULT] Stack Push (" << NUM_OPERATIONS << " elements): "
         << duration_push.count() << " us" << endl;
    cout << "[PERFORMANCE RESULT] Stack Pop (" << NUM_OPERATIONS << " elements): "
         << duration_pop.count() << " us" << endl;
}

// --- LRU Cache Performance Test ---
void testLRUCachePerformance() {
    cout << "\n[TEST] Running LRUCache Performance Test..." << endl;
    const int CACHE_CAPACITY = 1000;
    const int NUM_OPERATIONS = 100000;
    std::vector<Resource> resources = generateRandomResources(CACHE_CAPACITY, 30000); // Resources to initially fill cache

    LRUCache cache(CACHE_CAPACITY);

    // 1. Fill the cache
    for (Resource& r : resources) {
        // Suppress print output for performance test
        cache.put(&r, false);
    }

    // 2. Perform Random Access (mostly Cache Hits, checking O(1) behavior)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(30000, 30000 + CACHE_CAPACITY - 1); // Access IDs in the cache range

    // --- Get Timing (O(1)) ---
    auto start_get = chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_OPERATIONS; ++i) {
        // Suppress print output for performance test
        cache.get(distrib(gen), false);
    }
    auto end_get = chrono::high_resolution_clock::now();
    auto duration_get = chrono::duration_cast<chrono::microseconds>(end_get - start_get);

    cout << "[PERFORMANCE RESULT] LRUCache Get (" << NUM_OPERATIONS << " operations): "
         << duration_get.count() << " us" << endl;

    // --- Put Timing (O(1) with potential eviction) ---
    // Note: We use a mix of existing IDs (hits) and new IDs (misses/evictions)
    auto start_put = chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_OPERATIONS; ++i) {
        // Use a small set of resources to ensure high cache hit rate and check complexity
        // Suppress print output for performance test
        cache.put(&resources[i % CACHE_CAPACITY], false);
    }
    auto end_put = chrono::high_resolution_clock::now();
    auto duration_put = chrono::duration_cast<chrono::microseconds>(end_put - start_put);

    cout << "[PERFORMANCE RESULT] LRUCache Put (" << NUM_OPERATIONS << " operations): "
         << duration_put.count() << " us" << endl;
}


// --- Stack Test (Uses visualization from Stack.h) ---
void testStack() {
    cout << "\n[TEST] Running Stack Visualization Test (Correctness Check)..." << endl;

    Resource r1 = {1, "R-Stack-A", "", "TopicA", 10, 4.0f, {}};
    Resource r2 = {2, "R-Stack-B", "", "TopicB", 20, 4.5f, {}};
    Resource r3 = {3, "R-Stack-C", "", "TopicC", 30, 5.0f, {}};

    Stack history;

    // Default calls (no 'false' flag) use doPrint=true for visualization
    history.push(&r1);
    history.push(&r2);
    history.push(&r3);

    history.pop();
    history.pop();

    Resource* peeked = history.peek();
    assert(peeked != nullptr && peeked->id == 1);

    // Clean up remaining element, suppressing print
    history.pop(false);

    cout << "[PASS] Stack Tests Passed." << endl;
}

// --- LRU Cache Test (Uses visualization from LRUCache.h) ---
void testLRUCacheVisualization() {
    cout << "\n[TEST] Running LRU Cache Visualization Test (Correctness Check)..." << endl;

    Resource r1 = {10, "Cache-A", "", "", 10, 4.0f, {}};
    Resource r2 = {20, "Cache-B", "", "", 20, 4.5f, {}};
    Resource r3 = {30, "Cache-C", "", "", 30, 5.0f, {}};
    Resource r4 = {40, "Cache-D", "", "", 40, 5.0f, {}};

    LRUCache cache(3);

    // Default calls (no 'false' flag) use doPrint=true for visualization
    cache.put(&r1);
    cache.put(&r2);
    cache.put(&r3);

    cache.get(10);

    cout << "\n--- CAUSING EVICTION (Resource B [ID 20] should be evicted) ---" << endl;
    cache.put(&r4);

    // Suppress prints during assertion checks
    assert(cache.get(20, false) == nullptr);
    assert(cache.size() == 3);

    assert(cache.get(30, false) != nullptr);
    assert(cache.get(40, false) != nullptr);

    // Clean up remaining elements for completeness, suppressing print
    // The specific cleanup method for LRUCache depends on implementation.
    // Since LRU is hard to empty sequentially, we leave the test cleanup minimal.

    cout << "[PASS] LRU Cache Visualization Tests Passed." << endl;
}


int main() {
    // 1. Linear Data Structure Tests (Stack)
    testStack();
    testStackPerformance(); // New performance test

    // 2. Cache Data Structure Tests (LRUCache)
    testLRUCacheVisualization();
    testLRUCachePerformance(); // New performance test
    
    return 0;
}