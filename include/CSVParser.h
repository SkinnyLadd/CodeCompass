#ifndef CSVPARSER_H
#define CSVPARSER_H

#include <vector>
#include <string>
#include "Resource.h"

// =========================================================
// CSV PARSER MODULE
// Responsibility: Read raw text files and convert them into
// usable C++ objects for the Engine.
// =========================================================

namespace CSVParser {

#ifndef CSVPARSER_H
#define CSVPARSER_H

#include <vector>
#include <string>
#include "Resource.h"

    // =========================================================
    // CSV PARSER MODULE
    // Responsibility: Read raw text files and convert them into
    // usable C++ objects for the Engine.
    // =========================================================

    namespace CSVParser {

        /**
         * Loads all resources from the specified CSV file.
         * * @param filename Path to the CSV file (e.g., "data/resources.csv")
         * @return A vector of pointers to dynamically allocated Resource objects.
         * (The caller is responsible for eventual memory cleanup).
         */
        std::vector<Resource*> loadResources(const std::string& filename);

    }

#endif/**
     * Loads all resources from the specified CSV file.
     * * @param filename Path to the CSV file (e.g., "data/resources.csv")
     * @return A vector of pointers to dynamically allocated Resource objects.
     * (The caller is responsible for eventual memory cleanup).
     */
    std::vector<Resource*> loadResources(const std::string& filename);

}

#endif