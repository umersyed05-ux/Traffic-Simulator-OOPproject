#pragma once
#include "VehicleCore.h"

inline void Vehicle::Draw(Texture2D tBike, Texture2D tSedan, Texture2D tRickshaw) {
    Texture2D activeTex = tBike;
    if (type == SEDAN) {
        activeTex = tSedan;
    } else if (type == RICKSHAW) {
        activeTex = tRickshaw;
    }

    for (auto& p : exhaust) {
        p.life -= GetFrameTime() * 1.5f;
        if (p.life > 0) {
            DrawCircleV(p.pos, 4.0f * p.life, Fade(GRAY, p.life * 0.6f));
        }
    }

    exhaust.erase(remove_if(exhaust.begin(), exhaust.end(), [](const ExhaustParticle& p){
        return p.life <= 0;
    }), exhaust.end());

    float baseScale;
    if (type == SEDAN) {
        baseScale = 38.0f / activeTex.width;
    } else if (type == RICKSHAW) {
        baseScale = 32.0f / activeTex.width;
    } else {
        baseScale = 28.0f / activeTex.width;
    }

    float finalScale = isPlayer ? baseScale * 1.25f : baseScale * 0.8f;

    if (isPlayer) {
        DrawCircleLines((int)position.x, (int)position.y, 45.0f, GREEN);
        DrawCircleLines((int)position.x, (int)position.y, 40.0f, Fade(LIME, 0.5f));
        DrawText("YOU", (int)(position.x - 15), (int)(position.y - 55), 16, LIME);
    }

    if (activeTex.width > 0) {
        Rectangle src = { 0.0f, 0.0f, (float)activeTex.width, (float)activeTex.height };
        Rectangle dest = { position.x, position.y, activeTex.width * finalScale, activeTex.height * finalScale };
        Vector2 origin = { dest.width / 2.0f, dest.height / 2.0f };
        DrawTexturePro(activeTex, src, dest, origin, currentCarAngle, WHITE);

        if (!state["arrived"] && !state["at_station"]) {
            DrawRectanglePro({position.x, position.y, 8, 8}, {4, 4}, currentCarAngle, ORANGE);
        }
    } else {
        DrawRectanglePro({position.x, position.y, 30, 15}, {15, 7.5f}, currentCarAngle, GRAY);
        DrawText("?", (int)position.x - 5, (int)position.y - 5, 14, RED);
    }

    DrawRectangle((int)position.x - 15, (int)position.y - 30, 30, 4, RED);
    DrawRectangle((int)position.x - 15, (int)position.y - 30, (max(0.0f, fuel) / 100.0f) * 30.0f, 4, GREEN);

    if (state["is_trapped"]) {
        DrawText("TRAPPED", (int)(position.x - 25), (int)(position.y - 45), 10, RED);
    }

    if (isPushing) {
        DrawText("pushing vehicle OUT OF FUEL", (int)(position.x - 60), (int)(position.y - 45), 10, RED);
    }

    if (patienceTimer > 1.5f && !state["arrived"]) {
        DrawText("*HONK!*", (int)(position.x + 15), (int)(position.y - 20), 14, ORANGE);
    }

    if (!isPlayer && (state["arrived"] || state["is_trapped"]) && waitTimer > 0.0f) {
        DrawText(TextFormat("%.1fs", tripTimer), (int)(position.x - 15), (int)(position.y - 45), 12, GREEN);
    }
}