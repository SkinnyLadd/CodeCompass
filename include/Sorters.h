#ifndef SORTERS_H
#define SORTERS_H

#include <vector>
#include <string>
#include <algorithm> // For std::swap
#include "Resource.h"

// =========================================================
// SORTERS MODULE
// Responsibility: Sort vectors of Resources based on different
// criteria (Difficulty, Title) using custom algorithms.
// =========================================================

namespace Sorters {

    // ---------------------------------------------------------
    // QUICK SORT (Sort by Difficulty - Ascending)
    // ---------------------------------------------------------

    // Partition helper for QuickSort
    int partition(std::vector<Resource*>& res, int low, int high) {
        int pivot = res[high]->difficulty; // Pivot is the last element
        int i = (low - 1);

        for (int j = low; j <= high - 1; j++) {
            // If current element is smaller than the pivot
            if (res[j]->difficulty < pivot) {
                i++;
                std::swap(res[i], res[j]);
            }
        }
        std::swap(res[i + 1], res[high]);
        return (i + 1);
    }

    // Main QuickSort function
    void quickSort(std::vector<Resource*>& res, int low, int high) {
        if (low < high) {
            int pi = partition(res, low, high);

            // Recursively sort elements before and after partition
            quickSort(res, low, pi - 1);
            quickSort(res, pi + 1, high);
        }
    }

    // Wrapper for ease of use
    void sortByDifficulty(std::vector<Resource*>& res) {
        if (res.empty()) return;
        quickSort(res, 0, res.size() - 1);
    }

    // ---------------------------------------------------------
    // MERGE SORT (Sort by Title - Alphabetical)
    // ---------------------------------------------------------

    void merge(std::vector<Resource*>& res, int left, int mid, int right) {
        int n1 = mid - left + 1;
        int n2 = right - mid;

        // Create temp vectors
        std::vector<Resource*> L(n1);
        std::vector<Resource*> R(n2);

        // Copy data
        for (int i = 0; i < n1; i++)
            L[i] = res[left + i];
        for (int j = 0; j < n2; j++)
            R[j] = res[mid + 1 + j];

        // Merge the temp vectors back
        int i = 0, j = 0, k = left;
        while (i < n1 && j < n2) {
            // String comparison for alphabetical order
            if (L[i]->topic <= R[j]->topic) {
                res[k] = L[i];
                i++;
            } else {
                res[k] = R[j];
                j++;
            }
            k++;
        }

        // Copy remaining elements
        while (i < n1) {
            res[k] = L[i];
            i++;
            k++;
        }
        while (j < n2) {
            res[k] = R[j];
            j++;
            k++;
        }
    }

    void mergeSort(std::vector<Resource*>& res, int left, int right) {
        if (left < right) {
            int mid = left + (right - left) / 2;

            mergeSort(res, left, mid);
            mergeSort(res, mid + 1, right);
            merge(res, left, mid, right);
        }
    }

    // Wrapper
    void sortByTopic(std::vector<Resource*>& res) {
        if (res.empty()) return;
        mergeSort(res, 0, res.size() - 1);
    }
}

#endif