#include "../include/Engine.h"
#include "../include/Sorters.h"
#include "../include/Optimizer.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>

// Helper to split string
std::vector<std::string> splitInternal(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

Engine::Engine(const std::vector<Resource*>& data) : masterList(data) {
    searchIndex = new Trie();
    storageTree = new AVLTree();
    ratingQueue = new MaxHeap();
    depGraph = new KnowledgeGraph();
    cache = new LRUCache(5);
    historyStack = new Stack();

    for (Resource* r : data) {
        searchIndex->insert(r->title, r->id);
        searchIndex->insert(r->topic, r->id);
        storageTree->insert(r);
        ratingQueue->insert(r, false);
        depGraph->addResource(r);
    }
    depGraph->buildGraph(data);
}

Engine::~Engine() {
    delete searchIndex; delete storageTree;
    delete ratingQueue; delete depGraph; delete cache; delete historyStack;
}

std::vector<Resource*> Engine::resourceList() { return masterList; }

void Engine::printResourceLine(Resource* r) {
    std::cout << r->id << "," << r->title << "," << r->url << ","
              << r->topic << "," << r->difficulty << "," << r->rating << ","
              << r->duration << std::endl;
}

void Engine::execute(const std::string& command) {
    std::string action = "";
    std::string value = "";

    size_t delimiterPos = command.find('|');
    if (delimiterPos != std::string::npos) {
        action = command.substr(0, delimiterPos);
        value = command.substr(delimiterPos + 1);
    } else {
        action = command;
    }

    if (action == "LIST") handleList(value);
    else if (action == "PLAN") handlePlan(value);
    else if (action == "TITLES") handleTitles();
    else if (action == "SEARCH") handleSearch(value);
    else if (action == "SUGGEST") {
        // Parse "Count|Level" from value (e.g. "5|BEGINNER")
        int k = 5;
        std::string level = "ALL";

        std::vector<std::string> args = splitInternal(value, '|');
        if (!args.empty()) {
            try { k = std::stoi(args[0]); } catch(...) { k = 5; }
            if (args.size() > 1) level = args[1];
        }
        handleTopRated(k, level);
    }
    else if (action == "BACK") handleBack();
    else if (action == "CRAM") handleCram(value);
    else {
        std::cerr << "ERROR: Unknown command: " << action << std::endl;
    }
}

// ... [Existing implementations for LIST, TITLES, PLAN, SEARCH, BACK] ...
// (Assume handleList, handleSearch, handlePlan, handleBack, handleCram are here as previously defined)

void Engine::handleList(const std::string& sortMode) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Capture tree state before operation
    int treeHeightBefore = storageTree->getHeight();
    int nodeCountBefore = storageTree->getNodeCount();
    int maxBalanceBefore = storageTree->getMaxBalance();
    
    std::cout <<"ID,Title,URL,Topic,Difficulty,Rating,Duration" << std::endl;
    std::vector<Resource*> temp = storageTree->inorderTraversal();
    
    auto traversalEnd = std::chrono::high_resolution_clock::now();
    
    if (sortMode == "DIFFICULTY") Sorters::sortByDifficulty(temp);
    else if (sortMode == "TOPIC") Sorters::sortByTopic(temp);
    
    auto sortEnd = std::chrono::high_resolution_clock::now();
    
    for (Resource* r : temp) printResourceLine(r);
    
    auto end = std::chrono::high_resolution_clock::now();
    
    // Capture tree state after operation
    int treeHeightAfter = storageTree->getHeight();
    int nodeCountAfter = storageTree->getNodeCount();
    int maxBalanceAfter = storageTree->getMaxBalance();
    
    // Calculate timings
    auto traversalTime = std::chrono::duration_cast<std::chrono::microseconds>(traversalEnd - start).count();
    auto sortTime = std::chrono::duration_cast<std::chrono::microseconds>(sortEnd - traversalEnd).count();
    auto totalTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    // Get tree structure for visualization
    auto treeStruct = storageTree->getTreeStructure();
    
    // Output analysis data (for UI parsing)
    std::cout << "---ANALYSIS---" << std::endl;
    std::cout << "OPERATION:LIST|" << sortMode << std::endl;
    std::cout << "TREE_HEIGHT:" << treeHeightAfter << std::endl;
    std::cout << "NODE_COUNT:" << nodeCountAfter << std::endl;
    std::cout << "MAX_BALANCE:" << maxBalanceAfter << std::endl;
    std::cout << "ROOT_BALANCE:" << storageTree->getRootBalance() << std::endl;
    std::cout << "TRAVERSAL_TIME_US:" << traversalTime << std::endl;
    std::cout << "SORT_TIME_US:" << sortTime << std::endl;
    std::cout << "TOTAL_TIME_US:" << totalTime << std::endl;
    std::cout << "SORT_ALGORITHM:" << (sortMode == "DIFFICULTY" ? "QUICKSORT" : (sortMode == "TOPIC" ? "MERGESORT" : "NONE")) << std::endl;
    std::cout << "RESULT_COUNT:" << temp.size() << std::endl;
    
    // Output tree structure
    std::cout << "TREE_EDGES_START" << std::endl;
    for (const auto& edge : treeStruct.first) {
        std::cout << edge << std::endl;
    }
    std::cout << "TREE_EDGES_END" << std::endl;
    std::cout << "TREE_NODES_START" << std::endl;
    for (const auto& node : treeStruct.second) {
        std::cout << node << std::endl;
    }
    std::cout << "TREE_NODES_END" << std::endl;
    
    std::cout << "---END_ANALYSIS---" << std::endl;
}

