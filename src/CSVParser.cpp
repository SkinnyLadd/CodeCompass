#include "CSVParser.h"
#include <fstream>   // For file handling
#include <sstream>   // For string stream processing
#include <iostream>  // For error logging

namespace CSVParser {

    // Helper function to split a string by a specific delimiter
    // Input: "101;102", Delimiter: ';' -> Output: {101, 102}
    std::vector<int> parsePrereqs(const std::string& prereqString) {
        std::vector<int> ids;

        // If empty, return empty vector
        if (prereqString.empty()) return ids;

        std::stringstream ss(prereqString);
        std::string segment;

        // Split by semicolon ';'
        while (std::getline(ss, segment, ';')) {
            try {
                if (!segment.empty()) {
                    ids.push_back(std::stoi(segment));
                }
            } catch (...) {
                std::cerr << "Warning: Could not parse prereq ID: " << segment << std::endl;
            }
        }
        return ids;
    }

    std::vector<Resource*> loadResources(const std::string& filename) {
        std::vector<Resource*> resources;
        std::ifstream file(filename);

        // 1. Check if file opened successfully
        if (!file.is_open()) {
            std::cerr << "CRITICAL ERROR: Could not open file: " << filename << std::endl;
            std::cerr << "Tip: Check your 'Working Directory' in CLion configurations." << std::endl;
            return resources;
        }

        std::string line;
        // 2. Skip the Header line (ID,Title,URL...)
        std::getline(file, line);

        // 3. Process line by line
        while (std::getline(file, line)) {
            if (line.empty()) continue;

            std::stringstream ss(line);
            std::string segment;
            std::vector<std::string> rowData;

            // 4. Split line by Comma ',' to get columns
            while (std::getline(ss, segment, ',')) {
                rowData.push_back(segment);
            }

            // Validation: Ensure we have at least 8 columns (ID to Duration)
            if (rowData.size() < 8) {
                // Handle cases where the last column (Prereqs) might be empty/missing
                if(rowData.size() == 6) rowData.push_back("");
                else continue; // Skip malformed rows
            }

            try {
                // 5. Extract and Convert Data
                int id = std::stoi(rowData[0]);
                std::string title = rowData[1];
                std::string url = rowData[2];
                std::string topic = rowData[3];
                int difficulty = std::stoi(rowData[4]);
                double rating = std::stod(rowData[5]);
                int duration = std::stoi(rowData[7]);

                // 6. Special Handling: Parse the Semicolon-separated Prereqs
                std::vector<int> prereqs = parsePrereqs(rowData[6]);

                // 7. Create Object (Dynamic Allocation)
                Resource* newResource = new Resource(id, title, url, topic, difficulty, rating, prereqs, duration);

                // 8. Add to Master Vector
                resources.push_back(newResource);

            } catch (const std::exception& e) {
                std::cerr << "Error parsing row: " << line << "\nReason: " << e.what() << std::endl;
            }
        }

        file.close();
        //std::cout << "Successfully loaded " << resources.size() << " resources from " << filename << std::endl;
        return resources;
    }
}