#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm> // Required for std::transform (case insensitivity)
#include "Resource.h"
#include "CSVParser.h"
#include "Engine.h"
#include "Optimizer.h" // Include the new Optimizer

using namespace std;

bool fileExists(const string& name) {
    ifstream f(name.c_str());
    return f.good();
}

int main(int argc, char* argv[]) {
    // 1. Find CSV
    string csvPath = "data/resources.csv";
    if (!fileExists(csvPath)) csvPath = "../data/resources.csv";
    if (!fileExists(csvPath)) csvPath = "../../data/resources.csv";

    // 2. Load
    vector<Resource*> data = CSVParser::loadResources(csvPath);
    if (data.empty()) {
        cout << "Error: Could not load resources.csv" << endl;
        return 1;
    }

    // 3. Init Engine
    Engine engine(data);

    // 4. Run
    if (argc > 1) {
        // Python/External Mode
        engine.execute(argv[1]);
    } else {
        // --- CONSOLE INTERACTIVE MODE ---
        cout << "========================================" << endl;
        cout << "      CODE COMPASS CONSOLE MODE" << endl;
        cout << "========================================" << endl;
        cout << "Commands:" << endl;
        cout << "  LIST            : Show all resources" << endl;
        cout << "  PLAN|Title      : Generate a study path for a title" << endl;
        cout << "  CRAM            : Optimize study schedule (Knapsack)" << endl;
        cout << "  exit            : Quit" << endl;
        cout << "----------------------------------------" << endl;

        string line;
        while (true) {
            cout << "> ";
            if (!getline(cin, line)) break;
            if (line == "exit") break;

            if (line == "CRAM") {
                // --- CRAMMING MODE LOGIC ---
                string inputTopic;
                int inputTime;

                cout << "\n--- CRAMMING MODE ---" << endl;
                cout << "Enter Topic (e.g., Trees, Graphs, DP): ";
                getline(cin, inputTopic);

                cout << "Enter Time Constraint (minutes): ";
                cin >> inputTime;
                cin.ignore(); // Clear newline from buffer

                // 1. Filter resources by Topic
                vector<Resource*> topicResources;
                for (Resource* r : data) {
                    // Simple substring check or exact match
                    if (r->topic == inputTopic) {
                        topicResources.push_back(r);
                    }
                }

                if (topicResources.empty()) {
                    cout << "No resources found for topic: " << inputTopic << endl;
                } else {
                    // 2. Call Optimizer
                    cout << "Optimizing " << topicResources.size() << " resources for max rating..." << endl;
                    vector<Resource*> bestPlan = Optimizer::maximizeRating(topicResources, inputTime);

                    // 3. Display Result
                    cout << "\nRECOMMENDED CRAMMING PLAN:" << endl;
                    cout << "ID,Title,Duration,Rating" << endl; // CSV-style header

                    int totalTime = 0;
                    double totalScore = 0.0;

                    for (Resource* r : bestPlan) {
                        cout << r->id << ","
                             << r->title << ","
                             << r->duration << "m,"
                             << r->rating << endl;
                        totalTime += r->duration;
                        totalScore += r->rating;
                    }
                    cout << "----------------------------------------" << endl;
                    cout << "Total Time: " << totalTime << " / " << inputTime << " min" << endl;
                    cout << "Total Rating: " << totalScore << endl;
                }
                cout << "\n";
            }
            else {
                // Pass standard commands to Engine
                engine.execute(line);
            }
        }
    }

    // Cleanup
    // Note: Engine destructor handles its internal pointers,
    // but the 'data' vector owns the raw Resource pointers in this main scope.
    for (auto* r : data) delete r;
    return 0;
}