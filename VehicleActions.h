#pragma once
#include "VehicleCore.h"

inline void Vehicle::InitializeActions() {
    auto aDrive = make_shared<GoapAction>();
    aDrive->name = "Drive to Destination";
    aDrive->cost = 1;
    aDrive->preconditions = { {"has_fuel", true}, {"path_blocked", false}, {"is_trapped", false} };
    aDrive->effects = { {"arrived", true} };

    aDrive->executeFunc = [](Vehicle* v, const vector<Waypoint>& g, const vector<Vehicle>& t, const vector<Obstacle>& o, bool hg) {
        v->tripTimer += GetFrameTime();

        if (!v->isPushing) {
            v->fuel -= GetFrameTime() * 2.5f;
            if (v->fuel <= 0.0f) {
                v->fuel = 0.0f;
                v->isPushing = true;
                v->state["has_fuel"] = false;
                cout << "[ALERT] Vehicle ran out of fuel!" << endl;
                return false;
            } else if (v->fuel < 15.0f && v->currentTargetNode != 14) {
                v->state["has_fuel"] = false;
                return false;
            }
        }

        bool arrived = v->DriveAlongPath(g, t, o, hg);

        if (v->state["path_blocked"]) {
            return false;
        }

        if (arrived) {
            v->state["arrived"] = true;
            return true;
        }
        return false;
    };

    auto aDetour = make_shared<GoapAction>();
    aDetour->name = "Calculate Detour";
    aDetour->cost = 1;
    aDetour->preconditions = { {"path_blocked", true} };
    aDetour->effects = { {"path_blocked", false} };

    aDetour->executeFunc = [](Vehicle* v, const vector<Waypoint>& g, const vector<Vehicle>& t, const vector<Obstacle>& o, bool hg) {
        bool immediate = v->knownBlockedNodes[v->physicalTarget];
        int routeStart = immediate ? v->physicalStart : v->physicalTarget;
        v->currentPath = CalculatePath(routeStart, v->currentTargetNode, g, v->knownBlockedNodes);

        if (v->currentPath.empty()) {
            v->state["is_trapped"] = true;
        } else {
            if (immediate) {
                int temp = v->physicalStart;
                v->physicalStart = v->physicalTarget;
                v->physicalTarget = temp;
                v->edgeProgress = 1.0f - v->edgeProgress;
                v->currentPath.insert(v->currentPath.begin(), temp);
            } else {
                v->currentPath.insert(v->currentPath.begin(), v->physicalStart);
            }
            v->currentWaypointIndex = 2;
        }
        v->state["path_blocked"] = false;
        return true;
    };

    auto aStation = make_shared<GoapAction>();
    aStation->name = "Detour to Gas Station";
    aStation->cost = 2;
    aStation->preconditions = { {"has_fuel", false}, {"path_blocked", false} };
    aStation->effects = { {"at_station", true} };

    aStation->executeFunc = [](Vehicle* v, const vector<Waypoint>& g, const vector<Vehicle>& t, const vector<Obstacle>& o, bool hg) {
        if (v->currentTargetNode != 14) {
            v->currentTargetNode = 14;
            v->currentPath = CalculatePath(v->physicalTarget, 14, g, v->knownBlockedNodes);

            if (v->currentPath.size() > 0) {
                v->currentPath.insert(v->currentPath.begin(), v->physicalStart);
                v->currentWaypointIndex = 2;
            } else {
                v->state["is_trapped"] = true;
                return false;
            }
        }

        if (!v->isPushing) {
            v->fuel -= GetFrameTime() * 2.5f;
            if (v->fuel <= 0.0f) {
                v->fuel = 0.0f;
                v->isPushing = true;
            }
        }

        bool arrived = v->DriveAlongPath(g, t, o, hg);

        if (v->state["path_blocked"]) {
            return false;
        }

        if (arrived) {
            v->state["at_station"] = true;
            v->isPushing = false;
            return true;
        }
        return false;
    };

    auto aRefuel = make_shared<GoapAction>();
    aRefuel->name = "Refueling...";
    aRefuel->cost = 1;
    aRefuel->preconditions = { {"at_station", true} };
    aRefuel->effects = { {"has_fuel", true} };

    aRefuel->executeFunc = [](Vehicle* v, const vector<Waypoint>& g, const vector<Vehicle>& t, const vector<Obstacle>& o, bool hg) {
        v->fuel += GetFrameTime() * 40.0f;
        if (v->fuel >= 100.0f) {
            v->fuel = 100.0f;
            v->money -= 10;
            v->state["has_fuel"] = true;
            v->state["at_station"] = false;

            v->currentTargetNode = v->destinationNode;
            v->currentPath = CalculatePath(v->physicalStart, v->destinationNode, g, v->knownBlockedNodes);

            if(v->currentPath.size() > 1) {
                v->physicalTarget = v->currentPath[1];
                v->currentWaypointIndex = 2;
                v->edgeProgress = 0.0f;
            } else {
                v->state["is_trapped"] = true;
            }
            return true;
        }
        return false;
    };

    auto aStop = make_shared<GoapAction>();
    aStop->name = "Emergency Stop";
    aStop->cost = 5;
    aStop->preconditions = { {"is_trapped", true} };
    aStop->effects = { {"arrived", true} };

    aStop->executeFunc = [](Vehicle* v, const vector<Waypoint>& g, const vector<Vehicle>& t, const vector<Obstacle>& o, bool hg) {
        v->statusMsg = "CRITICAL: Trapped!";
        v->state["arrived"] = true;
        return true;
    };

    availableActions = { aDrive, aDetour, aStation, aRefuel, aStop };
}

