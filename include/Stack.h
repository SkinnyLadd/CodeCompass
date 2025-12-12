#ifndef STACK_H
#define STACK_H

#include "Resource.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>

class Stack {
private:
    std::vector<Resource*> items;

    // Helper function to print the current state of the stack
    void printStackState(const std::string& operation) const {
        std::cout << "\n--- Stack State after " << operation << " (Size: " << items.size() << ") ---" << std::endl;
        if (items.empty()) {
            std::cout << "Stack is empty." << std::endl;
        } else {
            // Print from top to bottom (LIFO order)
            std::cout << "  Top -> ";
            for (auto it = items.rbegin(); it != items.rend(); ++it) {
                std::cout << (*it)->title << " [" << (*it)->id << "] | ";
            }
            std::cout << "Bottom" << std::endl;
        }
        std::cout << "----------------------------------------------------" << std::endl;
    }

public:
    void push(Resource* res, bool doPrint = true){
        items.push_back(res);
        if (doPrint) {
            printStackState("PUSH (Resource ID: " + std::to_string(res->id) + ")");
        }
    }

    Resource* pop(bool doPrint = true) {
        if (items.empty()) {
            if (doPrint) {
                printStackState("POP (Stack Empty)");
            }
            return nullptr;
        }
        Resource* res = items.back();
        items.pop_back();
        if (doPrint) {
            printStackState("POP (Resource ID: " + std::to_string(res->id) + ")");
        }
        return res;
    }

    Resource* peek() const {
        if (items.empty()) {
            return nullptr;
        }
        return items.back();
    }

    bool isEmpty() const {
        return items.empty();
    }

    // Export structure for visualization
    std::vector<std::string> getStructure() {
        std::vector<std::string> result;
        result.push_back("SIZE:" + std::to_string(items.size()));
        
        // Stack is LIFO, so we show from top (end) to bottom (start)
        for (int i = items.size() - 1; i >= 0; --i) {
            std::string nodeInfo = std::to_string(items[i]->id) + ":" + 
                                  items[i]->title + ":" + 
                                  std::to_string(items.size() - 1 - i);
            result.push_back("ITEM:" + nodeInfo);
        }
        return result;
    }
};

#endif