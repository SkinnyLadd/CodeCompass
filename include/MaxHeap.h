#ifndef MAXHEAP_H
#define MAXHEAP_H

#include "Resource.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>

class MaxHeap {
private:
    std::vector<Resource*> heap;

    void heapifyUp(int index) {
        if (index > 0) {
            int parent = (index - 1) / 2;
            // MaxHeap property: parent rating must be >= child rating
            if (heap[index]->rating > heap[parent]->rating) {
                std::swap(heap[index], heap[parent]);
                heapifyUp(parent);
            }
        }
    }

    void heapifyDown(int index) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int largest = index;

        if (left < heap.size() && heap[left]->rating > heap[largest]->rating) {
            largest = left;
        }

        if (right < heap.size() && heap[right]->rating > heap[largest]->rating) {
            largest = right;
        }

        if (largest != index) {
            std::swap(heap[index], heap[largest]);
            heapifyDown(largest);
        }
    }

    // Helper function to print the current state of the heap's internal array
    void printHeapState(const std::string& operation) const {
        std::cout << "\n--- MaxHeap State after " << operation << " (Size: " << heap.size() << ") ---" << std::endl;
        std::cout << "  (Internal Array Representation, Max Element is always first)" << std::endl;
        if (heap.empty()) {
            std::cout << "Heap is empty." << std::endl;
        } else {
            std::cout << "  [";
            for (size_t i = 0; i < heap.size(); ++i) {
                std::cout << heap[i]->title << " (R=" << heap[i]->rating << ")";
                if (i < heap.size() - 1) {
                    std::cout << ", ";
                }
            }
            std::cout << "]" << std::endl;
        }
        std::cout << "---------------------------------------------------------" << std::endl;
    }


public:
    // Added doPrint flag
    void insert(Resource* res, bool doPrint = true) {
        heap.push_back(res);
        heapifyUp(heap.size() - 1);
       /* if (doPrint) {
            printHeapState("INSERT (Resource ID: " + std::to_string(res->id) + ")");
        }*/
    }

    // Added doPrint flag
    Resource* extractMax(bool doPrint = true) {
        if (heap.empty()) {
            if (doPrint) {
                printHeapState("EXTRACT_MAX (Heap Empty)");
            }
            return nullptr;
        }
        Resource* max = heap.front();

        // Move last element to the root
        heap.front() = heap.back();
        heap.pop_back();

        // Fix the heap property
        heapifyDown(0);

        if (doPrint) {
            printHeapState("EXTRACT_MAX (Extracted ID: " + std::to_string(max->id) + ")");
        }
        return max;
    }

    bool isEmpty() const {
        return heap.empty();
    }

    // Export structure for visualization
    std::vector<std::string> getStructure() {
        std::vector<std::string> result;
        result.push_back("HEAP_SIZE:" + std::to_string(heap.size()));
        
        for (size_t i = 0; i < heap.size(); ++i) {
            std::string nodeInfo = "NODE:" + std::to_string(heap[i]->id) + ":" + 
                                  std::to_string(heap[i]->rating) + ":" + 
                                  std::to_string(i);
            result.push_back(nodeInfo);
            
            int left = 2 * i + 1;
            int right = 2 * i + 2;
            if (left < (int)heap.size()) {
                result.push_back("EDGE:" + std::to_string(heap[i]->id) + "->" + 
                               std::to_string(heap[left]->id) + ":L");
            }
            if (right < (int)heap.size()) {
                result.push_back("EDGE:" + std::to_string(heap[i]->id) + "->" + 
                               std::to_string(heap[right]->id) + ":R");
            }
        }
        return result;
    }

    int getSize() const {
        return heap.size();
    }

    Resource* getMax() const {
        return heap.empty() ? nullptr : heap[0];
    }
};

#endif