inline void Vehicle::Update(const vector<Waypoint>& graph, const vector<Vehicle>& allTraffic, const vector<Obstacle>& obstacles, bool horizontalGreen) {
    if (state["arrived"] || state["is_trapped"]) {
        waitTimer += GetFrameTime();

        if (isPlayer) {
            if (state["is_trapped"]) {
                statusMsg = "TRAPPED! Canceling Route ($0 Payout)";
                if (waitTimer > 3.0f) {
                    waitTimer = 0.0f;
                    AssignNewRandomDestination(graph);
                }
            } else {
                statusMsg = "Job Done! Collecting fare...";
                if (waitTimer > 3.0f) {
                    int payout = (type == SEDAN) ? 25 : (type == RICKSHAW) ? 15 : 10;
                    money += payout;
                    jobsCompleted++;
                    waitTimer = 0.0f;
                    AssignNewRandomDestination(graph);
                }
            }
        } else {
            statusMsg = state["is_trapped"] ? "Trapped! Giving up." : TextFormat("Finished in %.1f sec!", tripTimer);
            if (waitTimer > 3.0f) {
                readyToDespawn = true;
            }
        }
        return;
    }

    if (currentPlan.empty()) {
        currentPlan = BuildPlan(state, goal, availableActions);
        if (currentPlan.empty()) {
            state["is_trapped"] = true;
            return;
        }
    }

    auto currentAction = currentPlan.front();
    bool actionComplete = currentAction->executeFunc(this, graph, allTraffic, obstacles, horizontalGreen);

    if (state["path_blocked"] && (currentAction->name == "Drive to Destination" || currentAction->name == "Detour to Gas Station")) {
        currentPlan.clear();
        statusMsg = "Road Blocked! Replanning...";
        cout << "[WARNING] Road blocked! Forcing replan." << endl;
    } else if (!state["has_fuel"] && currentAction->name == "Drive to Destination") {
        currentPlan.clear();
        statusMsg = "Low Fuel! Detouring to Gas Station...";
    } else if (actionComplete) {
        currentPlan.erase(currentPlan.begin());
    } else {
        statusMsg = isPushing ? "OUT OF GAS! Pushing..." : "GOAP: " + currentAction->name;
    }
}