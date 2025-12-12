#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "../include/Resource.h"
#include "../include/CSVParser.h"
#include "../include/Engine.h"

using namespace std;

// Helper to check file existence
bool fileExists(const string& name) {
    ifstream f(name.c_str());
    return f.good();
}

int main(int argc, char* argv[]) {
    // 1. Locate Data File
    string csvPath = "data/resources.csv";
    if (!fileExists(csvPath)) csvPath = "../data/resources.csv"; // Check parent
    if (!fileExists(csvPath)) csvPath = "../../data/resources.csv"; // Check grandparent

    // 2. Load Data
    vector<Resource*> data = CSVParser::loadResources(csvPath);
    if (data.empty()) {
        cerr << "Error: Could not load resources.csv from " << csvPath << endl;
        return 1;
    }

    // 3. Initialize Engine (Builds Trie, AVL, Graph, Heap)
    Engine engine(data);

    // 4. Mode Selection
    if (argc > 1) {
        // --- CLI / SUBPROCESS MODE ---
        // Now fully delegated to Engine.
        // Commands like "CRAM|Topic|Time" are handled via engine.execute()
        // utilizing Trie and Cache internally.
        string command = argv[1];
        engine.execute(command);
    }
    else {
        // --- INTERACTIVE DEBUG MODE ---
        cout << "CodeCompass Engine (Interactive)" << endl;
        cout << "Try: SEARCH|Graph, LIST|DIFFICULTY, SUGGEST|5, PLAN|Dijkstra, CRAM|Trees|60" << endl;

        string input;
        while(true) {
            cout << "> ";
            if(!getline(cin, input)) break;
            if(input == "exit") break;
            engine.execute(input);
        }
    }

    // Cleanup
    for (auto* r : data) delete r;
    return 0;
}