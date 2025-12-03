#ifndef RESOURCE_H
#define RESOURCE_H

#include <string>
#include <vector>
#include <iostream>

using namespace std;

// =========================================================
// THE RESOURCE CLASS
// This is the fundamental unit of data for "CodeCompass".
// All Data Structures (AVL, Cache, Graph) will point to objects of this type.
// =========================================================

struct Resource {
    int id;                     // Unique ID (Used for AVL Tree balancing)
    string title;          // e.g., "Introduction to BST"
    string url;            // e.g., "https://youtube.com/..."
    string topic;          // Used for Trie Indexing (e.g., "Trees")

    int difficulty;             // 1-100 Scale (Used for QuickSort/MergeSort)
    double rating;              // 1.0-5.0 Scale (Used for Max-Heap Priority Queue)

    vector<int> prereqIDs; // List of Resource IDs required to learn this.
    // Used for Graph Construction (Adjacency List) & Dijkstra.

    // Constructor for easy creation
    Resource(int i, string t, string u, string top, int diff, double rate, vector<int> pre)
        : id(i), title(t), url(u), topic(top), difficulty(diff), rating(rate), prereqIDs(pre) {}

    // Helper to print resource details (Useful for debugging)
    void print() const {
        cout << "[ID: " << id << "] " << title
                  << " | Diff: " << difficulty
                  << " | Rating: " << rating << endl;
    }
};

#endif