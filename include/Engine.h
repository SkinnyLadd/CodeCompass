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
#include "Stack.h"

class Engine {
private:
    std::vector<Resource*> masterList;
    Trie* searchIndex;
    AVLTree* storageTree;
    MaxHeap* ratingQueue;
    KnowledgeGraph* depGraph;
    LRUCache* cache;
    Stack* historyStack;

public:
    Engine(const std::vector<Resource*>& data);
    ~Engine();

    void execute(const std::string& command);
    std::vector<Resource*> resourceList();

private:
    // Handlers
    void handleList(const std::string& sortMode = "");
    void handlePlan(const std::string& targetTitle);
    void handleTitles();
    void handleSearch(const std::string& query);
    void handleTopRated(int k, const std::string& difficultyLevel);
    void handleBack();
    void handleCram(const std::string& args);

    void printResourceLine(Resource* r);

    /**
     * @brief Adds a resource to the navigation history stack if it's not a duplicate
     * @param resource The resource to add to history
     */
    void addToHistory(Resource* resource);
};

#endif