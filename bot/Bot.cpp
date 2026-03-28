#include "Bot.h"
#include "Game.h"
#include "Particle.h"
#include "raylib.h"

#include <cmath>
#include <cstdlib>

Bot::Bot(const Vector2 startPos, const float difficulty, const float aggression)
    : position(startPos),
      speed(0.0f),
      canJump(false),
      health(100),
      maxHealth(100),
      difficulty(difficulty),
      aggression(aggression),
      damageMultiplier(0.5f + difficulty * 1.5f),
      maxIdleTime(3.0f - difficulty * 2.5f),
      visionRadius(200.0f + aggression * 600.0f),
      weapon(50.0f, 6.0f, 1600.0f, 1.2f - difficulty * 0.7f), 
      state(BotState::IDLE),
      idleTimer(0.0f),
      patrolTimer(0.0f),
      patrolDir(1.0f)
{}

Rectangle Bot::GetRect() const
{
    constexpr float halfWidth  = 10.0f;
    constexpr float fullHeight = 60.0f;
    return { position.x - halfWidth, position.y - fullHeight, halfWidth * 2.0f, fullHeight };
}

void Bot::Update(const float delta, const std::vector<EnvItem>& envItems,
                 const Vector2 playerPos, std::vector<Particle>& particles)
{
    if (IsDead()) return;

    constexpr float halfWidth  = 10.0f;
    constexpr float fullHeight = 60.0f;

    const float dx   = playerPos.x - position.x;
    const float dy   = playerPos.y - position.y; 
    const float dist = sqrtf(dx * dx + dy * dy);

    const bool playerVisible = (dist <= visionRadius);

    if (playerVisible)
    {
        state = (dist <= ATTACK_RANGE) ? BotState::ATTACK : BotState::CHASE;
    }
    else if (state == BotState::CHASE || state == BotState::ATTACK)
    {
        state     = BotState::IDLE;
        idleTimer = 0.0f;
    }

    float moveDir   = 0.0f;
    bool  wantsJump = false;
    bool  shouldFire = false;

    switch (state)
    {
        case BotState::IDLE:
            idleTimer += delta;
            if (idleTimer >= maxIdleTime)
            {
                state       = BotState::PATROL;
                patrolDir   = (std::rand() % 2 == 0) ? -1.0f : 1.0f;
                patrolTimer = 2.0f + (1.0f - difficulty) * 2.0f; 
                idleTimer   = 0.0f;
            }
            break;

        case BotState::PATROL:
            moveDir      = patrolDir;
            patrolTimer -= delta;
            if (patrolTimer <= 0.0f)
            {
                state     = BotState::IDLE;
                idleTimer = 0.0f;
            }
            break;

        case BotState::CHASE:
            moveDir = (dx > 0.0f) ? 1.0f : -1.0f;
            if (dy < -60.0f && canJump) wantsJump = true;
            break;

        case BotState::ATTACK:
            if (dist > 200.0f) moveDir = (dx > 0.0f) ? 0.7f : -0.7f;
            if (dy < -60.0f && canJump) wantsJump = true;
            shouldFire = true;
            break;
    }

    position.x += moveDir * HOR_SPEED * delta;

    if (wantsJump && canJump)
    {
        speed   = -JUMP_SPEED;
        canJump = false;
    }

    speed      += GRAVITY * delta;
    position.y += speed * delta;

    Rectangle botRect = {
        position.x - halfWidth,
        position.y - fullHeight,
        halfWidth * 2.0f,
        fullHeight
    };

    bool grounded = false;

    for (const auto& [rect, blocking, color] : envItems)
    {
        if (!blocking) continue;

        if (CheckCollisionRecs(rect, botRect))
        {
            const float overlapLeft   = botRect.x + botRect.width  - rect.x;
            const float overlapRight  = rect.x + rect.width  - botRect.x;
            const float overlapTop    = botRect.y + botRect.height - rect.y;
            const float overlapBottom = rect.y + rect.height - botRect.y;

            const float minOverlap = fminf(
                fminf(overlapLeft, overlapRight),
                fminf(overlapTop,  overlapBottom));

            if (minOverlap == overlapTop)
            {
                position.y = rect.y;
                speed      = 0.0f;
                grounded   = true;
            }
            else if (minOverlap == overlapBottom)
            {
                position.y = rect.y + rect.height + fullHeight;
                speed      = 0.0f;
            }
            else if (minOverlap == overlapLeft)
            {
                position.x = rect.x - halfWidth;
                if (state == BotState::PATROL) patrolDir = 1.0f; 
            }
            else
            {
                position.x = rect.x + rect.width + halfWidth;
                if (state == BotState::PATROL) patrolDir = -1.0f;
            }

            botRect.x = position.x - halfWidth;
            botRect.y = position.y - fullHeight;
        }
    }

    canJump = grounded;

    const Vector2 anchor = { position.x, position.y - 35.0f };
    weapon.Update(delta, anchor, playerPos, envItems, particles, 0.0f, shouldFire);
}

void Bot::Draw() const
{
    if (IsDead()) return;

    constexpr float halfWidth  = 10.0f;
    constexpr float fullHeight = 60.0f;

    Color bodyColor;
    switch (state)
    {
        case BotState::IDLE:   bodyColor = Color{ 40,  110, 210, 255 }; break; 
        case BotState::PATROL: bodyColor = Color{ 60,  140, 230, 255 }; break; 
        case BotState::CHASE:  bodyColor = Color{ 220, 120,  20, 255 }; break; 
        case BotState::ATTACK: bodyColor = Color{ 210,  35,  35, 255 }; break; 
        default:               bodyColor = BLUE;
    }

    const Rectangle r = { position.x - halfWidth, position.y - fullHeight,
                           halfWidth * 2.0f, fullHeight };
    DrawRectangleRec(r, bodyColor);

    const float barWidth = halfWidth * 2.0f;
    const float barX     = position.x - barWidth / 2.0f;
    const float barY     = position.y - fullHeight - 11.0f;
    DrawRectangleRec({ barX - 1.0f, barY - 1.0f, barWidth + 2.0f, 7.0f }, DARKGRAY);
    const float ratio = (maxHealth > 0) ? static_cast<float>(health) / static_cast<float>(maxHealth) : 0.0f;
    DrawRectangleRec({ barX, barY, barWidth * ratio, 5.0f }, GREEN);

    weapon.Draw();
}
