#include "Engine.h"
#include <algorithm>
#include <cctype>
#include <sstream>

// =========================================================
// CONSTRUCTOR: BUILD THE SYSTEM
// =========================================================
Engine::Engine(const std::vector<Resource*>& data) : masterList(data) {
    std::cout << "[Engine] Initializing Data Structures..." << std::endl;

    // 1. Allocate Memory for Structures
    searchIndex = new Trie();
    storageTree = new AVLTree();
    ratingQueue = new MaxHeap();
    depGraph = new KnowledgeGraph();
    cache = new LRUCache(5); // Cache last 5 results

    // 2. Populate Structures (Single Pass through Data)
    for (Resource* r : data) {
        // A. Trie: Index Title and Topic for fast text search
        searchIndex->insert(r->title, r->id);
        searchIndex->insert(r->topic, r->id);

        // B. AVL Tree: Index by ID for fast retrieval
        storageTree->insert(r);

        // C. Max Heap: Index by Rating
        ratingQueue->insert(r);

        // D. Knowledge Graph: Add the node
        depGraph->addResource(r);
    }

    // 3. Build Graph Edges (Connect Dependencies)
    // This must happen AFTER all nodes are added
    depGraph->buildGraph(data);

    std::cout << "[Engine] System Ready. Loaded " << data.size() << " resources." << std::endl;
}

// =========================================================
// DESTRUCTOR
// =========================================================
Engine::~Engine() {
    delete searchIndex;
    delete storageTree;
    delete ratingQueue;
    delete depGraph;
    delete cache;
    // Note: We generally don't delete Resource* here if main.cpp owns them,
    // but if Engine owns them, uncomment the loop below:
    // for(auto r : masterList) delete r;
}

// =========================================================
// EXECUTION ROUTER
// =========================================================
void Engine::execute(const std::string& json) {
    // 1. Check Cache (Optional Optimization)
    // if (cache->contains(json)) { ... return cache->get(json); }

    // 2. Route Command
    if (json.find("\"action\": \"search\"") != std::string::npos) {
        handleSearch(json);
    }
    else if (json.find("\"action\": \"filter\"") != std::string::npos) {
        handleFilter(json);
    }
    else if (json.find("\"action\": \"plan\"") != std::string::npos) {
        handlePlan(json); // <--- Calls the new Graph Logic
    }
    else if (json.find("\"action\": \"count\"") != std::string::npos) {
        handleCount();
    }
    else {
        std::cerr << "Engine Error: Unknown command action." << std::endl;
    }
}

// =========================================================
// HANDLERS
// =========================================================

void Engine::handleCount() {
    std::cout << "Total Resources: " << masterList.size() << std::endl;
}

void Engine::handleSearch(const std::string& json) {
    std::string term = extractValue(json, "value");
    std::cout << "\n--- Search Results for: '" << term << "' ---" << std::endl;

    // OPTION A: Use Trie for Prefix Search (Super Fast)
    // std::vector<int> ids = searchIndex->getResourcesByPrefix(term);
    // for(int id : ids) { Resource* r = storageTree->search(id); if(r) r->print(); }

    // OPTION B: Robust Substring Search (Better for general queries like "easy python")
    // We will use Option B for now as it's more flexible for natural language queries.
    bool found = false;
    for (const auto* r : masterList) {
        if (containsIgnoreCase(r->title, term) || containsIgnoreCase(r->topic, term)) {
            r->print();
            found = true;
        }
    }
    if (!found) std::cout << "No matches found." << std::endl;
}

void Engine::handleFilter(const std::string& json) {
    std::string key = extractValue(json, "key");
    std::string minStr = extractValue(json, "min");
    std::string maxStr = extractValue(json, "max");

    double minVal = minStr.empty() ? 0 : std::stod(minStr);
    double maxVal = maxStr.empty() ? 9999 : std::stod(maxStr);

    std::cout << "\n--- Filter: " << key << " [" << minVal << " - " << maxVal << "] ---" << std::endl;

    int count = 0;
    for (const auto* r : masterList) {
        bool match = false;
        if (key == "difficulty") {
            if (r->difficulty >= minVal && r->difficulty <= maxVal) match = true;
        } else if (key == "rating") {
            if (r->rating >= minVal && r->rating <= maxVal) match = true;
        }

        if (match) {
            r->print();
            count++;
        }
    }
    if (count == 0) std::cout << "No resources matched." << std::endl;
}

// --- NEW FEATURE: CURRICULUM PLANNER ---
void Engine::handlePlan(const std::string& json) {
    std::string val = extractValue(json, "value");
    if (val.empty()) {
        std::cout << "Error: Plan requires a Target ID." << std::endl;
        return;
    }

    int targetID = std::stoi(val);

    // 1. Ask Graph for the Topological Sort path
    // Note: We don't pass AVLTree here because the graph now has its own map (per your update)
    // OR if you kept my previous suggestion, pass storageTree.
    // Assuming new KnowledgeGraph has 'getCurriculum(int targetID)':
    std::vector<int> path = depGraph->getCurriculum(targetID);

    if (path.empty()) {
        std::cout << "Could not generate plan (Target ID not found or Cycle detected)." << std::endl;
        return;
    }

    std::cout << "\n==============================================" << std::endl;
    std::cout << " 🎓 OPTIMIZED LEARNING PATH (Linear Order) " << std::endl;
    std::cout << "==============================================" << std::endl;

    int step = 1;
    for (int id : path) {
        // Use AVL Tree for O(log n) lookup of details
        Resource* r = storageTree->search(id);
        if (r) {
            std::cout << "Step " << step++ << ": [" << r->id << "] " << r->title
                      << " (" << r->topic << ")" << std::endl;
        }
    }
    std::cout << "==============================================" << std::endl;
}

// =========================================================
// UTILITIES
// =========================================================
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