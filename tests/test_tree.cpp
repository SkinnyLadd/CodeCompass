#include <iostream>
#include <cassert>
#include <chrono>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include "../include/AVLTree.h"
#include "../include/Trie.h"
#include "../include/Resource.h"

using namespace std;

// =========================================================
// HELPER FUNCTIONS
// =========================================================

// Generate random resources for benchmarking
std::vector<Resource> generateResources(int count, int start_id) {
    std::vector<Resource> resources;
    resources.reserve(count);
    for (int i = 0; i < count; ++i) {
        resources.emplace_back(
            start_id + i,
            "Resource " + std::to_string(start_id + i),
            "http://test.url",
            "Topic" + std::to_string(i), // Unique topics for Trie test
            rand() % 100,
            4.5,
            std::vector<int>{}
        );
    }
    return resources;
}

// =========================================================
// AVL TREE TESTS (Database)
// =========================================================

void testAVLCorrectness() {
    cout << "\n[TEST] Running AVL Tree Correctness Test..." << endl;

    AVLTree tree;
    
    // Create persistent resources
    Resource r1(10, "Node 10", "", "", 1, 5.0, {});
    Resource r2(20, "Node 20", "", "", 1, 5.0, {});
    Resource r3(5,  "Node 5",  "", "", 1, 5.0, {}); 
    Resource r4(30, "Node 30", "", "", 1, 5.0, {}); 

    // Insert (Scrambled order to trigger balancing)
    tree.insert(&r1);
    tree.insert(&r2);
    tree.insert(&r3);
    tree.insert(&r4);

    // 1. Test Search (Hit)
    Resource* res = tree.search(20);
    assert(res != nullptr && res->title == "Node 20");
    cout << "  [PASS] Search existing ID (20) successful." << endl;

    // 2. Test Search (Miss)
    Resource* missing = tree.search(999);
    assert(missing == nullptr);
    cout << "  [PASS] Search non-existing ID (999) returned nullptr." << endl;

    // 3. Test Sorted Retrieval (In-order traversal)
    vector<Resource*> sorted = tree.getAllSorted();
    assert(sorted.size() == 4);
    assert(sorted[0]->id == 5);
    assert(sorted[1]->id == 10);
    assert(sorted[2]->id == 20);
    assert(sorted[3]->id == 30);
    cout << "  [PASS] In-order traversal returned IDs in sorted order." << endl;
}

void testAVLPerformance() {
    cout << "\n[TEST] Running AVL Tree Performance Test..." << endl;

    const int NUM_ITEMS = 50000;
    vector<Resource> data = generateResources(NUM_ITEMS, 1000);
    
    // Shuffle to force AVL rotations (inserting sorted data into a BST is worst-case, AVL handles it)
    auto rng = std::default_random_engine {};
    std::shuffle(std::begin(data), std::end(data), rng);

    AVLTree tree;

    // --- Insertion Timing ---
    auto start_insert = chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_ITEMS; ++i) {
        tree.insert(&data[i]);
    }
    auto end_insert = chrono::high_resolution_clock::now();
    chrono::duration<double> diff_insert = end_insert - start_insert;
    cout << "  >> Inserted " << NUM_ITEMS << " items in " << diff_insert.count() << " s" << endl;

    // --- Search Timing ---
    auto start_search = chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_ITEMS; ++i) {
        volatile Resource* r = tree.search(data[i].id); 
    }
    auto end_search = chrono::high_resolution_clock::now();
    chrono::duration<double> diff_search = end_search - start_search;
    cout << "  >> Searched " << NUM_ITEMS << " items in " << diff_search.count() << " s" << endl;
}

// =========================================================
// TRIE TESTS (Search Engine)
// =========================================================

void testTrieCorrectness() {
    cout << "\n[TEST] Running Trie Search Correctness Test..." << endl;

    Trie trie;
    
    // Create test resources with specific topics
    Resource r1(1, "Array Basics", "", "Arrays", 1, 1.0, {});
    Resource r2(2, "Adv Arrays",   "", "Arrays", 1, 1.0, {});
    Resource r3(3, "BST Intro",    "", "Binary Search Tree", 1, 1.0, {});
    Resource r4(4, "BFS Guide",    "", "Breadth First Search", 1, 1.0, {});
    Resource r5(5, "Backtracking", "", "Backtracking", 1, 1.0, {});

    trie.insert(&r1);
    trie.insert(&r2);
    trie.insert(&r3);
    trie.insert(&r4);
    trie.insert(&r5);

    // 1. Exact Prefix Match
    vector<Resource*> res1 = trie.searchByPrefix("Back");
    assert(res1.size() == 1);
    assert(res1[0]->title == "Backtracking");
    cout << "  [PASS] Prefix 'Back' found 'Backtracking'." << endl;

    // 2. Case Insensitivity
    vector<Resource*> res2 = trie.searchByPrefix("aRrAyS");
    assert(res2.size() == 2);
    cout << "  [PASS] Prefix 'aRrAyS' (mixed case) found 2 resources." << endl;

    // 3. Broad Prefix (Autocomplete)
    vector<Resource*> res3 = trie.searchByPrefix("B");
    // Should find: Binary..., Breadth..., Backtracking
    assert(res3.size() == 3); 
    cout << "  [PASS] Broad Prefix 'B' found 3 distinct topics." << endl;

    // 4. No Match
    vector<Resource*> res4 = trie.searchByPrefix("X");
    assert(res4.empty());
    cout << "  [PASS] Non-existent prefix returned empty list." << endl;
}

void testTriePerformance() {
    cout << "\n[TEST] Running Trie Performance Test..." << endl;

    const int NUM_ITEMS = 50000;
    // Reuse generation logic but focus on topics
    vector<Resource> resources = generateResources(NUM_ITEMS, 200000);

    Trie trie;

    // --- Insertion Timing ---
    auto start_insert = chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_ITEMS; ++i) {
        trie.insert(&resources[i]);
    }
    auto end_insert = chrono::high_resolution_clock::now();
    chrono::duration<double> diff_insert = end_insert - start_insert;
    cout << "  >> Indexed " << NUM_ITEMS << " topics in " << diff_insert.count() << " s" << endl;

    // --- Prefix Search Timing ---
    auto start_search = chrono::high_resolution_clock::now();
    // "Topic1" should match Topic1, Topic10, Topic100, etc.
    vector<Resource*> results = trie.searchByPrefix("Topic1");
    auto end_search = chrono::high_resolution_clock::now();
    
    chrono::duration<double> diff_search = end_search - start_search;
    cout << "  >> Autocomplete lookup for 'Topic1' found " << results.size() 
         << " matches in " << diff_search.count() << " s" << endl;
}

// =========================================================
// MAIN EXECUTION
// =========================================================

int main() {
    std::cout << "============================================" << std::endl;
    std::cout << "       RUNNING TREE MODULE TESTS            " << std::endl;
    std::cout << "============================================" << std::endl;

    testAVLCorrectness();
    testAVLPerformance();
    
    testTrieCorrectness();
    testTriePerformance();

    std::cout << "\n[SUCCESS] All Tree tests passed!" << std::endl;
    return 0;
}