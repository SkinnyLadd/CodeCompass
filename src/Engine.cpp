#include "Engine.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>

Engine::Engine(const std::vector<Resource*>& data) : masterList(data) {
    searchIndex = new Trie();
    storageTree = new AVLTree();
    ratingQueue = new MaxHeap();
    depGraph = new KnowledgeGraph();
    cache = new LRUCache(5);

    for (Resource* r : data) {
        searchIndex->insert(r->title, r->id);
        searchIndex->insert(r->topic, r->id);
        storageTree->insert(r);
        ratingQueue->insert(r);
        depGraph->addResource(r);
    }
   // depGraph->buildGraph(data);
}

Engine::~Engine() {
    delete searchIndex; delete storageTree;
    delete ratingQueue; delete depGraph; delete cache;
}

std::vector<Resource*> Engine::resourceList() {
    return masterList;
}
// --- HELPER: Prints valid CSV line ---
void Engine::printResourceLine(Resource* r) {
    std::cout << r->id << ","
              << r->title << ","
              << r->url << ","
              << r->topic << ","
              << r->difficulty << ","
              << r->rating << std::endl;
}

// --- COMMAND PARSER (No JSON) ---
void Engine::execute(const std::string& command) {
    // Format: "ACTION|VALUE"
    std::string action = "";
    std::string value = "";

    size_t delimiterPos = command.find('|');
    if (delimiterPos != std::string::npos) {
        action = command.substr(0, delimiterPos);
        value = command.substr(delimiterPos + 1);
    } else {
        action = command;
    }

    // Routing
    if (action == "LIST") handleList();
    else if (action == "PLAN") handlePlan(value);
    else if (action == "TITLES") handleTitles();
    else std::cout << "ERROR: Unknown command" << std::endl;
}

// --- LIST: Dump CSV ---
void Engine::handleList() {
    // Print Header
    std::cout << "ID,Title,URL,Topic,Difficulty,Rating" << std::endl;
    for (Resource* r : masterList) {
        printResourceLine(r);
    }
}
void Engine::handleTitles() {
    std::vector<Resource*> resources = resourceList();
    for (const auto* r : resources) {
        std::cout << r->title << std::endl;
    }
}
void Engine::handleDetails(const std::string& title) {
    for (Resource* r : masterList) {
        if (r->title == title) {
            printResourceLine(r);
            return;
        }
    }
    std::cout << "NOT_FOUND" << std::endl;
}
// --- PLAN: Title -> ID -> Path -> CSV ---
void Engine::handlePlan(const std::string& targetTitle) {
    // 1. Find ID from Title
    int targetID = -1;
    for (Resource* r : masterList) {
        if (r->title == targetTitle) {
            targetID = r->id;
            break;
        }
    }

    if (targetID == -1) {
        // Output empty or specific error, strictly nothing for CSV parser to choke on
        return;
    }

    // 2. Get Path from Graph
    std::vector<int> path = depGraph->getCurriculum(targetID);

    if (path.empty()) return;

    // 3. Print Header & Rows
    std::cout << "ID,Title,URL,Topic,Difficulty,Rating" << std::endl;
    for (int id : path) {
        Resource* r = storageTree->search(id);
        if (r) {
            printResourceLine(r);
        }
    }
}
