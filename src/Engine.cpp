#include "../include/Engine.h"
#include "../include/Sorters.h"
#include "../include/Optimizer.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>

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
    std::cout <<"ID,Title,URL,Topic,Difficulty,Rating,Duration" << std::endl;
    std::vector<Resource*> temp = storageTree->inorderTraversal();;
    if (sortMode == "DIFFICULTY") Sorters::sortByDifficulty(temp);
    else if (sortMode == "TOPIC") Sorters::sortByTopic(temp);
    for (Resource* r : temp) printResourceLine(r);
}

void Engine::handleTitles() {
    for (const auto* r : masterList) std::cout << r->title << std::endl;
}

void Engine::handleSearch(const std::string& query) {
    std::vector<int> results = searchIndex->getResourcesByPrefix(query);
    if (results.empty()) return;

    std::cout << "ID,Title,URL,Topic,Difficulty,Rating,Duration" << std::endl;
    for (int id : results) {
        Resource* r = cache->get(id, true);
        if (!r) {
            r = storageTree->search(id);
            if (r) cache->put(r, true);
        }
        if (r) {
            printResourceLine(r);
            historyStack->push(r, false);
        }
    }
}

void Engine::handlePlan(const std::string& targetTitle) {
    int targetID = -1;
    for (Resource* r : masterList) {
        if (r->title == targetTitle) { targetID = r->id; break; }
    }
    if (targetID == -1) return;

    std::vector<int> path = depGraph->getCurriculum(targetID);
    if (path.empty()) return;

    std::cout << "ID,Title,URL,Topic,Difficulty,Rating,Duration" << std::endl;
    for (int id : path) {
        Resource* r = storageTree->search(id);
        if (r) printResourceLine(r);
    }
}

void Engine::handleBack() {
    Resource* r = historyStack->pop(true);
    if (r) {
        std::cout << "ID,Title,URL,Topic,Difficulty,Rating,Duration" << std::endl;
        printResourceLine(r);
    }
}

void Engine::handleCram(const std::string& args) {
    std::vector<std::string> parts = splitInternal(args, '|');
    if (parts.size() < 2) return;

    std::string topic = parts[0];
    int maxTime = 0;
    try { maxTime = std::stoi(parts[1]); } catch (...) { return; }

    std::vector<int> candidateIDs = searchIndex->getResourcesByPrefix(topic);
    std::vector<Resource*> candidates;
    for (int id : candidateIDs) {
        Resource* r = cache->get(id, false);
        if (!r) {
            r = storageTree->search(id);
            if (r) cache->put(r, false);
        }
        if (r) candidates.push_back(r);
    }

    std::vector<Resource*> bestPlan = Optimizer::maximizeRating(candidates, maxTime);
    std::cout << "ID,Title,URL,Topic,Difficulty,Rating,Duration" << std::endl;
    for (Resource* r : bestPlan) printResourceLine(r);
}

// --- NEW: Smart Recommendations using MaxHeap ---
void Engine::handleTopRated(int k, const std::string& difficultyLevel) {
    // 1. Create a filtered subset based on Difficulty
    MaxHeap tempHeap;

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
        }
    }

    // 3. Extract Top K (O(k log N))
    std::cout << "ID,Title,URL,Topic,Difficulty,Rating,Duration" << std::endl;
    for(int i=0; i<k; i++) {
        if(tempHeap.isEmpty()) break;
        Resource* r = tempHeap.extractMax(false);
        if(r) printResourceLine(r);
    }
}