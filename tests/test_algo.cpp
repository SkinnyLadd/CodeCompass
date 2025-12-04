#include "../include/CSVParser.h"
#include "../include/Sorters.h"
#include <iostream>
#include <iomanip> // For std::setw
#include <vector>
#include <chrono>  // For timing
#include <random>  // For generating large datasets

// Helper to print list
void printList(const std::vector<Resource*>& list, std::string label) {
    std::cout << "\n--- " << label << " ---\n";
    // Print up to 5 items to avoid cluttering the console
    for(int i=0; i< std::min((int)list.size(), 5); i++) {
        std::cout << "ID: " << std::setw(3) << list[i]->id
                  << " | Diff: " << std::setw(3) << list[i]->difficulty
                  << " | Title: " << list[i]->title << "\n";
    }
    if (list.size() > 5) std::cout << "... (" << list.size() - 5 << " more items)\n";
}

// ---------------------------------------------------------
// RIGOROUS TESTING FUNCTIONS
// ---------------------------------------------------------

void testEdgeCases() {
    std::cout << "\n=============================================\n";
    std::cout << "   RUNNING EDGE CASE TESTS\n";
    std::cout << "=============================================\n";

    std::vector<int> dummyPrereqs;
    std::vector<Resource*> emptyList;

    // 1. Empty Vector
    Sorters::sortByDifficulty(emptyList); // Should not crash
    Sorters::sortByTitle(emptyList);      // Should not crash
    std::cout << "[PASS] Edge Case [Empty List]: Passed (No Crash)\n";

    // 2. Single Element
    std::vector<Resource*> singleList;
    singleList.push_back(new Resource(1, "Solo", "u", "t", 50, 5.0, dummyPrereqs));
    Sorters::sortByDifficulty(singleList);
    if(singleList.size() == 1 && singleList[0]->difficulty == 50)
        std::cout << "[PASS] Edge Case [Single Element]: Passed\n";
    else
        std::cout << "[FAIL] Edge Case [Single Element]: Failed\n";
    delete singleList[0];

    // 3. Duplicates
    std::vector<Resource*> dupList;
    dupList.push_back(new Resource(1, "A", "u", "t", 20, 5.0, dummyPrereqs));
    dupList.push_back(new Resource(2, "B", "u", "t", 20, 5.0, dummyPrereqs)); // Duplicate Diff
    dupList.push_back(new Resource(3, "C", "u", "t", 10, 5.0, dummyPrereqs));

    Sorters::sortByDifficulty(dupList);

    // Check if sorted (10, 20, 20)
    if(dupList[0]->difficulty == 10 && dupList[1]->difficulty == 20 && dupList[2]->difficulty == 20)
        std::cout << "[PASS] Edge Case [Duplicates]: Passed\n";
    else
        std::cout << "[FAIL] Edge Case [Duplicates]: Failed\n";

    for(auto* r : dupList) delete r;
}

void comparePerformance() {
    std::cout << "\n=============================================\n";
    std::cout << "   ALGORITHM PERFORMANCE SHOWDOWN\n";
    std::cout << "=============================================\n";

    // Generate 10,000 Dummy Resources
    const int N = 10000;
    std::cout << "Generating " << N << " random resources in memory...\n";

    std::vector<Resource*> datasetQuick;
    std::vector<Resource*> datasetMerge;
    std::vector<int> dummyPrereqs;

    // Random Number Generator
    std::mt19937 rng(42); // Seed 42 for consistency
    std::uniform_int_distribution<int> diffDist(1, 100); // Difficulty 1-100

    for(int i = 0; i < N; i++) {
        int randDiff = diffDist(rng);
        std::string title = "Resource_" + std::to_string(N - i); // Reverse sorted titles

        Resource* r = new Resource(i, title, "url", "topic", randDiff, 4.5, dummyPrereqs);

        // Push to both vectors (pointing to same objects)
        datasetQuick.push_back(r);
        datasetMerge.push_back(r);
    }

    // --- TIMING QUICKSORT ---
    std::cout << ">> Running QuickSort (Integer Compare) on " << N << " items...\n";
    auto startQ = std::chrono::high_resolution_clock::now();

    Sorters::sortByDifficulty(datasetQuick);

    auto endQ = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> durationQ = endQ - startQ;

    // --- TIMING MERGESORT ---
    std::cout << ">> Running MergeSort (String Compare) on " << N << " items...\n";
    auto startM = std::chrono::high_resolution_clock::now();

    Sorters::sortByTitle(datasetMerge);

    auto endM = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> durationM = endM - startM;

    // --- REPORT ---
    std::cout << "\n[Results]\n";
    std::cout << "QuickSort Time: " << std::fixed << std::setprecision(2) << durationQ.count() << " ms\n";
    std::cout << "MergeSort Time: " << std::fixed << std::setprecision(2) << durationM.count() << " ms\n";

    std::cout << "\n[Complexity Analysis]\n";
    std::cout << "* QuickSort: Avg O(n log n), Worst O(n^2). Sorts Integers (Fast).\n";
    std::cout << "* MergeSort: Always O(n log n). Sorts Strings (Slower comparison overhead).\n";

    // Cleanup memory
    for(auto* r : datasetQuick) delete r;
}

int main() {
    // ---------------------------------------------------------
    // PHASE 1: Verify Correctness on Real Data
    // ---------------------------------------------------------
    std::cout << "Loading data from data/resources.csv...\n";
    std::vector<Resource*> data = CSVParser::loadResources("data/resources.csv");

    if (data.empty()) {
        std::cerr << "CRITICAL ERROR: No data loaded. Check Working Directory in CLion.\n";
        return 1;
    }

    std::cout << "Successfully loaded " << data.size() << " resources.\n";
    printList(data, "Original Order (from CSV)");

    // Test Quick Sort
    std::cout << "\n>> Verifying QuickSort (by Difficulty)...\n";
    Sorters::sortByDifficulty(data);

    bool sortedDiff = true;
    for(size_t i=0; i<data.size()-1; i++) {
        if(data[i]->difficulty > data[i+1]->difficulty) {
            sortedDiff = false; break;
        }
    }
    if(sortedDiff) std::cout << "[PASS] QuickSort Logic: Valid\n";
    else std::cout << "[FAIL] QuickSort Logic: Invalid\n";

    // Test Merge Sort
    std::cout << "\n>> Verifying MergeSort (by Title)...\n";
    Sorters::sortByTitle(data);

    bool sortedTitle = true;
    for(size_t i=0; i<data.size()-1; i++) {
        if(data[i]->title > data[i+1]->title) {
            sortedTitle = false; break;
        }
    }
    if(sortedTitle) std::cout << "[PASS] MergeSort Logic: Valid\n";
    else std::cout << "[FAIL] MergeSort Logic: Invalid\n";

    // Cleanup Phase 1 pointers
    for(auto* r : data) delete r;

    // ---------------------------------------------------------
    // PHASE 2 & 3: Rigorous Tests
    // ---------------------------------------------------------
    testEdgeCases();
    comparePerformance();

    return 0;
}