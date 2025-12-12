#ifndef TRIE_H
#define TRIE_H

#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <utility>

struct TrieNode {
    std::unordered_map<char, TrieNode*> children;
    bool isEndOfWord;
    std::vector<int> resourceIDs; // Stores IDs of resources matching this specific word

    TrieNode() : isEndOfWord(false) {}
};

class Trie {
private:
    TrieNode* root;

    // Helper: DFS to collect all IDs in the subtree of a given node
    void collectAllIDs(TrieNode* node, std::vector<int>& results) {
        if (!node) return;

        // Add IDs found at this specific node
        for (int id : node->resourceIDs) {
            results.push_back(id);
        }

        // Recurse to children
        for (auto& pair : node->children) {
            collectAllIDs(pair.second, results);
        }
    }

public:
    Trie() { root = new TrieNode(); }

    // Insert a Topic (e.g., "Binary Search") and link it to a Resource ID
    void insert(std::string key, int id) {
        TrieNode* curr = root;
        for (char c : key) {
            c = tolower(c); // Case insensitive
            if (curr->children.find(c) == curr->children.end()) {
                curr->children[c] = new TrieNode();
            }
            curr = curr->children[c];
        }
        curr->isEndOfWord = true;
        curr->resourceIDs.push_back(id);
    }

    // The Feature You Requested: Get all IDs for a prefix (e.g., "Graph")
    std::vector<int> getResourcesByPrefix(std::string prefix) {
        TrieNode* curr = root;
        std::vector<int> results;

        // 1. Navigate to the end of the prefix
        for (char c : prefix) {
            c = tolower(c);
            if (curr->children.find(c) == curr->children.end()) {
                return results; // Prefix not found
            }
            curr = curr->children[c];
        }

        // 2. Collect all IDs descending from this point
        collectAllIDs(curr, results);
        return results;
    }

    // Export structure for visualization
    void getStructureRec(TrieNode* node, const std::string& path, std::vector<std::string>& edges, std::vector<std::string>& nodes) {
        if (!node) return;
        
        std::string nodeId = path.empty() ? "ROOT" : path;
        std::string nodeInfo = nodeId + ":" + (node->isEndOfWord ? "1" : "0") + ":" + std::to_string(node->resourceIDs.size());
        nodes.push_back(nodeInfo);
        
        for (auto& pair : node->children) {
            char c = pair.first;
            std::string childPath = path + c;
            edges.push_back(nodeId + "->" + childPath + ":" + c);
            getStructureRec(pair.second, childPath, edges, nodes);
        }
    }

    std::pair<std::vector<std::string>, std::vector<std::string>> getStructure() {
        std::vector<std::string> edges;
        std::vector<std::string> nodes;
        getStructureRec(root, "", edges, nodes);
        return std::make_pair(edges, nodes);
    }

    int getNodeCount() {
        std::vector<std::string> edges, nodes;
        getStructureRec(root, "", edges, nodes);
        return nodes.size();
    }
};

#endif