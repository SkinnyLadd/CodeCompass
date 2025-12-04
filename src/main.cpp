#include <iostream>
#include <string>
#include <vector>
#include <limits> // For clearing buffer
#include "Resource.h"
#include "CSVParser.h"
#include "Sorters.h"

// =========================================================
// SKELETON ENGINE
// This acts as a placeholder until the other bhondumans finish their work.
// =========================================================

void printResource(Resource* r) {
    // Format: ID|Title|URL|Difficulty|Rating
    // This specific format makes it easy for Python to split the string
    std::cout << r->id << "|" << r->title << "|" << r->url << "|"
              << r->difficulty << "|" << r->rating << std::endl;
}

int main() {
    // 1. Load Data (This is REAL logic)
    std::vector<Resource*> masterData = CSVParser::loadResources("data/resources.csv");

    // 2. Mock Initialization (Member 1 & 2 & 3 Placeholder)
    std::cerr << "[CPP] Engine Initialized. Loaded " << masterData.size() << " resources." << std::endl;

    // 3. The Controller Loop
    std::string command;
    while (true) {
        // Wait for input from Python (or user typing in terminal)
        if (!std::getline(std::cin, command)) break;

        if (command == "EXIT") break;

        // 4. MOCK SEARCH LOGIC
        // Later, this will be: vector<int> ids = trie.getSuggestions(command);
        std::cerr << "[CPP] Received command: " << command << std::endl; // Debug log

        bool found = false;
        // Simple linear search to simulate a Trie lookup
        for (Resource* r : masterData) {
            // Check if title starts with the command (case-sensitive for now)
            if (r->title.find(command) == 0) {
                printResource(r);
                found = true;
            }
        }

        if (!found) {
            std::cout << "NO_RESULTS" << std::endl;
        }

        // CRITICAL: FLUSH STDOUT
        // If you don't do this, Python will hang waiting for the line.
        std::cout << "END_OF_RESULTS" << std::endl; // Signal that we are done sending rows
        std::cout.flush();
    }

    // Cleanup
    for (Resource* r : masterData) delete r;
    return 0;
}