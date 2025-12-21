#include "../include/CSVParser.h"
#include <iostream>

int main() {
    // Test the loading function
    // Ensure "data/resources.csv" exists relative to where you run this!
    std::cout << "Loading resources from data/resources.csv...\n";
    std::vector<Resource*> data = CSVParser::loadResources("data/resources.csv");

    if (data.empty()) {
        std::cout << "Test Failed: No data loaded.\n";
        return 1;
    }

    // Print the first item to verify columns
    Resource* r = data[0];
    std::cout << "--- First Resource Loaded ---\n";
    std::cout << "ID: " << r->id << "\n";
    std::cout << "Title: " << r->title << "\n";
    std::cout << "Duration: " << r->duration << " minutes\n"; // Added check
    std::cout << "Prereq Count: " << r->prereqIDs.size() << "\n";

    // Print an item with prereqs (e.g., Dijkstra at index 9 probably)
    for(auto* item : data) {
        if (item->prereqIDs.size() > 0) {
            std::cout << "--- Resource with Prereqs Found ---\n";
            std::cout << "Title: " << item->title << " requires: ";
            for(int pid : item->prereqIDs) std::cout << pid << " ";
            std::cout << "\n";
            break;
        }
    }

    // Cleanup
    for(auto* ptr : data) delete ptr;

    return 0;
}