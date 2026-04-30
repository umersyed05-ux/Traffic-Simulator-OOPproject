#pragma once
#include "VehicleCore.h"

inline void Vehicle::AssignNewRandomDestination(const vector<Waypoint>& graph) {
    int targetDest = GetRandomValue(0, graph.size() - 1);
    int attempts = 0;
    
    while ((targetDest == destinationNode || graph[targetDest].isBlocked || targetDest == 14) && attempts < 50) {
        targetDest = GetRandomValue(0, graph.size() - 1);
        attempts++;
    }

    destinationNode = targetDest; 
    currentTargetNode = targetDest;
    currentPath = CalculatePath(physicalTarget, destinationNode, graph, knownBlockedNodes);

    if (currentPath.size() > 1) {
        physicalStart = currentPath[0];
        physicalTarget = currentPath[1];
        currentWaypointIndex = 2;
        edgeProgress = 0.0f;

        state["arrived"] = false; 
        state["is_trapped"] = false; 
        state["path_blocked"] = false;
        tripTimer = 0.0f;

        if (fuel < 40.0f && GetRandomValue(0, 10) > 3) {
            fuel = (float)GetRandomValue(60, 100);
        }
        
        isPushing = false;
        currentPlan.clear();
        cout << "[INFO] Dispatch assigned new destination!" << endl;
    } else {
        state["is_trapped"] = true;
        state["arrived"] = false;
    }
}

inline bool Vehicle::DriveAlongPath(const vector<Waypoint>& graph, const vector<Vehicle>& allTraffic, const vector<Obstacle>& obstacles, bool horizontalGreen) {
    if (currentPath.empty() || currentWaypointIndex > currentPath.size()) {
        return true;
    }

    bool discovered = false;
    for (int i = 0; i < graph.size(); i++) {
        if (graph[i].isBlocked && !knownBlockedNodes[i]) {
            if (sqrt(pow(position.x - graph[i].pos.x, 2) + pow(position.y - graph[i].pos.y, 2)) < 60.0f) {
                knownBlockedNodes[i] = true; 
                discovered = true;
                cout << "[LOG] Driver discovered an active blockade!" << endl;
            }
        }
    }
    
    if (discovered) {
        if (knownBlockedNodes[physicalTarget]) { 
            state["path_blocked"] = true; 
            return false; 
        }
        for (int i = currentWaypointIndex; i < currentPath.size(); i++) {
            if (knownBlockedNodes[currentPath[i]]) { 
                state["path_blocked"] = true; 
                return false; 
            }
        }
    }

    Vector2 startPos = graph[physicalStart].pos; 
    Vector2 targetPos = graph[physicalTarget].pos;
    
    float dx = targetPos.x - startPos.x; 
    float dy = targetPos.y - startPos.y;
    float edgeDist = sqrt(dx * dx + dy * dy);
    
    if (edgeDist == 0.0f) { 
        currentWaypointIndex++; 
        edgeProgress = 0.0f; 
        return false; 
    }

    float rotationOffset = 0.0f;
    if (type == SEDAN) {
        rotationOffset = 90.0f;
    } else if (type == BIKE) {
        rotationOffset = 90.0f;
    } else if (type == RICKSHAW) {
        rotationOffset = 180.0f;
    }

    currentCarAngle = atan2(dy, dx) * (180.0f / PI) + rotationOffset;

    float speed = (type == BIKE) ? 120.0f : (type == SEDAN) ? 100.0f : 80.0f;
    if (isPushing) {
        speed = 15.0f;
    }

    float mult = 1.0f;
    bool stoppedForRedLight = false;
    int trafficJamCount = 0;

    for (const auto& obs : obstacles) {
        if (obs.type == POTHOLE && sqrt(pow(position.x - obs.position.x, 2) + pow(position.y - obs.position.y, 2)) < obs.radius + 15.0f) {
            speed *= 0.3f;
        }
    }

    for (const auto& other : allTraffic) {
        if (&other == this || other.state.at("arrived")) {
            continue;
        }

        float distToOther = sqrt(pow(position.x - other.position.x, 2) + pow(position.y - other.position.y, 2));
        if (distToOther < 50.0f) {
            trafficJamCount++;
        }

        if (physicalStart == other.physicalStart && physicalTarget == other.physicalTarget) {
            if (other.edgeProgress - edgeProgress > 0.05f && other.edgeProgress - edgeProgress < 0.20f) {
                mult = 0.0f;
            }
        }
        
        if (distToOther < 40.0f && mult > 0.0f) {
            mult = 0.4f;
        }
    }

    if (graph[physicalTarget].hasTrafficLight) {
        bool isHoriz = abs(graph[physicalStart].pos.y - graph[physicalTarget].pos.y) < 20.0f;
        if (((isHoriz && !horizontalGreen) || (!isHoriz && horizontalGreen)) && edgeProgress > 0.80f) {
            if (edgeProgress < 0.95f) { 
                edgeProgress = 0.80f; 
                mult = 0.0f; 
                stoppedForRedLight = true; 
            }
        }
    }

    if (mult == 0.0f && !stoppedForRedLight) {
        patienceTimer += GetFrameTime();
        if (patienceTimer > 1.5f && !isPushing) {
            mult = 0.2f;
        }
    } else {
        patienceTimer = 0.0f;
    }

    if (isPlayer && trafficJamCount >= 2 && patienceTimer > 2.5f) {
        vector<bool> tempBlocks = knownBlockedNodes;
        tempBlocks[physicalTarget] = true;
        vector<int> altPath = CalculatePath(physicalStart, destinationNode, graph, tempBlocks);

        if (!altPath.empty() && altPath.size() > 1) {
            statusMsg = "Traffic Jam! GPS Rerouting...";
            cout << "[LOG] Heavy traffic detected. Finding alternate route." << endl;
            
            int oldTarget = physicalTarget;
            physicalTarget = physicalStart;
            physicalStart = oldTarget;
            edgeProgress = 1.0f - edgeProgress;

            altPath.insert(altPath.begin(), oldTarget);
            currentPath = altPath;
            currentWaypointIndex = 2;
            patienceTimer = 0.0f;
            return false;
        }
    }

    speed *= mult;
    if (speed < 10.0f && mult > 0.0f) {
        speed = 10.0f;
    }

    if (speed > 0.0f && !isPushing && GetRandomValue(0, 100) < 15) {
        exhaust.push_back({position, 1.0f});
    }

    edgeProgress += (speed * GetFrameTime()) / edgeDist;
    
    if (edgeProgress >= 1.0f) {
        position = targetPos; 
        physicalStart = physicalTarget;
        
        if (currentWaypointIndex < currentPath.size()) {
            physicalTarget = currentPath[currentWaypointIndex];
            currentWaypointIndex++; 
            edgeProgress = 0.0f;
        } else {
            return true;
        }
    } else {
        position.x = startPos.x + (dx * edgeProgress); 
        position.y = startPos.y + (dy * edgeProgress);
    }
    
    return false;
}