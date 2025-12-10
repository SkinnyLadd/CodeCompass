#ifndef KNOWLEDGEGRAPH_H
#define KNOWLEDGEGRAPH_H

#include "Resource.h"
#include "AVLTree.h" // We need AVLTree to look up Resource details
#include <unordered_map>
#include <map>
#include <queue>
#include <set>
#include <algorithm>
#include <iostream>
#include <vector>

class KnowledgeGraph {
private:
    // Key: Prereq ID, Value: List of Resources that depend on it (Adjacency List)
    std::unordered_map<int, std::vector<int>> adjList;

    // Key: Resource ID, Value: Pointer to the actual Resource object
    std::unordered_map<int, Resource*> resourceMap;

public:
    // 1. Build the Graph Nodes
    void addResource(Resource* res) {
        resourceMap[res->id] = res;
    }

    // 2. Build the Graph Edges (Dependencies)
    // Called by Engine.cpp after loading CSV
    void buildGraph(const std::vector<Resource*>& allResources) {
        for (Resource* r : allResources) {
            for (int prereqID : r->prereqIDs) {
                // If Prereq exists, add edge: Prereq -> Current Resource
                if (resourceMap.find(prereqID) != resourceMap.end()) {
                    adjList[prereqID].push_back(r->id);
                }
            }
        }
    }

    // --- NEW LOGIC: Generate a Linear Lesson Plan (Topological Sort) ---
    std::vector<int> getCurriculum(int targetID) {
        std::vector<int> plan;
        std::set<int> visited;
        std::vector<int> subgraphNodes;

        // Step A: BFS/DFS to find ONLY the relevant ancestors
        // (We don't want to sort the whole database, only what is needed for targetID)
        std::queue<int> q;
        q.push(targetID);
        visited.insert(targetID);
        subgraphNodes.push_back(targetID);

        // Reverse-traverse to find all prerequisites
        // (This is a bit tricky with a standard AdjList, so we cheat by looking at Resource objects directly)
        int head = 0;
        while(head < subgraphNodes.size()) {
            int currID = subgraphNodes[head++];
            Resource* r = resourceMap[currID];

            if (r) {
                for (int preID : r->prereqIDs) {
                    if (visited.find(preID) == visited.end()) {
                        visited.insert(preID);
                        subgraphNodes.push_back(preID);
                    }
                }
            }
        }

        // Step B: Build a Mini-Graph for just these nodes
        std::map<int, int> inDegree;
        std::map<int, std::vector<int>> localAdj;

        for (int node : subgraphNodes) inDegree[node] = 0;

        for (int u : subgraphNodes) {
            Resource* r = resourceMap[u];
            if (!r) continue;
            for (int pre : r->prereqIDs) {
                // Only include edges that exist within our subgraph
                if (visited.count(pre)) {
                    localAdj[pre].push_back(u);
                    inDegree[u]++;
                }
            }
        }

        // Step C: Kahn's Algorithm (Topological Sort)
        std::queue<int> zeroQ;
        for (int node : subgraphNodes) {
            if (inDegree[node] == 0) zeroQ.push(node);
        }

        while (!zeroQ.empty()) {
            int u = zeroQ.front();
            zeroQ.pop();
            plan.push_back(u);

            for (int v : localAdj[u]) {
                inDegree[v]--;
                if (inDegree[v] == 0) zeroQ.push(v);
            }
        }

        // Validation: If cycle detected, return empty
        if (plan.size() != subgraphNodes.size()) {
            std::cerr << "Error: Cyclic dependency found in curriculum." << std::endl;
            return {};
        }

        return plan;
    }
};

#endif