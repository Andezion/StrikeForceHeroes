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
#include "Particle.h"
#include "NetworkClient.h"

#include <unordered_map>
#include <cstdint>

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

    std::vector<Particle> particles;

    using CameraUpdater = void(*)(Camera2D*, Player*, EnvItem*, int, float, float, float);
    std::vector<CameraUpdater> cameraUpdaters;
    int cameraOption = 0;

    void InitScene();

    NetworkClient netClient;
    uint32_t clientId = 0;
    std::unordered_map<uint32_t, Vector2> remotePlayers;

    float sendTimer = 0.0f;
    static inline constexpr float SEND_PERIOD = 0.1f;
};
