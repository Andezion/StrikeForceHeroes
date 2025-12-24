#include "raylib.h"
#include "Game.h"

#define MAX_TRAIL_LENGTH 30

int main()
{
    constexpr int screenWidth = 1800;
    constexpr int screenHeight = 900;

    Vector2 trailPositions[MAX_TRAIL_LENGTH] = { 0 };

    InitWindow(screenWidth, screenHeight, "StrikeForceHeroes");
    SetTargetFPS(60);

    Game game(screenWidth, screenHeight);

    while (!WindowShouldClose())
    {
        const float deltaTime = GetFrameTime();

        game.Update(deltaTime);
        game.Draw();

        Vector2 mousePosition = GetMousePosition();

        for (int i = MAX_TRAIL_LENGTH - 1; i > 0; i--)
        {
            trailPositions[i] = trailPositions[i - 1];
        }

        trailPositions[0] = mousePosition;

        for (int i = 0; i < MAX_TRAIL_LENGTH; i++)
        {
            if ((trailPositions[i].x != 0.0f) || (trailPositions[i].y != 0.0f))
            {
                float ratio = static_cast<float>((MAX_TRAIL_LENGTH - i))/MAX_TRAIL_LENGTH;


                Color trailColor = Fade(SKYBLUE, ratio*0.5f + 0.5f);

                float trailRadius = 15.0f*ratio;

                DrawCircleV(trailPositions[i], trailRadius, trailColor);
            }
        }

        DrawCircleV(mousePosition, 15.0f, WHITE);
    }

    CloseWindow();
    return 0;
}