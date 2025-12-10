#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "Resource.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath> // For checking double equality

namespace Optimizer {

    /**
     * Solves the 0/1 Knapsack Problem to maximize total Rating within maxDuration.
     * * @param items Vector of available resources.
     * @param maxMinutes The time constraint.
     * @return A vector of the selected resources that yield the highest rating.
     */
    std::vector<Resource*> maximizeRating(const std::vector<Resource*>& items, int maxMinutes) {
        int n = items.size();

        // DP Table: dp[i][w] stores the max rating using a subset of the first 'i' items
        // with a total duration limit of 'w'.
        // Rows: 0 to n (items)
        // Cols: 0 to maxMinutes (time)
        std::vector<std::vector<double>> dp(n + 1, std::vector<double>(maxMinutes + 1, 0.0));

        // ---------------------------------------------------------
        // 1. Build the DP Table
        // ---------------------------------------------------------
        for (int i = 1; i <= n; i++) {
            Resource* r = items[i - 1]; // items is 0-indexed
            int weight = r->duration;
            double value = r->rating;

            for (int w = 0; w <= maxMinutes; w++) {
                if (weight <= w) {
                    // Option A: Include item (Value of item + Best value with remaining weight)
                    double include = value + dp[i - 1][w - weight];

                    // Option B: Exclude item (Best value found so far without this item)
                    double exclude = dp[i - 1][w];

                    dp[i][w] = std::max(include, exclude);
                } else {
                    // Cannot include item because it takes too much time
                    dp[i][w] = dp[i - 1][w];
                }
            }
        }

        // ---------------------------------------------------------
        // 2. Backtrack to Find Selected Items
        // ---------------------------------------------------------
        std::vector<Resource*> selectedItems;
        int w = maxMinutes;

        for (int i = n; i > 0; i--) {
            // If the value came from the row above, we didn't include item 'i'
            // We use a small epsilon for double comparison safety, though != usually works fine here.
            if (std::abs(dp[i][w] - dp[i - 1][w]) > 1e-9) {
                Resource* r = items[i - 1];
                selectedItems.push_back(r);
                w -= r->duration; // Reduce remaining capacity
            }
        }

        return selectedItems;
    }
}

#endif