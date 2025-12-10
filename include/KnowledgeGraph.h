#ifndef KNOWLEDGEGRAPH_H
#define KNOWLEDGEGRAPH_H

#include "Resource.h"
#include "MaxHeap.h"
#include <unordered_map>
#include <queue>
#include <limits>
#include <algorithm>
#include <iostream>
#include <vector>

class KnowledgeGraph {
private:
    // Key: Prereq ID, Value: List of Resources that depend on it
    std::unordered_map<int, std::vector<Resource*>> adjList;
    // Fast lookup for all resources by ID
    std::unordered_map<int, Resource*> resourceMap;

public:
    void addResource(Resource* res) {
        resourceMap[res->id] = res;
    }

    void addDependency(int prereqID, int dependentID) {
        // Check if the dependent resource exists before adding the edge
        if (resourceMap.count(dependentID)) {
            adjList[prereqID].push_back(resourceMap[dependentID]);
        }
    }

    // BFS: Find all reachable topics (Text-based Visualization)
    std::vector<int> bfs(int startID) {
        std::vector<int> traversalOrder;

        if (resourceMap.find(startID) == resourceMap.end()) {
            std::cerr << "ERROR: BFS start node ID " << startID << " not found." << std::endl;
            return traversalOrder;
        }

        std::queue<int> q;
        std::unordered_map<int, bool> visited;

        q.push(startID);
        visited[startID] = true; // Mark the starting node as visited

        std::cout << "\n--- BFS Traversal Visualization (Start: " << resourceMap[startID]->title << " [" << startID << "]) ---" << std::endl;

        int step = 1;
        while(!q.empty()) {
            int size = q.size();
            std::cout << "Step " << step++ << " (Queue Size: " << size << "): ";

            // Process all nodes at the current level
            for (int i = 0; i < size; ++i) {
                int currID = q.front();
                q.pop();
                traversalOrder.push_back(currID);

                Resource* currRes = resourceMap[currID];
                std::cout << currRes->title << " [" << currID << "] ";

                // Check neighbors
                if (adjList.count(currID)) {
                    for (Resource* neighbor : adjList[currID]) {
                        // Check if the neighbor has been visited. This check is crucial for breaking cycles.
                        if (!visited[neighbor->id]) {
                            visited[neighbor->id] = true;
                            q.push(neighbor->id);
                        }
                    }
                }
            }
            std::cout << std::endl;
        }
        std::cout << "-------------------------------------------------------------------------" << std::endl;
        return traversalOrder;
    }

    // Helper function to print the full adjacency list state
    void printGraphState() const {
        std::cout << "\n--- KnowledgeGraph Full Adjacency List State ---" << std::endl;
        std::cout << "(Total Resources: " << resourceMap.size() << ", Edges starting from a node: " << adjList.size() << ")" << std::endl;

        for (const auto& pair : adjList) {
            int prereqID = pair.first;
            const std::vector<Resource*>& dependents = pair.second;

            // Get the title for the prerequisite node
            std::string prereqTitle = resourceMap.count(prereqID) ? resourceMap.at(prereqID)->title : "Unknown Resource";

            std::cout << "  [" << prereqID << "] " << prereqTitle << " -> DEPENDENTS:" << std::endl;
            for (Resource* dependent : dependents) {
                std::cout << "    - [" << dependent->id << "] " << dependent->title << std::endl;
            }
        }
        std::cout << "--------------------------------------------------------" << std::endl;
    }

    // Placeholder for Dijkstra/Max-Rating Path
    void findBestPath(int startID, int targetID) {
        std::cout << "Pathfinding from " << startID << " to " << targetID << " (Dijkstra not yet implemented)." << std::endl;
    }
};

#endif