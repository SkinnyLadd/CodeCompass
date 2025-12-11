#include "Engine.h"
#include <algorithm>
#include <cctype>
#include <iostream>

Engine::Engine(const std::vector<Resource*>& data) : masterList(data) {
    // Init Data Structures
    searchIndex = new Trie();
    storageTree = new AVLTree();
    ratingQueue = new MaxHeap();
    depGraph = new KnowledgeGraph();
    cache = new LRUCache(5);

    // Populate
    for (Resource* r : data) {
        searchIndex->insert(r->title, r->id);
        searchIndex->insert(r->topic, r->id);
        storageTree->insert(r);
        ratingQueue->insert(r);
        depGraph->addResource(r);
    }
    depGraph->buildGraph(data);
}

Engine::~Engine() {
    delete searchIndex; delete storageTree;
    delete ratingQueue; delete depGraph; delete cache;
}

void Engine::execute(const std::string& json) {
    if (json.find("\"action\": \"plan\"") != std::string::npos) {
        handlePlan(json);
    }
    else if (json.find("\"action\": \"search\"") != std::string::npos) {
        handleSearch(json);
    }
    else if (json.find("\"action\": \"list\"") != std::string::npos) {
        handleList(); // <-- Calls Selection Mode Logic
    }
    else if (json.find("\"action\": \"count\"") != std::string::npos) {
        handleCount();
    }
    else {
        std::cout << "Unknown command." << std::endl;
    }
}

// --- MODE 1: SELECTION (Dumps Data) ---
void Engine::handleList() {
    // Header for Python to parse
    std::cout << "ID,Title,Topic,Difficulty,Rating" << std::endl;
    for (const auto* r : masterList) {
        std::cout << r->id << "," << r->title << "," << r->topic << ","
                  << r->difficulty << "," << r->rating << std::endl;
    }
}

// --- MODE 2: PLAN (Builds Path) ---
void Engine::handlePlan(const std::string& json) {
    std::string val = extractValue(json, "value");
    if (val.empty()) {
        std::cout << "Error: No ID provided." << std::endl;
        return;
    }

    int targetID = std::stoi(val);
    std::vector<int> path = depGraph->getCurriculum(targetID);

    if (path.empty()) {
        std::cout << "Error: Could not generate plan (Check ID or Circular Dependency)." << std::endl;
        return;
    }

    std::cout << "--- Generated Plan ---" << std::endl;
    int step = 1;
    for (int id : path) {
        Resource* r = storageTree->search(id);
        if(r) {
            std::cout << "Step " << step++ << ": [" << r->id << "] "
                      << r->title << " (" << r->topic << ")" << std::endl;
        }
    }
}

void Engine::handleSearch(const std::string& json) {
    std::string term = extractValue(json, "value");
    std::cout << "Searching: " << term << std::endl;
    for (const auto* r : masterList) {
        if (containsIgnoreCase(r->title, term) || containsIgnoreCase(r->topic, term)) {
            std::cout << "[" << r->id << "] " << r->title << std::endl;
        }
    }
}

void Engine::handleCount() {
    std::cout << "Count: " << masterList.size() << std::endl;
}

std::string Engine::extractValue(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\":";
    size_t start = json.find(search);
    if (start == std::string::npos) return "";
    start += search.length();
    while (start < json.length() && (json[start] == ' ' || json[start] == '"')) start++;
    size_t end = start;
    while (end < json.length() && json[end] != '"' && json[end] != ',' && json[end] != '}') end++;
    return json.substr(start, end - start);
}

bool Engine::containsIgnoreCase(const std::string& source, const std::string& query) {
    auto it = std::search(
        source.begin(), source.end(),
        query.begin(), query.end(),
        [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
    );
    return (it != source.end());
}