#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream> // Required for parsing
#include <algorithm>
#include "Resource.h"
#include "CSVParser.h"
#include "Engine.h"
#include "Optimizer.h"

using namespace std;

bool fileExists(const string& name) {
    ifstream f(name.c_str());
    return f.good();
}

// Helper to split string by delimiter
vector<string> split(const string& s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

int main(int argc, char* argv[]) {
    // 1. Find CSV
    string csvPath = "data/resources.csv";
    if (!fileExists(csvPath)) csvPath = "../data/resources.csv";
    if (!fileExists(csvPath)) csvPath = "../../data/resources.csv";

    // 2. Load
    vector<Resource*> data = CSVParser::loadResources(csvPath);
    if (data.empty()) {
        // [CRITICAL CHANGE] Use cerr for errors so frontend doesn't parse this as data
        cerr << "Error: Could not load resources.csv from " << csvPath << endl;
        return 1;
    }

    // 3. Init Engine
    Engine engine(data);

    // 4. Run
    if (argc > 1) {
        // --- HEADLESS / FRONTEND MODE ---
        // Expecting command: "PLAN|Title" or "CRAM|Topic|Time"
        string command = argv[1];

        if (command.rfind("CRAM", 0) == 0) {
            // Parse "CRAM|Topic|Time"
            vector<string> parts = split(command, '|');

            if (parts.size() < 3) {
                cerr << "Error: Invalid CRAM command format. Use CRAM|Topic|Time" << endl;
                return 1;
            }

            string inputTopic = parts[1];
            int inputTime = 0;
            try {
                inputTime = stoi(parts[2]);
            } catch (...) {
                cerr << "Error: Invalid time format." << endl;
                return 1;
            }

            // Filter
            vector<Resource*> topicResources;
            for (Resource* r : data) {
                if (r->topic == inputTopic) {
                    topicResources.push_back(r);
                }
            }

            // Optimize
            vector<Resource*> bestPlan = Optimizer::maximizeRating(topicResources, inputTime);

            // Output PURE CSV for Frontend (No "Recommended Plan" text)
            cout << "ID,Title,Duration,Rating" << endl;
            for (Resource* r : bestPlan) {
                cout << r->id << ","
                     << r->title << ","
                     << r->duration << ","  // Raw number for frontend
                     << r->rating << endl;
            }

        } else {
            // Pass other commands (LIST, PLAN) to Engine
            engine.execute(command);
        }

    } else {
        // --- CONSOLE INTERACTIVE MODE ---
        cout << "========================================" << endl;
        cout << "      CODE COMPASS CONSOLE MODE" << endl;
        cout << "========================================" << endl;
        // ... (Keep your existing interactive logic here if you want) ...
        cout << "Commands: LIST, PLAN|Title, CRAM, exit" << endl;

        string line;
        while (true) {
            cout << "> ";
            if (!getline(cin, line)) break;
            if (line == "exit") break;

            if (line == "CRAM") {
                // Interactive Cramming (Pretty Print)
                string inputTopic;
                int inputTime;
                cout << "Enter Topic: "; getline(cin, inputTopic);
                cout << "Enter Time: "; cin >> inputTime; cin.ignore();

                vector<Resource*> topicResources;
                for (Resource* r : data) {
                    if (r->topic == inputTopic) topicResources.push_back(r);
                }

                vector<Resource*> bestPlan = Optimizer::maximizeRating(topicResources, inputTime);

                cout << "\nRECOMMENDED PLAN:\n";
                for (Resource* r : bestPlan) {
                     cout << "- " << r->title << " (" << r->duration << "m)\n";
                }
                cout << endl;
            } else {
                engine.execute(line);
            }
        }
    }

    // Cleanup
    for (auto* r : data) delete r;
    return 0;
}