#include <iostream>
#include <cassert>
#include <chrono>
#include <vector>
#include <memory>
#include <random>
#include <iomanip>
#include <algorithm>
#include "../include/AVLTree.h"
#include "../include/Trie.h"
#include "../include/Resource.h"

using namespace std;

// Helper function to generate random resources for testing
vector<unique_ptr<Resource>> generateRandomResources(int count, int start_id) {
    vector<unique_ptr<Resource>> resources;
    resources.reserve(count);
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> diff_dist(1.0, 10.0);
    
    for (int i = 0; i < count; ++i) {
        resources.push_back(make_unique<Resource>(
            start_id + i,
            "Resource " + to_string(start_id + i),
            "",
            "Type" + to_string((start_id + i) % 5),  // 5 different types
            (start_id + i) % 100,                    // Difficulty 0-99
            static_cast<float>(diff_dist(gen)),       // Random rating 1.0-10.0
            vector<int>{},                           // Empty prerequisites
            45                                       // Default duration
        ));
    }
    return resources;
}

// ======================= AVL Tree Tests =======================

void testAVLTreeBasic() {
    cout << "\n[TEST] Running AVL Tree Basic Operations Test..." << endl;
    
    AVLTree tree;
    
    // Create test resources with unique_ptr for automatic memory management
    auto resources = generateRandomResources(3, 1);
    
    // Test insertion
    tree.insert(resources[0].get());
    tree.insert(resources[1].get());
    tree.insert(resources[2].get());
    
    // Test search
    assert(tree.search(resources[0]->id) == resources[0].get());
    assert(tree.search(resources[1]->id) == resources[1].get());
    assert(tree.search(resources[2]->id) == resources[2].get());
    assert(tree.search(999) == nullptr); // Non-existent ID
    
    // Test traversal orders - get the actual order first
    auto inorder = tree.inorderTraversal();
    assert(inorder.size() == 3);
    
    // Print the actual order for debugging
    cout << "In-order traversal: ";
    for (const auto& r : inorder) {
        cout << r->id << " ";
    }
    cout << endl;
    
    // Verify the order is strictly increasing (BST property)
    assert(std::is_sorted(inorder.begin(), inorder.end(), 
        [](const Resource* a, const Resource* b) { 
            return a->id < b->id; 
        }));
    
    // Test tree statistics
    assert(tree.getNodeCount() == 3);
    assert(tree.getHeight() <= 2); // AVL tree with 3 nodes should have height <= 2
    assert(abs(tree.getRootBalance()) <= 1); // Should be balanced
    
    cout << "[PASS] AVL Tree Basic Operations Test Passed." << endl;
}

void testAVLTreePerformance() {
    cout << "\n[TEST] Running AVL Tree Performance Test..." << endl;
    
    const int NUM_INSERTS = 10000;
    const int NUM_SEARCHES = 10000;
    
    // Generate test data with unique_ptr for automatic memory management
    auto resources = generateRandomResources(NUM_INSERTS, 1000);
    AVLTree tree;
    
    // Insertion test
    auto start_insert = chrono::high_resolution_clock::now();
    for (auto& r : resources) {
        tree.insert(r.get());
    }
    auto end_insert = chrono::high_resolution_clock::now();
    auto insert_time = chrono::duration_cast<chrono::milliseconds>(end_insert - start_insert);
    
    // Search test (mix of found and not found)
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(1, NUM_INSERTS * 2); // 50% hit rate
    
    auto start_search = chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_SEARCHES; ++i) {
        tree.search(distrib(gen));
    }
    auto end_search = chrono::high_resolution_clock::now();
    auto search_time = chrono::duration_cast<chrono::milliseconds>(end_search - start_search);
    
    // Print results
    cout << "[PERFORMANCE] AVL Tree with " << NUM_INSERTS << " elements:" << endl;
    cout << "- Insertion time: " << insert_time.count() << " ms" << endl;
    cout << "- Search time (" << NUM_SEARCHES << " ops): " << search_time.count() << " ms" << endl;
    cout << "- Tree height: " << tree.getHeight() << " (log2(" << NUM_INSERTS << ") ≈ " 
         << static_cast<int>(log2(NUM_INSERTS)) << ")" << endl;
    cout << "- Max balance factor: " << tree.getMaxBalance() << " (should be <= 1 for AVL)" << endl;
}

