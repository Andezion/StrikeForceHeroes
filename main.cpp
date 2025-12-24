#include "raylib.h"
#include "Game.h"

#define MAX_TRAIL_LENGTH 30

int main()
{
    constexpr int screenWidth = 1800;
    constexpr int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "StrikeForceHeroes");
    SetTargetFPS(60);

    Game game(screenWidth, screenHeight);

    while (!WindowShouldClose())
    {
        const float deltaTime = GetFrameTime();

        const Vector2 mousePosition = GetMousePosition();
        DrawCircleV(mousePosition, 15.0f, WHITE);

        game.Update(deltaTime);
        game.Draw();
    }

    CloseWindow();
    return 0;
}