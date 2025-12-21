#include "raylib.h"
#include "Game.h"

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

        game.Update(deltaTime);
        game.Draw();
    }

    CloseWindow();
    return 0;
}