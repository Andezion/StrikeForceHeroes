#include "raylib.h"
#include "raymath.h"

#define G 400
#define PLAYER_JUMP_SPD 350.0f
#define PLAYER_HOR_SPD 200.0f


typedef struct Player {
    Vector2 position;
    float speed;
    bool canJump;
} Player;

typedef struct EnvItem {
    Rectangle rect;
    int blocking;
    Color color;
} EnvItem;

void UpdatePlayer(Player *player, const EnvItem *envItems, int envItemsLength, float delta);
void UpdateCameraCenter(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, float width, float height);
void UpdateCameraCenterInsideMap(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, float width, float height);
void UpdateCameraCenterSmoothFollow(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, float width, float height);
void UpdateCameraEvenOutOnLanding(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, float width, float height);
void UpdateCameraPlayerBoundsPush(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, float width, float height);

int main()
{
    constexpr int screenWidth = 800;
    constexpr int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - 2d camera platformer");

    Player player = {};
    player.position = (Vector2){ 400, 280 };

    player.speed = 0;
    player.canJump = false;

    EnvItem envItems[] = {
        {{ 0, 0, 1000, 400 }, 0, LIGHTGRAY },
        {{ 0, 400, 1000, 200 }, 1, GRAY },
        {{ 300, 200, 400, 10 }, 1, GRAY },
        {{ 250, 300, 100, 10 }, 1, GRAY },
        {{ 650, 300, 100, 10 }, 1, GRAY }
    };

    constexpr int envItemsLength = sizeof(envItems) / sizeof(envItems[0]);

    Camera2D camera = {};
    camera.target = player.position;
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    void (*cameraUpdaters[])(Camera2D*, Player*, EnvItem*, int, float, float, float) = {
        UpdateCameraCenter,
        UpdateCameraCenterInsideMap,
        UpdateCameraCenterSmoothFollow,
        UpdateCameraEvenOutOnLanding,
        UpdateCameraPlayerBoundsPush
    };

    int cameraOption = 0;
    constexpr int cameraUpdatersLength = sizeof(cameraUpdaters) / sizeof(cameraUpdaters[0]);

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        const float deltaTime = GetFrameTime();

        UpdatePlayer(&player, envItems, envItemsLength, deltaTime);

        camera.zoom += GetMouseWheelMove() * 0.05f;

        if (camera.zoom > 3.0f)
        {
            camera.zoom = 3.0f;
        }
        else if (camera.zoom < 0.25f)
        {
            camera.zoom = 0.25f;
        }

        if (IsKeyPressed(KEY_R))
        {
            camera.zoom = 1.0f;
            player.position = (Vector2) { 400, 280 };
        }

        if (IsKeyPressed(KEY_C)) cameraOption = (cameraOption + 1)%cameraUpdatersLength;

        cameraUpdaters[cameraOption](&camera, &player, envItems, envItemsLength, deltaTime, screenWidth, screenHeight);

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

    CloseWindow();

    return 0;
}

void UpdatePlayer(Player *player, const EnvItem *envItems, const int envItemsLength, const float delta)
{
    if (IsKeyDown(KEY_LEFT))
    {
        player->position.x -= PLAYER_HOR_SPD * delta;
    }
    if (IsKeyDown(KEY_RIGHT))
    {
        player->position.x += PLAYER_HOR_SPD * delta;
    }
    if (IsKeyDown(KEY_SPACE) && player->canJump)
    {
        player->speed = -PLAYER_JUMP_SPD;
        player->canJump = false;
    }

    bool hitObstacle = false;
    for (int i = 0; i < envItemsLength; i++)
    {
        const EnvItem *ei = envItems + i;
        if (Vector2 *p = &player->position;
            ei->blocking &&
            ei->rect.x <= p->x &&
            ei->rect.x + ei->rect.width >= p->x &&
            ei->rect.y >= p->y &&
            ei->rect.y <= p->y + player->speed*delta)
        {
            hitObstacle = true;
            player->speed = 0.0f;
            p->y = ei->rect.y;
            break;
        }
    }

    if (!hitObstacle)
    {
        player->position.y += player->speed * delta;
        player->speed += G * delta;
        player->canJump = false;
    }
    else player->canJump = true;
}

void UpdateCameraCenter(Camera2D *camera, Player *player,
    EnvItem *envItems, int envItemsLength,
    float delta, const float width, const float height)
{
    camera->offset = (Vector2){ width / 2.0f, height / 2.0f };
    camera->target = player->position;
}

void UpdateCameraCenterInsideMap(Camera2D *camera, Player *player,
    EnvItem *envItems, int envItemsLength,
    float delta, const float width, const float height)
{
    camera->target = player->position;
    camera->offset = (Vector2) { width / 2.0f, height / 2.0f };

    float minX = 1000, minY = 1000, maxX = -1000, maxY = -1000;

    for (int i = 0; i < envItemsLength; i++)
    {
        const EnvItem *ei = envItems + i;

        minX = fminf(ei->rect.x, minX);
        maxX = fmaxf(ei->rect.x + ei->rect.width, maxX);
        minY = fminf(ei->rect.y, minY);
        maxY = fmaxf(ei->rect.y + ei->rect.height, maxY);
    }

    auto [x, y] = GetWorldToScreen2D((Vector2){ maxX, maxY }, *camera);
    auto [x1, y1] = GetWorldToScreen2D((Vector2){ minX, minY }, *camera);

    if (x < width)
    {
        camera->offset.x = width - (x - width / 2);
    }
    if (y < height)
    {
        camera->offset.y = height - (y - height / 2);
    }
    if (x1 > 0)
    {
        camera->offset.x = width / 2 - x1;
    }
    if (y1 > 0)
    {
        camera->offset.y = height / 2 - y1;
    }
}

void UpdateCameraCenterSmoothFollow(Camera2D *camera, Player *player,
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

void UpdateCameraEvenOutOnLanding(Camera2D *camera, Player *player,
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

void UpdateCameraPlayerBoundsPush(Camera2D *camera, Player *player,
    EnvItem *envItems, int envItemsLength,
    float delta, const float width, const float height)
{
    static Vector2 bbox = { 0.2f, 0.2f };

    auto [x_1, y_1] = GetScreenToWorld2D((Vector2){ (1 - bbox.x) * 0.5f * width, (1 - bbox.y) * 0.5f * height }, *camera);
    auto [x_2, y_2] = GetScreenToWorld2D((Vector2){ (1 + bbox.x) * 0.5f * width, (1 + bbox.y) * 0.5f * height }, *camera);

    camera->offset = (Vector2){ (1 - bbox.x) * 0.5f * width, (1 - bbox.y) * 0.5f * height };

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