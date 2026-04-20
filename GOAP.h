#pragma once
#include "Common.h"

// Forward declaration so GOAP knows what a vehicle is.
class Vehicle;

struct GoapAction {
    string name;
    int cost;
    unordered_map<string, bool> preconditions;
    unordered_map<string, bool> effects;
    function<bool(Vehicle*, const vector<Waypoint>&, const vector<Vehicle>&, const vector<Obstacle>&, bool)> executeFunc;

    bool IsAchievable(const unordered_map<string, bool>& state) {
        for (auto const& [k, v] : preconditions) {
            auto it = state.find(k);
            if (it == state.end() || it->second != v) {
                return false;
            }
        }
        return true;
    }
};

struct GoapNode {
    unordered_map<string, bool> state;
    vector<shared_ptr<GoapAction>> plan;
    int cost;
    
    bool operator>(const GoapNode& other) const { 
        return cost > other.cost; 
    }
};

inline vector<shared_ptr<GoapAction>> BuildPlan(unordered_map<string, bool> startState, unordered_map<string, bool> goalState, const vector<shared_ptr<GoapAction>>& actions) {
    cout << "[GOAP] Constructing new action plan..." << endl;
    priority_queue<GoapNode, vector<GoapNode>, greater<GoapNode>> pq;
    pq.push({startState, {}, 0});
    int safety = 0;

    while (!pq.empty() && safety < 1000) {
        safety++; 
        GoapNode curr = pq.top(); 
        pq.pop();
        
        bool match = true;
        for (auto const& [k, v] : goalState) { 
            if (curr.state[k] != v) { 
                match = false; 
                break; 
            } 
        }
        
        if (match) {
            cout << "[GOAP] Plan successfully built with cost: " << curr.cost << endl;
            return curr.plan;
        }

        for (auto action : actions) {
            if (action->IsAchievable(curr.state)) {
                if (curr.plan.size() > 5) {
                    continue;
                }
                
                GoapNode nextNode = curr;
                nextNode.plan.push_back(action); 
                nextNode.cost += action->cost;
                
                for (auto const& [k, v] : action->effects) {
                    nextNode.state[k] = v;
                }
                
                pq.push(nextNode);
            }
        }
    }
    
    cout << "[GOAP ERROR] Failed to find a valid action sequence!" << endl;
    return {};
}
