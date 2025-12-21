#include "Game.h"
#include "raylib.h"
#include "raymath.h"

#include <cmath>

#define G 400
#define PLAYER_JUMP_SPD 350.0f
#define PLAYER_HOR_SPD 200.0f

static void UpdateCameraCenter(Camera2D *camera, Player *player,
    EnvItem *envItems, int envItemsLength,
    float delta, const float width, const float height)
{
    camera->offset = (Vector2){ width / 2.0f, height / 2.0f };
    camera->target = player->position;
}

static void UpdateCameraCenterInsideMap(Camera2D *camera, Player *player,
    EnvItem *envItems, int envItemsLength,
    float delta, const float width, const float height)
{
    camera->target = player->position;
    camera->offset = (Vector2) { width / 2.0f, height / 2.0f };

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

    camera->offset = (Vector2){ width / 2.0f, height / 2.0f };
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

    camera->offset = (Vector2){ width/2.0f, height/2.0f };
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
}

Game::~Game() = default;

void Game::InitScene()
{
    player = {};
    player.position = (Vector2){ 20, 20 };
    player.speed = 0;
    player.canJump = false;

    envItems = {
        EnvItem{ { 0, 0, 2000, 10 }, 1, GRAY },
        EnvItem{ { 0, 0, 10, 500 }, 1, GRAY },
        EnvItem{ { 1990, 0, 10, 500 }, 1, GRAY },
        EnvItem{ { 0, 490, 2000, 10 }, 1, GRAY },

        EnvItem{ { 300, 200, 400, 10 }, 1, GRAY },
        EnvItem{ { 250, 300, 100, 10 }, 1, GRAY },
        EnvItem{ { 650, 300, 100, 10 }, 1, GRAY }
    };

    camera = {};
    camera.target = player.position;
    camera.offset = (Vector2){ static_cast<float>(screenWidth) / 2.0f, static_cast<float>(screenHeight) / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
}

void Game::UpdatePlayer(const float delta)
{
    if (IsKeyDown(KEY_A))
    {
        player.position.x -= PLAYER_HOR_SPD * delta;
    }
    if (IsKeyDown(KEY_D))
    {
        player.position.x += PLAYER_HOR_SPD * delta;
    }
    if (IsKeyDown(KEY_W) && player.canJump)
    {
        player.speed = -PLAYER_JUMP_SPD;
        player.canJump = false;
    }

    bool collided = false;
    const int envItemsLength = static_cast<int>(envItems.size());

    Player *p = &player;
    const float nextY = p->position.y + p->speed * delta;
    const float playerLeft = p->position.x;
    const float playerRight = p->position.x;
    const float playerTop = p->position.y;
    const float playerBottom = p->position.y;
    const float nextBottom = playerBottom + p->speed * delta;
    const float nextTop = playerTop + p->speed * delta;

    for (int i = 0; i < envItemsLength; ++i)
    {
        const EnvItem *ei = envItems.data() + i;
        if (!ei->blocking)
        {
            continue;
        }

        const float eiLeft = ei->rect.x;
        const float eiRight = ei->rect.x + ei->rect.width;
        const float eiTop = ei->rect.y;
        const float eiBottom = ei->rect.y + ei->rect.height;

        const bool horizontallyOverlap = (playerRight > eiLeft) && (playerLeft < eiRight);

        if (p->speed > 0)
        {
            if (horizontallyOverlap && playerBottom <= eiTop && nextBottom >= eiTop)
            {
                collided = true;
                p->speed = 0.0f;
                p->position.y = eiTop;
                break;
            }
        }
        else if (p->speed < 0)
        {
            if (horizontallyOverlap && playerTop >= eiBottom && nextTop <= eiBottom)
            {
                collided = true;
                p->speed = 0.0f;
                p->position.y = eiBottom;
                break;
            }
        }
    }

    if (!collided)
    {
        p->position.y = nextY;
        p->speed += G * delta;
        p->canJump = false;
    }
    else
    {
        p->canJump = true;
    }
}

void Game::Update(float delta)
{
    UpdatePlayer(delta);

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

    if (IsKeyPressed(KEY_C))
    {
        cameraOption = (cameraOption + 1) % static_cast<int>(cameraUpdaters.size());
    }

    cameraUpdaters[cameraOption](&camera, &player, envItems.data(), static_cast<int>(envItems.size()),
        delta, static_cast<float>(screenWidth), static_cast<float>(screenHeight));
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

            const Rectangle playerRect = { player.position.x - 20, player.position.y - 40, 40.0f, 40.0f };
            DrawRectangleRec(playerRect, RED);

            DrawCircleV(player.position, 5.0f, GOLD);

        EndMode2D();

        DrawText("Controls:", 20, 20, 10, BLACK);
        DrawText("- Right/Left to move", 40, 40, 10, DARKGRAY);
        DrawText("- Space to jump", 40, 60, 10, DARKGRAY);
        DrawText("- Mouse Wheel to Zoom in-out, R to reset zoom", 40, 80, 10, DARKGRAY);
        DrawText("- C to change camera mode", 40, 100, 10, DARKGRAY);
        DrawText("Current camera mode:", 20, 120, 10, BLACK);

    EndDrawing();
}
