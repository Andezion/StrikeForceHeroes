#pragma once

#include "raylib.h"
#include <vector>

struct EnvItem {
    Rectangle rect;
    int blocking;
    Color color;
};

#include "Player.h"
#include "Aim.h"

class Game
{
public:
    Game(int screenWidth, int screenHeight);
    ~Game();

    void Update(float delta);
    void Draw();

private:
    int screenWidth;
    int screenHeight;

    Player player{};
    std::vector<EnvItem> envItems;
    Camera2D camera{};

    Aim aim{ Aim::Type::Default };

    using CameraUpdater = void(*)(Camera2D*, Player*, EnvItem*, int, float, float, float);
    std::vector<CameraUpdater> cameraUpdaters;
    int cameraOption = 0;

    void InitScene();
};
