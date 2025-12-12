#ifndef LRUCACHE_H
#define LRUCACHE_H

#include "Resource.h"
#include "DoublyLinkedList.h" // Assuming this includes the Node definition
#include <unordered_map>
#include <iostream>
#include <string>
#include <algorithm>

class LRUCache {
private:
    std::unordered_map<int, Node*> cacheMap;
    Node* head; // Dummy head (MRU side)
    Node* tail; // Dummy tail (LRU side)
    int capacity;

    // Helper function to print the full state of the cache (implementation remains inline)
    void printCacheState(const std::string& operation) const {
        std::cout << "\n--- LRUCache State after " << operation
                  << " (Size: " << cacheMap.size() << ", Capacity: " << capacity << ") ---" << std::endl;

        // Print Linked List (Recency Order)
        std::cout << "  Recency Order (MRU -> LRU): ";
        Node* curr = head->next;
        if (curr == tail) {
            std::cout << "Cache is empty." << std::endl;
        } else {
            while (curr != tail) {
                std::cout << curr->res->title << " [" << curr->res->id << "] ";
                if (curr->next != tail) {
                    std::cout << "<-> ";
                }
                curr = curr->next;
            }
            std::cout << std::endl;
        }

        // Print Hash Map keys
        std::cout << "  Map Keys (for verification): [";
        int count = 0;
        for (const auto& pair : cacheMap) {
            std::cout << pair.first;
            if (count++ < cacheMap.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
        std::cout << "------------------------------------------------------------------" << std::endl;
    }


    // Removes a node from its current position. O(1).
    void removeNode(Node* node) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    // Moves an existing node to the MRU position (right after head). O(1).
    void moveToHead(Node* node) {
        // Detach from current position
        removeNode(node);
        // Insert right after the dummy head
        node->next = head->next;
        node->prev = head;
        head->next->prev = node;
        head->next = node;
    }

    // Creates a new node and inserts it at the MRU position. O(1).
    void addNewNode(Resource* res) {
        Node* newNode = new Node(res);
        // Insert right after the dummy head
        newNode->next = head->next;
        newNode->prev = head;
        head->next->prev = newNode;
        head->next = newNode;
        cacheMap[res->id] = newNode;
    }

    // Evicts the LRU item (the node before the dummy tail). O(1).
    void evictLRU() {
        Node* lru = tail->prev;
        if (lru != head) {
            cacheMap.erase(lru->res->id);
            removeNode(lru);
            delete lru;
        }
    }

public:
    LRUCache(int cap) : capacity(cap) {
        // Initialize dummy head and tail
        head = new Node(nullptr);
        tail = new Node(nullptr);
        head->next = tail;
        tail->prev = head;
        //std::cout << "LRUCache initialized with capacity: " << capacity << std::endl;
    }

    ~LRUCache() {
        // Cleanup all nodes
        Node* curr = head;
        while (curr != nullptr) {
            Node* next = curr->next;
            delete curr;
            curr = next;
        }
    }

    Resource* get(int id, bool doPrint = true) {
        if (cacheMap.count(id)) {
            Node* node = cacheMap[id];
            moveToHead(node);
            if (doPrint) {
                printCacheState("GET (Cache Hit ID: " + std::to_string(id) + ")");
            }
            return node->res;
        }
        if (doPrint) {
            printCacheState("GET (Cache Miss ID: " + std::to_string(id) + ")");
        }
        return nullptr;
    }

    void put(Resource* res, bool doPrint = true) {
        if (cacheMap.count(res->id)) {
            // Already exists, update recency
            moveToHead(cacheMap[res->id]);
            if (doPrint) {
                printCacheState("PUT (Updated Recency ID: " + std::to_string(res->id) + ")");
            }
        } else {
            if (cacheMap.size() == capacity) {
                evictLRU();
            }
            addNewNode(res);
            if (doPrint) {
                printCacheState("PUT (New Entry ID: " + std::to_string(res->id) + ")");
            }
        }
    }

    int size() const {
        return cacheMap.size();
    }
};

#endif