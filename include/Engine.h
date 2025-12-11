#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <string>
#include <iostream>
#include "Resource.h"
#include "Trie.h"
#include "AVLTree.h"
#include "MaxHeap.h"
#include "KnowledgeGraph.h"
#include "LRUCache.h"

class Engine {
private:
    std::vector<Resource*> masterList;
    Trie* searchIndex;
    AVLTree* storageTree;
    MaxHeap* ratingQueue;
    KnowledgeGraph* depGraph;
    LRUCache* cache;

public:
    Engine(const std::vector<Resource*>& data);
    ~Engine();

    // Main Router
    void execute(const std::string& command);
    std::vector<Resource*> resourceList();
private:
    // Handlers
    void handleList();
    void handlePlan(const std::string& targetTitle);
    void handleTitles(); // Prints list of titles for UI Selection
    void handleDetails(const std::string& title); // Prints details for one resource

    // Helper to print attributes in CSV format
    void printResourceLine(Resource* r);
};

#endif