// ======================= Trie Tests =======================

void testTrieBasic() {
    cout << "\n[TEST] Running Trie Basic Operations Test..." << endl;
    
    Trie trie;
    
    // Insert test data
    trie.insert("algorithm", 1);
    trie.insert("binary search", 2);
    trie.insert("binary tree", 3);
    trie.insert("graph", 4);
    trie.insert("graph traversal", 5);
    
    // Test exact matches
    auto result1 = trie.getResourcesByPrefix("binary search");
    assert(result1.size() == 1);
    assert(result1[0] == 2);
    
    // Test prefix search
    auto result2 = trie.getResourcesByPrefix("bi");
    assert(result2.size() == 2); // Should find "binary search" and "binary tree"
    assert(find(result2.begin(), result2.end(), 2) != result2.end());
    assert(find(result2.begin(), result2.end(), 3) != result2.end());
    
    // Test case insensitivity
    auto result3 = trie.getResourcesByPrefix("GRAPH");
    assert(result3.size() == 2); // Should find "graph" and "graph traversal"
    
    // Test non-existent prefix
    auto result4 = trie.getResourcesByPrefix("xyz");
    assert(result4.empty());
    
    cout << "[PASS] Trie Basic Operations Test Passed." << endl;
}

void testTriePerformance() {
    cout << "\n[TEST] Running Trie Performance Test..." << endl;
    
    const int NUM_WORDS = 10000;
    const int NUM_SEARCHES = 1000;
    const int WORD_LENGTH = 10;
    
    // Generate random words
    vector<string> words;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> char_dist('a', 'z');
    
    for (int i = 0; i < NUM_WORDS; ++i) {
        string word;
        for (int j = 0; j < WORD_LENGTH; ++j) {
            word += static_cast<char>(char_dist(gen));
        }
        words.push_back(word);
    }
    
    // Build trie
    Trie trie;
    auto start_build = chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_WORDS; ++i) {
        trie.insert(words[i], i);
    }
    auto end_build = chrono::high_resolution_clock::now();
    auto build_time = chrono::duration_cast<chrono::milliseconds>(end_build - start_build);
    
    // Test search performance
    uniform_int_distribution<> word_dist(0, NUM_WORDS - 1);
    vector<string> search_terms;
    for (int i = 0; i < NUM_SEARCHES; ++i) {
        // Get a random word and use a prefix of random length (1 to full length)
        string word = words[word_dist(gen)];
        int prefix_len = uniform_int_distribution<>(1, word.length())(gen);
        search_terms.push_back(word.substr(0, prefix_len));
    }
    
    auto start_search = chrono::high_resolution_clock::now();
    int total_matches = 0;
    for (const auto& term : search_terms) {
        auto results = trie.getResourcesByPrefix(term);
        total_matches += results.size();
    }
    auto end_search = chrono::high_resolution_clock::now();
    auto search_time = chrono::duration_cast<chrono::milliseconds>(end_search - start_search);
    
    // Print results
    cout << "[PERFORMANCE] Trie with " << NUM_WORDS << " words:" << endl;
    cout << "- Build time: " << build_time.count() << " ms" << endl;
    cout << "- Search time (" << NUM_SEARCHES << " prefixes): " << search_time.count() << " ms" << endl;
    cout << "- Average matches per prefix: " << (double)total_matches / NUM_SEARCHES << endl;
    cout << "- Total nodes: " << trie.getNodeCount() << " (compression ratio: " 
         << fixed << setprecision(2) << (double)trie.getNodeCount() / (NUM_WORDS * WORD_LENGTH) * 100 << "%)" << endl;
}

// ======================= Main Function =======================

int main() {
    cout << "=============================================" << endl;
    cout << "   TREE DATA STRUCTURES TEST SUITE" << endl;
    cout << "=============================================" << endl;
    
    // Run AVL Tree tests
    testAVLTreeBasic();
    testAVLTreePerformance();
    
    // Run Trie tests
    testTrieBasic();
    testTriePerformance();
    
    cout << "\nAll tests completed successfully!" << endl;
    return 0;
}