void Engine::handleTitles() {
    for (const auto* r : masterList) std::cout << r->title << std::endl;
}

void Engine::handleSearch(const std::string& query) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Capture tree state before operation
    int treeHeightBefore = storageTree->getHeight();
    int nodeCountBefore = storageTree->getNodeCount();
    
    std::vector<int> results = searchIndex->getResourcesByPrefix(query);
    
    auto trieEnd = std::chrono::high_resolution_clock::now();
    
    if (results.empty()) {
        auto end = std::chrono::high_resolution_clock::now();
        auto totalTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        
        std::cout << "---ANALYSIS---" << std::endl;
        std::cout << "OPERATION:SEARCH|" << query << std::endl;
        std::cout << "TREE_HEIGHT:" << treeHeightBefore << std::endl;
        std::cout << "NODE_COUNT:" << nodeCountBefore << std::endl;
        std::cout << "TRIE_SEARCH_TIME_US:" << std::chrono::duration_cast<std::chrono::microseconds>(trieEnd - start).count() << std::endl;
        std::cout << "TOTAL_TIME_US:" << totalTime << std::endl;
        std::cout << "RESULT_COUNT:0" << std::endl;
        std::cout << "---END_ANALYSIS---" << std::endl;
        return;
    }

    std::cout << "ID,Title,URL,Topic,Difficulty,Rating,Duration" << std::endl;
    int treeSearchCount = 0;
    for (int id : results) {
        Resource* r = cache->get(id, true);
        if (!r) {
            r = storageTree->search(id);
            treeSearchCount++;
            if (r) cache->put(r, true);
        }
        if (r) {
            printResourceLine(r);
            historyStack->push(r, false);
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    
    // Capture tree state after operation
    int treeHeightAfter = storageTree->getHeight();
    int nodeCountAfter = storageTree->getNodeCount();
    
    // Calculate timings
    auto trieTime = std::chrono::duration_cast<std::chrono::microseconds>(trieEnd - start).count();
    auto totalTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    // Get tree structure for visualization
    auto treeStruct = storageTree->getTreeStructure();
    
    // Get structures for visualization
    auto trieStruct = searchIndex->getStructure();
    auto cacheStruct = cache->getStructure();
    auto stackStruct = historyStack->getStructure();
    
    // Output analysis data
    std::cout << "---ANALYSIS---" << std::endl;
    std::cout << "OPERATION:SEARCH|" << query << std::endl;
    std::cout << "TREE_HEIGHT:" << treeHeightAfter << std::endl;
    std::cout << "NODE_COUNT:" << nodeCountAfter << std::endl;
    std::cout << "MAX_BALANCE:" << storageTree->getMaxBalance() << std::endl;
    std::cout << "ROOT_BALANCE:" << storageTree->getRootBalance() << std::endl;
    std::cout << "TRIE_SEARCH_TIME_US:" << trieTime << std::endl;
    std::cout << "TREE_SEARCHES:" << treeSearchCount << std::endl;
    std::cout << "TOTAL_TIME_US:" << totalTime << std::endl;
    std::cout << "RESULT_COUNT:" << results.size() << std::endl;
    std::cout << "TRIE_NODE_COUNT:" << searchIndex->getNodeCount() << std::endl;
    std::cout << "CACHE_SIZE:" << cache->size() << std::endl;
    std::cout << "STACK_SIZE:" << (historyStack->isEmpty() ? 0 : stackStruct.size() - 1) << std::endl;
    
    // Output AVL Tree structure
    std::cout << "TREE_EDGES_START" << std::endl;
    for (const auto& edge : treeStruct.first) {
        std::cout << edge << std::endl;
    }
    std::cout << "TREE_EDGES_END" << std::endl;
    std::cout << "TREE_NODES_START" << std::endl;
    for (const auto& node : treeStruct.second) {
        std::cout << node << std::endl;
    }
    std::cout << "TREE_NODES_END" << std::endl;
    
    // Output Trie structure
    std::cout << "TRIE_EDGES_START" << std::endl;
    for (const auto& edge : trieStruct.first) {
        std::cout << edge << std::endl;
    }
    std::cout << "TRIE_EDGES_END" << std::endl;
    std::cout << "TRIE_NODES_START" << std::endl;
    for (const auto& node : trieStruct.second) {
        std::cout << node << std::endl;
    }
    std::cout << "TRIE_NODES_END" << std::endl;
    
    // Output LRU Cache structure
    std::cout << "CACHE_STRUCTURE_START" << std::endl;
    for (const auto& item : cacheStruct) {
        std::cout << item << std::endl;
    }
    std::cout << "CACHE_STRUCTURE_END" << std::endl;
    
    // Output Stack structure
    std::cout << "STACK_STRUCTURE_START" << std::endl;
    for (const auto& item : stackStruct) {
        std::cout << item << std::endl;
    }
    std::cout << "STACK_STRUCTURE_END" << std::endl;
    
    std::cout << "---END_ANALYSIS---" << std::endl;
}

void Engine::handlePlan(const std::string& targetTitle) {
    auto start = std::chrono::high_resolution_clock::now();
    
    int targetID = -1;
    for (Resource* r : masterList) {
        if (r->title == targetTitle) { targetID = r->id; break; }
    }
    if (targetID == -1) {
        auto end = std::chrono::high_resolution_clock::now();
        auto totalTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        
        std::cout << "---ANALYSIS---" << std::endl;
        std::cout << "OPERATION:PLAN|" << targetTitle << std::endl;
        std::cout << "TOTAL_TIME_US:" << totalTime << std::endl;
        std::cout << "RESULT_COUNT:0" << std::endl;
        std::cout << "ERROR:Target not found" << std::endl;
        std::cout << "---END_ANALYSIS---" << std::endl;
        return;
    }

    std::vector<int> path = depGraph->getCurriculum(targetID);
    
    auto graphEnd = std::chrono::high_resolution_clock::now();
    
    if (path.empty()) {
        auto end = std::chrono::high_resolution_clock::now();
        auto totalTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        
        std::cout << "---ANALYSIS---" << std::endl;
        std::cout << "OPERATION:PLAN|" << targetTitle << std::endl;
        std::cout << "GRAPH_SEARCH_TIME_US:" << std::chrono::duration_cast<std::chrono::microseconds>(graphEnd - start).count() << std::endl;
        std::cout << "TOTAL_TIME_US:" << totalTime << std::endl;
        std::cout << "RESULT_COUNT:0" << std::endl;
        std::cout << "---END_ANALYSIS---" << std::endl;
        return;
    }

    std::cout << "ID,Title,URL,Topic,Difficulty,Rating,Duration" << std::endl;
    int treeSearchCount = 0;
    for (int id : path) {
        Resource* r = storageTree->search(id);
        treeSearchCount++;
        if (r) printResourceLine(r);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    
    auto graphTime = std::chrono::duration_cast<std::chrono::microseconds>(graphEnd - start).count();
    auto totalTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    // Get structures for visualization
    auto treeStruct = storageTree->getTreeStructure();
    auto graphStruct = depGraph->getStructure();
    
    std::cout << "---ANALYSIS---" << std::endl;
    std::cout << "OPERATION:PLAN|" << targetTitle << std::endl;
    std::cout << "TREE_HEIGHT:" << storageTree->getHeight() << std::endl;
    std::cout << "NODE_COUNT:" << storageTree->getNodeCount() << std::endl;
    std::cout << "GRAPH_SEARCH_TIME_US:" << graphTime << std::endl;
    std::cout << "TREE_SEARCHES:" << treeSearchCount << std::endl;
    std::cout << "TOTAL_TIME_US:" << totalTime << std::endl;
    std::cout << "RESULT_COUNT:" << path.size() << std::endl;
    std::cout << "GRAPH_NODE_COUNT:" << depGraph->getNodeCount() << std::endl;
    std::cout << "GRAPH_EDGE_COUNT:" << depGraph->getEdgeCount() << std::endl;
    
    // Output AVL Tree structure
    std::cout << "TREE_EDGES_START" << std::endl;
    for (const auto& edge : treeStruct.first) {
        std::cout << edge << std::endl;
    }
    std::cout << "TREE_EDGES_END" << std::endl;
    std::cout << "TREE_NODES_START" << std::endl;
    for (const auto& node : treeStruct.second) {
        std::cout << node << std::endl;
    }
    std::cout << "TREE_NODES_END" << std::endl;
    
    // Output Knowledge Graph structure
    std::cout << "GRAPH_EDGES_START" << std::endl;
    for (const auto& edge : graphStruct.first) {
        std::cout << edge << std::endl;
    }
    std::cout << "GRAPH_EDGES_END" << std::endl;
    std::cout << "GRAPH_NODES_START" << std::endl;
    for (const auto& node : graphStruct.second) {
        std::cout << node << std::endl;
    }
    std::cout << "GRAPH_NODES_END" << std::endl;
    
    std::cout << "---END_ANALYSIS---" << std::endl;
}

void Engine::handleBack() {
    if (!historyStack->isEmpty()) {
        Resource* r = historyStack->pop(true);  // true means don't delete the resource
        if (r) {
            std::cout << "ID,Title,URL,Topic,Difficulty,Rating,Duration" << std::endl;
            printResourceLine(r);
        }
    }
}

void Engine::addToHistory(Resource* resource) {
    if (resource) {
        // Don't add the same resource twice in a row
        if (historyStack->isEmpty() || 
            historyStack->peek()->id != resource->id) {
            historyStack->push(resource);
        }
    }
}

void Engine::handleCram(const std::string& args) {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::string> parts = splitInternal(args, '|');
    if (parts.size() < 2) {
        auto end = std::chrono::high_resolution_clock::now();
        auto totalTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        
        std::cout << "---ANALYSIS---" << std::endl;
        std::cout << "OPERATION:CRAM|" << args << std::endl;
        std::cout << "TOTAL_TIME_US:" << totalTime << std::endl;
        std::cout << "ERROR:Invalid arguments" << std::endl;
        std::cout << "---END_ANALYSIS---" << std::endl;
        return;
    }

    std::string topic = parts[0];
    int maxTime = 0;
    try { maxTime = std::stoi(parts[1]); } catch (...) {
        auto end = std::chrono::high_resolution_clock::now();
        auto totalTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        
        std::cout << "---ANALYSIS---" << std::endl;
        std::cout << "OPERATION:CRAM|" << args << std::endl;
        std::cout << "TOTAL_TIME_US:" << totalTime << std::endl;
        std::cout << "ERROR:Invalid time format" << std::endl;
        std::cout << "---END_ANALYSIS---" << std::endl;
        return;
    }

    // Capture tree state before operation
    int treeHeightBefore = storageTree->getHeight();
    int nodeCountBefore = storageTree->getNodeCount();
    
    std::vector<int> candidateIDs = searchIndex->getResourcesByPrefix(topic);
    
    auto trieEnd = std::chrono::high_resolution_clock::now();
    
    std::vector<Resource*> candidates;
    int treeSearchCount = 0;
    for (int id : candidateIDs) {
        Resource* r = cache->get(id, false);
        if (!r) {
            r = storageTree->search(id);
            treeSearchCount++;
            if (r) cache->put(r, false);
        }
        if (r) candidates.push_back(r);
    }
    
    auto searchEnd = std::chrono::high_resolution_clock::now();
    
    std::vector<Resource*> bestPlan = Optimizer::maximizeRating(candidates, maxTime);
    
    auto optimizeEnd = std::chrono::high_resolution_clock::now();
    
    std::cout << "ID,Title,URL,Topic,Difficulty,Rating,Duration" << std::endl;
    for (Resource* r : bestPlan) printResourceLine(r);
    
    auto end = std::chrono::high_resolution_clock::now();
    
    // Calculate timings
    auto trieTime = std::chrono::duration_cast<std::chrono::microseconds>(trieEnd - start).count();
    auto searchTime = std::chrono::duration_cast<std::chrono::microseconds>(searchEnd - trieEnd).count();
    auto optimizeTime = std::chrono::duration_cast<std::chrono::microseconds>(optimizeEnd - searchEnd).count();
    auto totalTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    // Get structures for visualization
    auto treeStruct = storageTree->getTreeStructure();
    auto trieStruct = searchIndex->getStructure();
    auto cacheStruct = cache->getStructure();
    
    // Output analysis data
    std::cout << "---ANALYSIS---" << std::endl;
    std::cout << "OPERATION:CRAM|" << topic << "|" << maxTime << std::endl;
    std::cout << "TREE_HEIGHT:" << treeHeightBefore << std::endl;
    std::cout << "NODE_COUNT:" << nodeCountBefore << std::endl;
    std::cout << "TRIE_SEARCH_TIME_US:" << trieTime << std::endl;
    std::cout << "TREE_SEARCH_TIME_US:" << searchTime << std::endl;
    std::cout << "OPTIMIZER_TIME_US:" << optimizeTime << std::endl;
    std::cout << "TREE_SEARCHES:" << treeSearchCount << std::endl;
    std::cout << "CANDIDATES:" << candidates.size() << std::endl;
    std::cout << "TOTAL_TIME_US:" << totalTime << std::endl;
    std::cout << "DATA_STRUCTURE:KNAPSACK_DP" << std::endl;
    std::cout << "RESULT_COUNT:" << bestPlan.size() << std::endl;
    std::cout << "TRIE_NODE_COUNT:" << searchIndex->getNodeCount() << std::endl;
    std::cout << "CACHE_SIZE:" << cache->size() << std::endl;
    
    // Output AVL Tree structure
    std::cout << "TREE_EDGES_START" << std::endl;
    for (const auto& edge : treeStruct.first) {
        std::cout << edge << std::endl;
    }
    std::cout << "TREE_EDGES_END" << std::endl;
    std::cout << "TREE_NODES_START" << std::endl;
    for (const auto& node : treeStruct.second) {
        std::cout << node << std::endl;
    }
    std::cout << "TREE_NODES_END" << std::endl;
    
    // Output Trie structure
    std::cout << "TRIE_EDGES_START" << std::endl;
    for (const auto& edge : trieStruct.first) {
        std::cout << edge << std::endl;
    }
    std::cout << "TRIE_EDGES_END" << std::endl;
    std::cout << "TRIE_NODES_START" << std::endl;
    for (const auto& node : trieStruct.second) {
        std::cout << node << std::endl;
    }
    std::cout << "TRIE_NODES_END" << std::endl;
    
    // Output LRU Cache structure
    std::cout << "CACHE_STRUCTURE_START" << std::endl;
    for (const auto& item : cacheStruct) {
        std::cout << item << std::endl;
    }
    std::cout << "CACHE_STRUCTURE_END" << std::endl;
    
    std::cout << "---END_ANALYSIS---" << std::endl;
}

// --- NEW: Smart Recommendations using MaxHeap ---
void Engine::handleTopRated(int k, const std::string& difficultyLevel) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Capture tree state before operation
    int treeHeightBefore = storageTree->getHeight();
    int nodeCountBefore = storageTree->getNodeCount();
    
    // 1. Create a filtered subset based on Difficulty
    MaxHeap tempHeap;
    
    auto filterStart = std::chrono::high_resolution_clock::now();
    int filteredCount = 0;

    for(Resource* r : masterList) {
        bool include = false;

        if (difficultyLevel == "ALL") {
            include = true;
        } else if (difficultyLevel == "BEGINNER") {
            if (r->difficulty <= 35) include = true;
        } else if (difficultyLevel == "INTERMEDIATE") {
            if (r->difficulty > 35 && r->difficulty <= 70) include = true;
        } else if (difficultyLevel == "ADVANCED") {
            if (r->difficulty > 70) include = true;
        }

        // 2. Insert filtered items into Heap (O(log N))
        if (include) {
            tempHeap.insert(r, false);
            filteredCount++;
        }
    }
    
    auto heapBuildEnd = std::chrono::high_resolution_clock::now();

    // 3. Extract Top K (O(k log N))
    std::cout << "ID,Title,URL,Topic,Difficulty,Rating,Duration" << std::endl;
    int extractedCount = 0;
    for(int i=0; i<k; i++) {
        if(tempHeap.isEmpty()) break;
        Resource* r = tempHeap.extractMax(false);
        if(r) {
            printResourceLine(r);
            extractedCount++;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    
    // Calculate timings
    auto filterTime = std::chrono::duration_cast<std::chrono::microseconds>(heapBuildEnd - filterStart).count();
    auto totalTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    // Get structures for visualization
    auto treeStruct = storageTree->getTreeStructure();
    auto heapStruct = tempHeap.getStructure();
    
    // Output analysis data
    std::cout << "---ANALYSIS---" << std::endl;
    std::cout << "OPERATION:SUGGEST|" << k << "|" << difficultyLevel << std::endl;
    std::cout << "TREE_HEIGHT:" << treeHeightBefore << std::endl;
    std::cout << "NODE_COUNT:" << nodeCountBefore << std::endl;
    std::cout << "FILTER_TIME_US:" << filterTime << std::endl;
    std::cout << "HEAP_OPERATIONS:" << filteredCount << std::endl;
    std::cout << "EXTRACTED_COUNT:" << extractedCount << std::endl;
    std::cout << "TOTAL_TIME_US:" << totalTime << std::endl;
    std::cout << "DATA_STRUCTURE:MAXHEAP" << std::endl;
    std::cout << "RESULT_COUNT:" << extractedCount << std::endl;
    std::cout << "HEAP_SIZE:" << tempHeap.getSize() << std::endl;
    
    // Output AVL Tree structure
    std::cout << "TREE_EDGES_START" << std::endl;
    for (const auto& edge : treeStruct.first) {
        std::cout << edge << std::endl;
    }
    std::cout << "TREE_EDGES_END" << std::endl;
    std::cout << "TREE_NODES_START" << std::endl;
    for (const auto& node : treeStruct.second) {
        std::cout << node << std::endl;
    }
    std::cout << "TREE_NODES_END" << std::endl;
    
    // Output MaxHeap structure
    std::cout << "HEAP_STRUCTURE_START" << std::endl;
    for (const auto& item : heapStruct) {
        std::cout << item << std::endl;
    }
    std::cout << "HEAP_STRUCTURE_END" << std::endl;
    
    std::cout << "---END_ANALYSIS---" << std::endl;
}