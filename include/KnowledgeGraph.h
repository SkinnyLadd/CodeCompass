#ifndef KNOWLEDGEGRAPH_H
#define KNOWLEDGEGRAPH_H

#include "Resource.h"
#include <unordered_map>
#include <map>
#include <queue>
#include <set>
#include <vector>
#include <iostream>

class KnowledgeGraph {
private:
    std::unordered_map<int, std::vector<int>> adjList;
    std::unordered_map<int, Resource*> resourceMap;

public:
    void addResource(Resource* res) { resourceMap[res->id] = res; }

    void buildGraph(const std::vector<Resource*>& allResources) {
        for (Resource* r : allResources) {
            for (int pre : r->prereqIDs) {
                if (resourceMap.count(pre)) adjList[pre].push_back(r->id);
            }
        }
    }

    // --- Topological Sort for Curriculum ---
    std::vector<int> getCurriculum(int targetID) {
        if (resourceMap.find(targetID) == resourceMap.end()) return {};

        // 1. BFS to find Ancestors
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
};
#endif