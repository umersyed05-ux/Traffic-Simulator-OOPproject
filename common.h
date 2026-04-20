#pragma once
#include "raylib.h"
#include <vector>
#include <cmath>
#include <queue>
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <algorithm>
#include <iostream>

using namespace std;

#define PI 3.14159265358979323846f

enum GameState { MENU_VEHICLE, MENU_DESTINATION, GAMEPLAY, GAME_OVER };
enum VehicleType { BIKE, SEDAN, RICKSHAW };
enum ObstacleType { PROTEST, POTHOLE };

struct BlockadeCar {
    Vector2 pos;
    float rotation;
    VehicleType type;
};

struct Obstacle {
    Vector2 position;
    float radius;
    ObstacleType type;
    vector<BlockadeCar> blockadeCars;
};

struct Waypoint {
    Vector2 pos;
    vector<int> connections;
    bool isBlocked;
    bool hasTrafficLight;
};

struct ExhaustParticle {
    Vector2 pos;
    float life;
};