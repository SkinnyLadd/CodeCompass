#ifndef KNOWLEDGEGRAPH_H
#define KNOWLEDGEGRAPH_H

#include "Resource.h"
#include <unordered_map>
#include <map>
#include <queue>
#include <set>
#include <algorithm>
#include <iostream>
#include <vector>
#include <iomanip> // For nice formatting
#include <utility>
#include <set>

class KnowledgeGraph {
private:
    // Key: Prereq ID, Value: List of Resources that depend on it
    std::unordered_map<int, std::vector<int>> adjList;

    // Key: Resource ID, Value: Pointer to the actual Resource object
    std::unordered_map<int, Resource*> resourceMap;

public:
    void addResource(Resource* res) {
        resourceMap[res->id] = res;
    }

    // =========================================================
    // VISUALIZE GRAPH BUILDING (STEP-BY-STEP)
    // =========================================================
    void buildGraph(const std::vector<Resource*>& allResources) {
        std::cout << "\n=== [DEBUG] LINKING GRAPH DEPENDENCIES ===" << std::endl;

        int edgesCount = 0;

        for (Resource* r : allResources) {
            for (int pre : r->prereqIDs) {
                // Check if the Prereq ID actually exists in our data
                if (resourceMap.count(pre)) {
                    // Create the Edge
                    adjList[pre].push_back(r->id);
                    edgesCount++;

                    // --- VISUALIZATION OF THE STEP ---
                    std::string pTitle = resourceMap[pre]->title;

                    std::cout << "[Step " << edgesCount << "] "
                              << std::left << std::setw(20) << pTitle
                              << " (" << pre << ")  ---> UNLOCKS --->  "
                              << r->title << " (" << r->id << ")" << std::endl;
                }
            }
        }
        std::cout << "==========================================\n" << std::endl;
    }

    // =========================================================
    // PRINT FINAL TOPOLOGY
    // =========================================================
    void printGraphState() {
        std::cout << "\n=== Knowledge Graph Topology (Final State) ===" << std::endl;
        if (adjList.empty()) {
            std::cout << "(No dependencies defined)" << std::endl;
        } else {
            for (auto const& pair : adjList) {
                int prereqID = pair.first;
                const std::vector<int>& dependents = pair.second;
                std::string pTitle = resourceMap[prereqID]->title;

                std::cout << "[" << prereqID << "] " << pTitle << " unlocks:" << std::endl;
                for (int depID : dependents) {
                    std::string dTitle = resourceMap[depID]->title;
                    std::cout << "  |-> [" << depID << "] " << dTitle << std::endl;
                }
            }
        }
        std::cout << "==============================================\n" << std::endl;
    }

    // --- Topological Sort for Curriculum ---
    std::vector<int> getCurriculum(int targetID) {
        if (resourceMap.find(targetID) == resourceMap.end()) return {};

        // 1. BFS Backwards (Subgraph Selection)
        std::set<int> nodes;
        std::queue<int> q;
        q.push(targetID);
        nodes.insert(targetID);

        while(!q.empty()){
            int curr = q.front(); q.pop();
            Resource* r = resourceMap[curr];
            if(r) {
                for(int pre : r->prereqIDs) {
                    if(nodes.find(pre) == nodes.end()) {
                        nodes.insert(pre);
                        q.push(pre);
                    }
                }
            }
        }

        // 2. Build Subgraph & Sort
        std::map<int, int> inDegree;
        std::map<int, std::vector<int>> localAdj;
        for(int n : nodes) inDegree[n] = 0;

        for(int u : nodes) {
            Resource* r = resourceMap[u];
            for(int pre : r->prereqIDs) {
                if(nodes.count(pre)) {
                    localAdj[pre].push_back(u);
                    inDegree[u]++;
                }
            }
        }

        std::queue<int> zeroQ;
        for(int n : nodes) if(inDegree[n] == 0) zeroQ.push(n);

        std::vector<int> result;
        while(!zeroQ.empty()){
            int u = zeroQ.front(); zeroQ.pop();
            result.push_back(u);
            for(int v : localAdj[u]) {
                inDegree[v]--;
                if(inDegree[v] == 0) zeroQ.push(v);
            }
        }
        return result;
    }

    // Export structure for visualization
    std::pair<std::vector<std::string>, std::vector<std::string>> getStructure() {
        std::vector<std::string> edges;
        std::vector<std::string> nodes;
        
        for (auto& pair : adjList) {
            int from = pair.first;
            nodes.push_back(std::to_string(from));
            for (int to : pair.second) {
                edges.push_back(std::to_string(from) + "->" + std::to_string(to));
                nodes.push_back(std::to_string(to));
            }
        }
        
        // Remove duplicate nodes
        std::set<int> uniqueNodes;
        for (const auto& nodeStr : nodes) {
            uniqueNodes.insert(std::stoi(nodeStr));
        }
        nodes.clear();
        for (int id : uniqueNodes) {
            nodes.push_back(std::to_string(id));
        }
        
        return std::make_pair(edges, nodes);
    }

    int getNodeCount() {
        return resourceMap.size();
    }

    int getEdgeCount() {
        int count = 0;
        for (auto& pair : adjList) {
            count += pair.second.size();
        }
        return count;
    }
};

#endif