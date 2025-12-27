#include "Game.h"
#include "raylib.h"
#include "raymath.h"

#include <cmath>
#include <sstream>
#include <cstdlib>
#include <ctime>

static void UpdateCameraCenter(Camera2D *camera, Player *player,
    EnvItem *envItems, int envItemsLength,
    float delta, const float width, const float height)
{
    camera->offset = Vector2{ width / 2.0f, height / 2.0f };
    camera->target = player->position;
}

static void UpdateCameraCenterInsideMap(Camera2D *camera, Player *player,
    EnvItem *envItems, int envItemsLength,
    float delta, const float width, const float height)
{
    camera->target = player->position;
    camera->offset = Vector2{ width / 2.0f, height / 2.0f };

    float minX = 3000, minY = 2000, maxX = -3000, maxY = -2000;

    for (int i = 0; i < envItemsLength; i++)
    {
        const EnvItem *ei = envItems + i;

        minX = fminf(ei->rect.x, minX);
        maxX = fmaxf(ei->rect.x + ei->rect.width, maxX);
        minY = fminf(ei->rect.y, minY);
        maxY = fmaxf(ei->rect.y + ei->rect.height, maxY);
    }

    const Vector2 maxWorld{ maxX, maxY };
    const Vector2 minWorld{ minX, minY };

    const auto [x, y] = GetWorldToScreen2D(maxWorld, *camera);
    const auto [a, b] = GetWorldToScreen2D(minWorld, *camera);

    if (x < width)
    {
        camera->offset.x = width - (x - width / 2);
    }
    if (y < height)
    {
        camera->offset.y = height - (y - height / 2);
    }
    if (a > 0)
    {
        camera->offset.x = width / 2 - a;
    }
    if (b > 0)
    {
        camera->offset.y = height / 2 - b;
    }
}

static void UpdateCameraCenterSmoothFollow(Camera2D *camera, Player *player,
    EnvItem *envItems, int envItemsLength,
    const float delta, const float width, const float height)
{
    static float minSpeed = 30;
    static float minEffectLength = 10;
    static float fractionSpeed = 0.8f;

    camera->offset = Vector2{ width / 2.0f, height / 2.0f };
    const Vector2 diff = Vector2Subtract(player->position, camera->target);

    if (const float length = Vector2Length(diff); length > minEffectLength)
    {
        const float speed = fmaxf(fractionSpeed*length, minSpeed);
        camera->target = Vector2Add(camera->target, Vector2Scale(diff, speed*delta/length));
    }
}

static void UpdateCameraEvenOutOnLanding(Camera2D *camera, Player *player,
    EnvItem *envItems, int envItemsLength,
    const float delta, const float width, const float height)
{
    static float evenOutSpeed = 700;
    static int eveningOut = false;
    static float evenOutTarget;

    camera->offset = Vector2{ width/2.0f, height/2.0f };
    camera->target.x = player->position.x;

    if (eveningOut)
    {
        if (evenOutTarget > camera->target.y)
        {
            camera->target.y += evenOutSpeed*delta;

            if (camera->target.y > evenOutTarget)
            {
                camera->target.y = evenOutTarget;
                eveningOut = 0;
            }
        }
        else
        {
            camera->target.y -= evenOutSpeed*delta;

            if (camera->target.y < evenOutTarget)
            {
                camera->target.y = evenOutTarget;
                eveningOut = 0;
            }
        }
    }
    else
    {
        if (player->canJump && player->speed == 0 && player->position.y != camera->target.y)
        {
            eveningOut = 1;
            evenOutTarget = player->position.y;
        }
    }
}

static void UpdateCameraPlayerBoundsPush(Camera2D *camera, Player *player,
    EnvItem *envItems, int envItemsLength,
    float delta, const float width, const float height)
{
    static Vector2 bbox = { 0.2f, 0.2f };

    const Vector2 topLeftScreen = { (1 - bbox.x) * 0.5f * width, (1 - bbox.y) * 0.5f * height };
    const Vector2 bottomRightScreen = { (1 + bbox.x) * 0.5f * width, (1 + bbox.y) * 0.5f * height };

    const auto [x, y] = GetScreenToWorld2D(topLeftScreen, *camera);
    const auto [a, b] = GetScreenToWorld2D(bottomRightScreen, *camera);

    camera->offset = topLeftScreen;

    const float x_1 = x; const float y_1 = y;
    const float x_2 = a; const float y_2 = b;

    if (player->position.x < x_1)
    {
        camera->target.x = player->position.x;
    }
    if (player->position.y < y_1)
    {
        camera->target.y = player->position.y;
    }
    if (player->position.x > x_2)
    {
        camera->target.x = x_1 + (player->position.x - x_2);
    }
    if (player->position.y > y_2)
    {
        camera->target.y = y_1 + (player->position.y - y_2);
    }
}

