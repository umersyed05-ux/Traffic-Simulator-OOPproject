#pragma once
#include "Common.h"

inline vector<int> CalculatePath(int startNode, int targetNode, const vector<Waypoint>& graph, const vector<bool>& memory) {
    cout << "[DEBUG] Calculating Dijkstra path from Node " << startNode << " to Node " << targetNode << endl;
    
    vector<float> dist(graph.size(), 999999.0f);
    vector<int> parent(graph.size(), -1);
    using pdi = pair<float, int>;
    priority_queue<pdi, vector<pdi>, greater<pdi>> pq;
    
    dist[startNode] = 0; 
    pq.push({0, startNode});

    while (!pq.empty()) {
        int curr = pq.top().second; 
        float d = pq.top().first; 
        pq.pop();
        
        if (curr == targetNode) {
            break;
        }
        
        if (d > dist[curr]) {
            continue;
        }

        for (int neighbor : graph[curr].connections) {
            if (memory[neighbor]) {
                continue;
            }
            
            float weight = sqrt(pow(graph[curr].pos.x - graph[neighbor].pos.x, 2) + pow(graph[curr].pos.y - graph[neighbor].pos.y, 2));
            
            if (dist[curr] + weight < dist[neighbor]) {
                dist[neighbor] = dist[curr] + weight;
                parent[neighbor] = curr;
                pq.push({dist[neighbor], neighbor});
            }
        }
    }
    
    if (parent[targetNode] == -1 && startNode != targetNode) {
        cout << "[WARNING] No valid route found! Reroute needed." << endl;
        return vector<int>();
    }
    
    vector<int> path; 
    for (int at = targetNode; at != -1; at = parent[at]) {
        path.push_back(at);
    }
    
    vector<int> finalPath; 
    for (int i = path.size() - 1; i >= 0; i--) {
        finalPath.push_back(path[i]);
    }
    
    return finalPath;
}