#pragma once
#include "Common.h"

inline void DrawButton(Rectangle btn, const char* text, Vector2 mousePos, Color baseColor, Color hoverColor) {
    bool hover = CheckCollisionPointRec(mousePos, btn);

    if (hover) {
        DrawRectangleRounded(btn, 0.2f, 10, hoverColor);
        DrawRectangleRoundedLines(btn, 0.2f, 10, WHITE);
    } else {
        DrawRectangleRounded(btn, 0.2f, 10, baseColor);
        DrawRectangleRoundedLines(btn, 0.2f, 10, LIGHTGRAY);
    }

    int textW = MeasureText(text, 22);
    DrawText(text, btn.x + (btn.width - textW) / 2, btn.y + (btn.height - 22) / 2, 22, WHITE);
}