Game::Game(const int screenWidth, const int screenHeight)
    : screenWidth(screenWidth), screenHeight(screenHeight)
{
    InitScene();

    cameraUpdaters = {
        UpdateCameraCenter,
        UpdateCameraCenterInsideMap,
        UpdateCameraCenterSmoothFollow,
        UpdateCameraEvenOutOnLanding,
        UpdateCameraPlayerBoundsPush
    };

    std::srand(static_cast<unsigned>(std::time(nullptr)));
    clientId = static_cast<uint32_t>(std::rand());

    if (netClient.connectTo("127.0.0.1", 1234))
    {
        netClient.setReceiveCallback([this](const std::vector<uint8_t>& data){
            const std::string s(data.begin(), data.end());
            std::istringstream iss(s);
            std::string cmd;
            iss >> cmd;

            if (cmd == "POS")
            {
                uint32_t id = 0; float x = 0, y = 0;
                iss >> id >> x >> y;

                if (id != clientId)
                {
                    remotePlayers[id] = Vector2{ x, y };
                }
            }
        });
    }
}

Game::~Game() = default;

void Game::InitScene()
{
    player = Player();
    player.position = Vector2{ 100, 500 };
    player.speed = 0;
    player.canJump = false;

    envItems = {
        EnvItem{ { 0,    0,   2000,   10  },     1, GRAY },
        EnvItem{ { 0,    0,   10,     700 },     1, GRAY },
        EnvItem{ { 1990, 0,   10,     700 },     1, GRAY },
        EnvItem{ { 0,    690, 2000,   10  },     1, GRAY },

        EnvItem{ { 0,   300, 400, 50 } ,1, GRAY },
        EnvItem{ { 350, 500, 400, 50 } ,1, GRAY },
        EnvItem{ { 600, 130, 250, 50 } ,1, GRAY },

        EnvItem{ { 1150, 500, 400, 50 } ,1, GRAY },
        EnvItem{ { 1600, 300, 400, 50 } ,1, GRAY },
        EnvItem{ { 1050, 160, 300, 50 } ,1, GRAY },

        EnvItem{ { 500,  640, 200, 50 }, 1,  GRAY },
        EnvItem{ { 1200, 640, 200, 50 }, 1,  GRAY },

        EnvItem{ { 850, 10, 200, 350 }, 1, GRAY},
    };

    camera = {};
    camera.target = player.position;
    camera.offset = Vector2{ static_cast<float>(screenWidth) / 2.0f, static_cast<float>(screenHeight) / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
}

void Game::Update(const float delta)
{
    player.Update(delta, envItems);

    sendTimer += delta;
    if (sendTimer >= SEND_PERIOD)
    {
        sendTimer = 0.0f;

        char buf[128];
        if (const int n = snprintf(buf, sizeof(buf), "POS %u %.2f %.2f", player.position.x, player.position.y); n > 0)
        {
            const std::vector<uint8_t> v(buf, buf + n);
            netClient.send(v);
        }
    }

    camera.zoom += GetMouseWheelMove() * 0.05f;

    if (camera.zoom > 2.0f)
    {
        camera.zoom = 2.0f;
    }
    else if (camera.zoom < 0.7f)
    {
        camera.zoom = 0.7f;
    }

    if (IsKeyPressed(KEY_R))
    {
        camera.zoom = 1.0f;
    }

    cameraUpdaters[2 % static_cast<int>(cameraUpdaters.size())](&camera, &player, envItems.data(), static_cast<int>(envItems.size()),
        delta, static_cast<float>(screenWidth), static_cast<float>(screenHeight));

    const Vector2 mouseScreen = GetMousePosition();
    const Vector2 mouseWorld = GetScreenToWorld2D(mouseScreen, camera);
    const Vector2 weaponAnchor = { player.position.x, player.position.y - 35.0f };
    player.weapon.Update(delta, weaponAnchor, mouseWorld, envItems, particles, aim.GetRadius());

    for (auto it = particles.begin(); it != particles.end(); )
    {
        if (!it->Update(delta)) 
        {
            it = particles.erase(it);
        }
        else ++it;
    }
}

void Game::Draw()
{
    BeginDrawing();

        ClearBackground(LIGHTGRAY);

        BeginMode2D(camera);

            for (auto &[rect, blocking, color] : envItems)
            {
                DrawRectangleRec(rect, color);
            }

            const Rectangle playerRect = { player.position.x - 10, player.position.y - 60, 20.0f, 60.0f };
            DrawRectangleRec(playerRect, RED);

            for (const auto &[fst, snd] : remotePlayers)
            {
                const auto &[x, y] = snd;
                const Rectangle r = { x - 10, y - 60, 20.0f, 60.0f };
                DrawRectangleRec(r, BLUE);
            }
            player.weapon.Draw();

            for (auto &p : particles)
            {
                p.Draw();
            }

        EndMode2D();

        const Vector2 mouseScreen2 = GetMousePosition();
        const Vector2 mouseWorld2 = GetScreenToWorld2D(mouseScreen2, camera);
        aim.Update(player.position, mouseWorld2, camera);


        if (player.weapon.IsCooling())
        {
            aim.SetColor(ORANGE);
        }
        else
        {
            aim.SetColor(RED);
        }
        aim.Draw();

        DrawText("Controls:", 20, 20, 10, BLACK);
        DrawText("- Right/Left to move", 40, 40, 10, DARKGRAY);
        DrawText("- Space to jump", 40, 60, 10, DARKGRAY);
        DrawText("- Mouse Wheel to Zoom in-out, R to reset zoom", 40, 80, 10, DARKGRAY);

    EndDrawing();
}
