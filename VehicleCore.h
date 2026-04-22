#pragma once
#include "Common.h"
#include "Pathfinding.h"
#include "GOAP.h"

class Vehicle {
public:
    bool isPlayer;
    VehicleType type;
    Vector2 position;
    vector<int> currentPath;

    int spawnNode;
    int destinationNode;
    int currentTargetNode;

    int physicalStart;
    int physicalTarget;
    int currentWaypointIndex;

    float currentCarAngle;
    float edgeProgress;
    string statusMsg;

    float fuel;
    bool isPushing;
    float tripTimer;
    float waitTimer;
    float patienceTimer;

    int jobsCompleted;
    int money;
    bool readyToDespawn;
    vector<ExhaustParticle> exhaust;

    unordered_map<string, bool> state;
    unordered_map<string, bool> goal;
    vector<shared_ptr<GoapAction>> availableActions;
    vector<shared_ptr<GoapAction>> currentPlan;
    vector<bool> knownBlockedNodes;

    // Constructor declaration
    Vehicle(int startNode, int endNode, VehicleType vType, bool playerStatus, const vector<Waypoint>& graph);

    // Method declarations
    void InitializeActions();
    void AssignNewRandomDestination(const vector<Waypoint>& graph);
    bool DriveAlongPath(const vector<Waypoint>& graph, const vector<Vehicle>& allTraffic, const vector<Obstacle>& obstacles, bool horizontalGreen);
    void Update(const vector<Waypoint>& graph, const vector<Vehicle>& allTraffic, const vector<Obstacle>& obstacles, bool horizontalGreen);
    void Draw(Texture2D tBike, Texture2D tSedan, Texture2D tRickshaw);
};

// Constructor implementation
inline Vehicle::Vehicle(int startNode, int endNode, VehicleType vType, bool playerStatus, const vector<Waypoint>& graph) {
    isPlayer = playerStatus;
    type = vType;
    spawnNode = startNode;
    destinationNode = endNode;
    currentTargetNode = endNode;
    edgeProgress = 0.0f;
    statusMsg = "Initializing...";

    fuel = (float)GetRandomValue(40, 100);
    isPushing = false;
    tripTimer = 0.0f;
    waitTimer = 0.0f;
    patienceTimer = 0.0f;

    jobsCompleted = 0;
    money = isPlayer ? 0 : GetRandomValue(20, 100);
    readyToDespawn = false;

    position = graph[startNode].pos;
    knownBlockedNodes = vector<bool>(graph.size(), false);
    currentPath = CalculatePath(startNode, endNode, graph, knownBlockedNodes);

    if (currentPath.size() > 1) {
        physicalStart = currentPath[0];
        physicalTarget = currentPath[1];
        currentWaypointIndex = 2;
    }

    state = { {"has_fuel", true}, {"at_station", false}, {"path_blocked", false}, {"is_trapped", false}, {"arrived", false} };
    goal = { {"arrived", true} };

    if (currentPath.empty() || currentPath.size() < 2) {
        state["is_trapped"] = true;
    }

    cout << "[TEST] Vehicle spawned at Node " << startNode << " heading to Node " << endNode << endl;
    InitializeActions();
}
// NO INCLUDES DOWN HERE ANYMORE!