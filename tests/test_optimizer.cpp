#include "../include/Resource.h"
#include "../include/Optimizer.h"
#include <iostream>
#include <vector>
#include <numeric> // For accumulate

// Helper to calculate total rating of a result list
double calculateTotalRating(const std::vector<Resource*>& resources) {
    double total = 0;
    for (const auto* r : resources) {
        total += r->rating;
    }
    return total;
}

// Helper to calculate total duration
int calculateTotalTime(const std::vector<Resource*>& resources) {
    int total = 0;
    for (const auto* r : resources) {
        total += r->duration;
    }
    return total;
}

void testStandardKnapsack() {
    std::cout << "\n[TEST 1] Standard Knapsack Scenario (The 'Greedy Trap')\n";
    std::cout << "-----------------------------------------------------\n";

    std::vector<int> emptyPrereqs;

    // Scenario: You have 30 minutes.
    // Item A: 10 mins, Rating 2.0
    // Item B: 20 mins, Rating 3.0
    // Item C: 30 mins, Rating 4.0

    // Logic:
    // - Choosing C takes 30 mins -> Total Rating 4.0
    // - Choosing A + B takes 30 mins -> Total Rating 5.0 (BETTER!)

    Resource* rA = new Resource(1, "A", "url", "topic", 10, 2.0, emptyPrereqs, 10);
    Resource* rB = new Resource(2, "B", "url", "topic", 10, 3.0, emptyPrereqs, 20);
    Resource* rC = new Resource(3, "C", "url", "topic", 10, 4.0, emptyPrereqs, 30);

    std::vector<Resource*> items = {rA, rB, rC};

    // Run Optimizer with 30 minutes limit
    std::vector<Resource*> plan = Optimizer::maximizeRating(items, 30);

    double totalRating = calculateTotalRating(plan);
    int totalTime = calculateTotalTime(plan);

    std::cout << "Selected Items Count: " << plan.size() << "\n";
    std::cout << "Total Rating: " << totalRating << " (Expected: 5.0)\n";
    std::cout << "Total Time: " << totalTime << " (Expected: 30)\n";

    if (totalRating == 5.0) std::cout << "✅ PASS: Correctly chose combination (A+B) over single item (C).\n";
    else std::cout << "❌ FAIL: Did not find optimal solution.\n";

    // Cleanup
    delete rA; delete rB; delete rC;
}

void testEdgeCases() {
    std::cout << "\n[TEST 2] Edge Cases (Empty / Zero Time)\n";
    std::cout << "--------------------------------------\n";

    std::vector<int> emptyPrereqs;
    Resource* r1 = new Resource(1, "Big", "url", "topic", 10, 5.0, emptyPrereqs, 60);
    std::vector<Resource*> items = {r1};

    // Case A: 0 Time available
    std::vector<Resource*> resultZero = Optimizer::maximizeRating(items, 0);
    if (resultZero.empty()) std::cout << "✅ PASS: Zero time returns empty list.\n";
    else std::cout << "❌ FAIL: Zero time returned items.\n";

    // Case B: Time less than smallest item
    std::vector<Resource*> resultSmall = Optimizer::maximizeRating(items, 30); // Item needs 60
    if (resultSmall.empty()) std::cout << "✅ PASS: Insufficient time returns empty list.\n";
    else std::cout << "❌ FAIL: Insufficient time returned items.\n";

    delete r1;
}

int main() {
    testStandardKnapsack();
    testEdgeCases();
    return 0;
}