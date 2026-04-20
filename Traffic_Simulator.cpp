#include "Common.h"
#include "UIHelpers.h"
#include "Pathfinding.h"
#include "GOAP.h"
#include "VehicleCore.h"
#include "VehicleActions.h"
#include "VehicleDrive.h"
#include "VehicleDesign.h"

int main() {
    cout << "--- Starting City Traffic Engine ---" << endl;

    const int screenWidth = 1024;
    const int screenHeight = 768;
    InitWindow(screenWidth, screenHeight, "City Traffic Tracker - Endless City");
    SetTargetFPS(60);

    GameState currentState = MENU_VEHICLE;
    VehicleType playerVehicleType = BIKE;
    int playerStartNode = 21;

    Texture2D bgMap = LoadTexture("map.png");
    Texture2D tBike = LoadTexture("bike.png");
    Texture2D tSedan = LoadTexture("sedan.png");
    Texture2D tRickshaw = LoadTexture("rikshaw.png");

    vector<Vehicle> traffic;
    vector<Obstacle> activeObstacles;

    float npcSpawnTimer = 0.0f;
    float trafficLightTimer = 0.0f;

    vector<Waypoint> graph = {
        { {510.0f, 180.0f}, {6, 1, 3, 10}, false, false },  { {510.0f, 381.0f}, {0, 2, 4, 11}, false, true },
        { {510.0f, 566.0f}, {1, 7, 5, 13}, false, false },  { {235.0f, 180.0f}, {8, 4, 0}, false, false },
        { {235.0f, 381.0f}, {3, 5, 1, 12}, false, false },  { {235.0f, 566.0f}, {4, 9, 2, 14}, false, true },
        { {510.0f, 99.0f},  {0}, false, false },            { {510.0f, 665.0f}, {2}, false, false },
        { {235.0f, 99.0f},  {3}, false, false },            { {235.0f, 665.0f}, {5}, false, false },
        { {784.0f, 180.0f}, {0}, false, false },            { {902.0f, 381.0f}, {1}, false, false },
        { {26.0f,  381.0f}, {4, 18, 19}, false, false },    { {901.0f, 566.0f}, {2}, false, false },
        { {150.0f, 566.0f}, {5, 22}, false, false },        { {203.0f, 96.0f},  {16}, false, false },
        { {39.0f,  96.0f},  {15, 17}, false, false },       { {35.0f,  290.0f}, {16, 18}, false, false },
        { {25.0f,  306.0f}, {17, 12}, false, false },       { {24.0f,  419.0f}, {12, 20}, false, false },
        { {29.0f,  648.0f}, {19, 21}, false, false },       { {75.0f,  647.0f}, {20, 22}, false, false },
        { {98.0f,  622.0f}, {21, 14}, false, false }
    };

    int btnW = 400;
    int btnH = 65;
    float cX = (screenWidth - btnW) / 2.0f;

    Rectangle btnBike     = { cX, 250, (float)btnW, (float)btnH };
    Rectangle btnSedan    = { cX, 340, (float)btnW, (float)btnH };
    Rectangle btnRickshaw = { cX, 430, (float)btnW, (float)btnH };

    Rectangle dest1 = { cX, 220, (float)btnW, (float)btnH };
    Rectangle dest2 = { cX, 310, (float)btnW, (float)btnH };
    Rectangle dest3 = { cX, 400, (float)btnW, (float)btnH };
    Rectangle dest4 = { cX, 490, (float)btnW, (float)btnH };
    Rectangle dest5 = { cX, 580, (float)btnW, (float)btnH };

    int finalScoreMoney = 0;
    int finalScoreJobs = 0;

    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();

        if (currentState == MENU_VEHICLE) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (CheckCollisionPointRec(mousePos, btnBike)) {
                    playerVehicleType = BIKE;
                    currentState = MENU_DESTINATION;
                }
                if (CheckCollisionPointRec(mousePos, btnSedan)) {
                    playerVehicleType = SEDAN;
                    currentState = MENU_DESTINATION;
                }
                if (CheckCollisionPointRec(mousePos, btnRickshaw)) {
                    playerVehicleType = RICKSHAW;
                    currentState = MENU_DESTINATION;
                }
            }
        }
        else if (currentState == MENU_DESTINATION) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                int chosenDest = -1;

                if (CheckCollisionPointRec(mousePos, dest1)) chosenDest = 0;
                if (CheckCollisionPointRec(mousePos, dest2)) chosenDest = 8;
                if (CheckCollisionPointRec(mousePos, dest3)) chosenDest = 10;
                if (CheckCollisionPointRec(mousePos, dest4)) chosenDest = 22;
                if (CheckCollisionPointRec(mousePos, dest5)) chosenDest = 7;

                if (chosenDest != -1) {
                    activeObstacles.clear();
                    traffic.clear();

                    for (auto& n : graph) {
                        n.isBlocked = false;
                    }

                    int barricadeNodes[] = {19, 7};
                    for (int n : barricadeNodes) {
                        graph[n].isBlocked = true;
                        Obstacle obs = { graph[n].pos, 50.0f, PROTEST };

                        for (int i = 0; i < 2; i++) {
                            float ang = (float)GetRandomValue(0, 360) * PI / 180.0f;
                            float dist = (float)GetRandomValue(0, 20);

                            BlockadeCar bCar;
                            bCar.pos.x = obs.position.x + cosf(ang) * dist;
                            bCar.pos.y = obs.position.y + sinf(ang) * dist;
                            bCar.rotation = (float)GetRandomValue(0, 360);
                            bCar.type = (i == 0) ? SEDAN : RICKSHAW;

                            obs.blockadeCars.push_back(bCar);
                        }
                        activeObstacles.push_back(obs);
                    }

                    activeObstacles.push_back({ {510.0f, 470.0f}, 20.0f, POTHOLE });
                    activeObstacles.push_back({ {370.0f, 180.0f}, 20.0f, POTHOLE });
                    activeObstacles.push_back({ {150.0f, 566.0f}, 20.0f, POTHOLE });

                    traffic.push_back(Vehicle(playerStartNode, chosenDest, playerVehicleType, true, graph));
                    currentState = GAMEPLAY;
                }
            }
        }
        else if (currentState == GAMEPLAY) {
            if (IsKeyPressed(KEY_BACKSPACE)) {
                currentState = MENU_VEHICLE;
            }

            trafficLightTimer += GetFrameTime();
            if (trafficLightTimer >= 10.0f) {
                trafficLightTimer = 0.0f;
            }

            bool horizontalIsGreen = (trafficLightTimer < 5.0f);

            npcSpawnTimer += GetFrameTime();
            if (npcSpawnTimer > 1.0f && traffic.size() < 15) {
                npcSpawnTimer = 0.0f;
                int edgeNodes[] = {6, 8, 9, 10, 11, 13, 15, 21, 22};
                int startI = edgeNodes[GetRandomValue(0, 8)];
                int endI = GetRandomValue(0, graph.size() - 1);

                while (endI == startI || graph[endI].isBlocked || endI == traffic[0].destinationNode || endI == 14) {
                    endI = GetRandomValue(0, graph.size() - 1);
                }

                VehicleType randomType = (VehicleType)GetRandomValue(0, 2);
                traffic.push_back(Vehicle(startI, endI, randomType, false, graph));
            }

            Vehicle* pVeh = nullptr;

            for (auto it = traffic.begin(); it != traffic.end(); ) {
                if (it->isPlayer) {
                    pVeh = &(*it);
                }

                it->Update(graph, traffic, activeObstacles, horizontalIsGreen);

                if (it->readyToDespawn && !it->isPlayer) {
                    it = traffic.erase(it);
                } else {
                    ++it;
                }
            }

            if (pVeh && (pVeh->jobsCompleted >= 10 || pVeh->money >= 100)) {
                finalScoreMoney = pVeh->money;
                finalScoreJobs = pVeh->jobsCompleted;
                currentState = GAME_OVER;
            }
        }
        else if (currentState == GAME_OVER) {
            if (IsKeyPressed(KEY_ENTER)) {
                currentState = MENU_VEHICLE;
            }
            if (IsKeyPressed(KEY_ESCAPE)) {
                break;
            }
        }

        // ====================================================
        // DRAW PHASE
        // ====================================================
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (currentState == MENU_VEHICLE) {
            DrawRectangleGradientV(0, 0, screenWidth, screenHeight, {20, 25, 35, 255}, {10, 10, 15, 255});
            int titleSize = 50;
            DrawText("CITY TRAFFIC: SELECT VEHICLE", (screenWidth - MeasureText("CITY TRAFFIC: SELECT VEHICLE", titleSize)) / 2, 100, titleSize, LIME);

            DrawButton(btnBike, "CD70 Bike (Fast, Low Payout)", mousePos, Fade(DARKGREEN, 0.7f), LIME);
            DrawButton(btnSedan, "Sedan (Balanced)", mousePos, Fade(DARKGREEN, 0.7f), LIME);
            DrawButton(btnRickshaw, "Rickshaw (Slow, Steady)", mousePos, Fade(DARKGREEN, 0.7f), LIME);
        }
        else if (currentState == MENU_DESTINATION) {
            DrawRectangleGradientV(0, 0, screenWidth, screenHeight, {20, 25, 35, 255}, {10, 10, 15, 255});
            int titleSize = 50;
            DrawText("SELECT FIRST DISPATCH", (screenWidth - MeasureText("SELECT FIRST DISPATCH", titleSize)) / 2, 80, titleSize, SKYBLUE);

            DrawButton(dest1, "North East Hub", mousePos, Fade(DARKBLUE, 0.7f), SKYBLUE);
            DrawButton(dest2, "North West Hub", mousePos, Fade(DARKBLUE, 0.7f), SKYBLUE);
            DrawButton(dest3, "Restaurant Block", mousePos, Fade(DARKBLUE, 0.7f), SKYBLUE);
            DrawButton(dest4, "Industrial Zone", mousePos, Fade(DARKBLUE, 0.7f), SKYBLUE);
            DrawButton(dest5, "South East Suburbs", mousePos, Fade(DARKBLUE, 0.7f), SKYBLUE);
        }
        else if (currentState == GAMEPLAY) {
            if (bgMap.width > 0) {
                DrawTextureEx(bgMap, { 0, (float)(screenHeight - bgMap.height * ((float)screenWidth / bgMap.width)) / 2.0f }, 0.0f, (float)screenWidth / bgMap.width, WHITE);
            }

            bool horizontalIsGreen = (trafficLightTimer < 5.0f);

            DrawRectangle((int)graph[0].pos.x - 45, (int)graph[0].pos.y - 32, 120, 20, Fade(WHITE, 0.7f));
            DrawText("North East Hub", (int)graph[0].pos.x - 40, (int)graph[0].pos.y - 30, 14, BLACK);

            DrawRectangle((int)graph[8].pos.x - 45, (int)graph[8].pos.y - 32, 120, 20, Fade(WHITE, 0.7f));
            DrawText("North West Hub", (int)graph[8].pos.x - 40, (int)graph[8].pos.y - 30, 14, BLACK);

            DrawRectangle((int)graph[10].pos.x - 55, (int)graph[10].pos.y - 32, 135, 20, Fade(WHITE, 0.7f));
            DrawText("Restaurant Block", (int)graph[10].pos.x - 50, (int)graph[10].pos.y - 30, 14, BLACK);

            DrawRectangle((int)graph[14].pos.x - 50, (int)graph[14].pos.y + 28, 135, 20, Fade(WHITE, 0.7f));
            DrawText("Gas Station Area", (int)graph[14].pos.x - 45, (int)graph[14].pos.y + 30, 14, BLACK);

            DrawRectangle((int)graph[22].pos.x - 50, (int)graph[22].pos.y + 28, 120, 20, Fade(WHITE, 0.7f));
            DrawText("Industrial Zone", (int)graph[22].pos.x - 45, (int)graph[22].pos.y + 30, 14, BLACK);

            DrawRectangle((int)graph[7].pos.x - 60, (int)graph[7].pos.y + 28, 150, 20, Fade(WHITE, 0.7f));
            DrawText("South East Suburbs", (int)graph[7].pos.x - 55, (int)graph[7].pos.y + 30, 14, BLACK);

            for (const auto& obs : activeObstacles) {
                if (obs.type == POTHOLE) {
                    DrawCircleV(obs.position, obs.radius, DARKBROWN);
                    DrawCircleLines((int)obs.position.x, (int)obs.position.y, obs.radius, BLACK);
                    DrawText("HOLE", (int)(obs.position.x - 15), (int)(obs.position.y - 5), 10, WHITE);
                } else if (obs.type == PROTEST) {
                    DrawCircleV(obs.position, obs.radius, Fade(RED, 0.5f));
                    DrawCircleLines((int)obs.position.x, (int)obs.position.y, obs.radius, MAROON);

                    for(const auto& bCar : obs.blockadeCars) {
                        Texture2D tex = (bCar.type == SEDAN) ? tSedan : tRickshaw;
                        if (tex.width > 0) {
                            float scale = 40.0f / tex.width;
                            Rectangle src = { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
                            Rectangle dest = { bCar.pos.x, bCar.pos.y, tex.width * scale, tex.height * scale };
                            DrawTexturePro(tex, src, dest, { dest.width / 2.0f, dest.height / 2.0f }, bCar.rotation, GRAY);
                        }
                    }

                    DrawRectangle((int)(obs.position.x - 45), (int)(obs.position.y - 37), 90, 18, Fade(BLACK, 0.6f));
                    DrawText("ROAD CLOSED", (int)(obs.position.x - 40), (int)(obs.position.y - 35), 12, WHITE);
                }
            }

            Vehicle* pVeh = nullptr;
            for (auto& v : traffic) {
                if (v.isPlayer) {
                    pVeh = &v;
                }
            }

            if (pVeh && !pVeh->state["arrived"] && !pVeh->state["is_trapped"] && pVeh->currentWaypointIndex <= pVeh->currentPath.size()) {
                Vector2 start = pVeh->position;
                for (size_t i = pVeh->currentWaypointIndex - 1; i < pVeh->currentPath.size(); i++) {
                    Vector2 end = graph[pVeh->currentPath[i]].pos;
                    DrawLineEx(start, end, 8.0f, Fade(LIME, 0.8f));
                    start = end;
                }
            }

            for (int i = 0; i < graph.size(); i++) {
                for (int neighbor : graph[i].connections) {
                    DrawLineEx(graph[i].pos, graph[neighbor].pos, 2.0f, Fade(LIGHTGRAY, 0.3f));
                }

                if (graph[i].hasTrafficLight) {
                    Vector2 p = graph[i].pos;
                    DrawRectangle((int)p.x - 15, (int)p.y - 15, 30, 30, Fade(BLACK, 0.7f));

                    Color hColor = horizontalIsGreen ? GREEN : RED;
                    Color vColor = horizontalIsGreen ? RED : GREEN;

                    DrawCircle((int)p.x - 15, (int)p.y, 5.0f, hColor);
                    DrawCircle((int)p.x + 15, (int)p.y, 5.0f, hColor);
                    DrawCircle((int)p.x, (int)p.y - 15, 5.0f, vColor);
                    DrawCircle((int)p.x, (int)p.y + 15, 5.0f, vColor);
                }
            }

            DrawCircle(150, 566, 30, Fade(BLUE, 0.3f));

            if (pVeh && !pVeh->state["is_trapped"]) {
                Vector2 destPos = graph[pVeh->destinationNode].pos;
                DrawCircle((int)destPos.x, (int)destPos.y, 25.0f, Fade(GREEN, 0.6f));
                DrawCircleLines((int)destPos.x, (int)destPos.y, 30.0f, DARKGREEN);
                DrawText("YOUR DEST", (int)(destPos.x - 30), (int)(destPos.y - 45), 14, DARKGREEN);
            }

            for (auto& car : traffic) {
                car.Draw(tBike, tSedan, tRickshaw);
            }

            DrawRectangle(0, 0, screenWidth, 80, Fade(BLACK, 0.85f));

            string statusText = "N/A";
            string planText = "Idle";
            int pMoney = 0, pJobs = 0;
            float pFuel = 0.0f, pTime = 0.0f;

            if (pVeh) {
                statusText = pVeh->statusMsg;
                pMoney = pVeh->money;
                pJobs = pVeh->jobsCompleted;
                pFuel = max(0.0f, pVeh->fuel);
                pTime = pVeh->tripTimer;

                planText = "";
                for (const auto& action : pVeh->currentPlan) {
                    planText += "[" + action->name + "] -> ";
                }

                if (planText.empty() && pVeh->state["arrived"]) {
                    planText = "[Waiting for dispatch]";
                }
            }

            DrawText(TextFormat("DRIVER STATUS: %s | Fuel: %.0f | Trip Time: %.1fs", statusText.c_str(), pFuel, pTime), 20, 15, 18, LIME);
            DrawText(TextFormat("GOAP QUEUE: %s", planText.c_str()), 20, 45, 14, SKYBLUE);
            DrawText(TextFormat("BANK: $%i", pMoney), 850, 15, 24, GREEN);
            DrawText(TextFormat("Jobs Completed: %i", pJobs), 820, 45, 16, YELLOW);
        }
        else if (currentState == GAME_OVER) {
            DrawRectangleGradientV(0, 0, screenWidth, screenHeight, {20, 40, 20, 255}, {10, 10, 15, 255});
            int titleSize = 60;
            DrawText("SHIFT COMPLETE!", (screenWidth - MeasureText("SHIFT COMPLETE!", titleSize)) / 2, 200, titleSize, LIME);

            const char* stat1 = TextFormat("Total Earnings: $%i", finalScoreMoney);
            const char* stat2 = TextFormat("Jobs Completed: %i", finalScoreJobs);

            DrawText(stat1, (screenWidth - MeasureText(stat1, 30)) / 2, 350, 30, GREEN);
            DrawText(stat2, (screenWidth - MeasureText(stat2, 30)) / 2, 400, 30, YELLOW);

            DrawText("Press [ENTER] to play again", (screenWidth - MeasureText("Press [ENTER] to play again", 20)) / 2, 550, 20, LIGHTGRAY);
            DrawText("Press [ESC] to quit", (screenWidth - MeasureText("Press [ESC] to quit", 20)) / 2, 600, 20, LIGHTGRAY);
        }

        EndDrawing();
    }

    UnloadTexture(bgMap);
    UnloadTexture(tBike);
    UnloadTexture(tSedan);
    UnloadTexture(tRickshaw);
    CloseWindow();

    cout << "--- Engine Shutdown successfully ---" << endl;
    return 0;
}