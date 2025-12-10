#include <iostream>
#include <string>
#include <vector>

// Note: Because of include_directories(include), we can use <Header.h> or "Header.h"
#include "Resource.h"
#include "CSVParser.h"
#include "Engine.h"

int main(int argc, char* argv[]) {
    // 1. Determine Input (CLI Argument or Manual Test)
    std::string command;
    if (argc > 1) {
        command = argv[1]; // Received from Python
    } else {
        // Fallback for testing directly in CLion
        command = "{ \"action\": \"count\" }";
    }

    // 2. Load Data
    // Ensure 'data/resources.csv' exists relative to the executable
    std::string csvPath = "data/resources.csv";
    std::vector<Resource*> data = CSVParser::loadResources(csvPath);

    if (data.empty()) {
        std::cerr << "CRITICAL: No data loaded. Check file path: " << csvPath << std::endl;
        // Proceeding anyway (Engine will just report 0 items)
    }

    // 3. Initialize Engine
    Engine engine(data);

    // 4. Execute Command
    engine.execute(command);

    // 5. Cleanup
    for (auto r : data) delete r;

    return 0;
}