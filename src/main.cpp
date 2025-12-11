#include <iostream>
#include <string>
#include <vector>
#include <limits> // for numeric_limits
#include "Resource.h"
#include "CSVParser.h"
#include "Engine.h"

// Helper to manually create JSON strings for the Engine
std::string makeJson(std::string action, std::string value = "") {
    return "{ \"action\": \"" + action + "\", \"value\": \"" + value + "\" }";
}

int main(int argc, char* argv[]) {
    // 1. Load Data
    std::string csvPath = "data/resources.csv";
    std::vector<Resource*> data = CSVParser::loadResources(csvPath);

    if (data.empty()) {
        std::cerr << "CRITICAL: No data loaded from " << csvPath << std::endl;
        return 1;
    }

    // 2. Initialize Engine
    Engine engine(data);

    // 3. IF PYTHON CALLS US (Arguments provided), execute and exit
    if (argc > 1) {
        engine.execute(argv[1]);
        return 0;
    }

    // 4. IF HUMAN RUNS US (Console Menu Mode)
    while (true) {
        std::cout << "\n===================================" << std::endl;
        std::cout << "      CODE COMPASS CONSOLE         " << std::endl;
        std::cout << "===================================" << std::endl;
        std::cout << "1. Selection Mode (View All Resources)" << std::endl;
        std::cout << "2. Plan Mode (Generate Curriculum)" << std::endl;
        std::cout << "3. Search (Find by Keyword)" << std::endl;
        std::cout << "4. Exit" << std::endl;
        std::cout << "Select Option: ";

        int choice;
        if (!(std::cin >> choice)) {
            // Handle non-integer input to prevent infinite loop
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        if (choice == 4) break;

        if (choice == 1) {
            // MODE 1: SELECTION
            std::cout << "\n--- RESOURCES LIST ---" << std::endl;
            engine.execute(makeJson("list"));
        }
        else if (choice == 2) {
            // MODE 2: PLAN
            std::string id;
            std::cout << "Enter Target Resource ID (e.g., 105): ";
            std::cin >> id;
            engine.execute(makeJson("plan", id));
        }
        else if (choice == 3) {
            // SEARCH
            std::string term;
            std::cout << "Enter Topic/Keyword: ";
            std::cin >> term;
            engine.execute(makeJson("search", term));
        }
        else {
            std::cout << "Invalid option." << std::endl;
        }
    }

    // Cleanup
    for (auto* r : data) delete r;
    return 0;
}