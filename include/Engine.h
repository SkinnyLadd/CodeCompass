#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <string>
#include <iostream>
#include "Resource.h"

// Include your Data Structures
#include "AVLTree.h"
#include "Trie.h"
#include "MaxHeap.h"
#include "KnowledgeGraph.h"
#include "LRUCache.h"

class Engine {
private:
    // Master list of raw data
    std::vector<Resource*> masterList;

    // --- Data Structures ---
    Trie* searchIndex;           // For Prefix Search
    AVLTree* storageTree;        // For ID Lookup (O(log n))
    MaxHeap* ratingQueue;        // For "Top Rated"
    KnowledgeGraph* depGraph;    // For "Curriculum Planning"
    LRUCache* cache;             // For caching query results

public:
    // Constructor: Loads data and builds all structures
    Engine(const std::vector<Resource*>& data);

    // Destructor: Cleans up memory
    ~Engine();

    // Main Entry Point
    void execute(const std::string& jsonToken);

private:
    // --- Command Handlers ---
    void handleSearch(const std::string& json);
    void handlePlan(const std::string& json);   // <--- NEW: Generates Learning Path
    void handleCount();
    void handleGraph();
    void handleList();

    // --- Helpers ---
    std::string extractValue(const std::string& json, const std::string& key);
    bool containsIgnoreCase(const std::string& source, const std::string& query);
};

#endif