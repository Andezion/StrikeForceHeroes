#include "raylib.h"
#include "Game.h"

int main()
{
    constexpr int screenWidth = 1800;
    constexpr int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "StrikeForceHeroes");
    HideCursor();
    SetTargetFPS(60);

    Game game(screenWidth, screenHeight);

    while (!WindowShouldClose())
    {
        const float deltaTime = GetFrameTime();

        (void)GetMousePosition();

        game.Update(deltaTime);
        game.Draw();
    }

    ShowCursor();
    CloseWindow();
    return 0;
}