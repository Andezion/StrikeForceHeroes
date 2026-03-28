#include "Weapon.h"
#include "raylib.h"
#include "Game.h"
#include "Particle.h"

#include <cmath>
#include <random>
#include <ctime>

#include "raymath.h"

static float RandomFloatW(const float a, const float b)
{
    static std::mt19937 rng(static_cast<unsigned>(time(nullptr)));
    std::uniform_real_distribution<float> dist(a, b);
    return dist(rng);
}

static Vector2 RandomPointInCircleW(const Vector2 &center, const float r)
{
    const float t = RandomFloatW(0.0f, 2.0f * PI);
    const float u = RandomFloatW(0.0f, 1.0f);

    const float rad = r * sqrtf(u);
    return Vector2{ center.x + cosf(t) * rad, center.y + sinf(t) * rad };
}

Weapon::Weapon(const float length, const float thickness, const float bulletSpeed, const float cooldown)
    : anchor{0.0f, 0.0f}, length(length), thickness(thickness), rotationDegrees(0.0f),
      cooldown(cooldown), cooldownTimer(0.0f), bulletSpeed(bulletSpeed), bullets() {}

void Weapon::Update(const float delta, const Vector2 &anchorPos, const Vector2 &targetPos,
    const std::vector<EnvItem> &envItems, std::vector<Particle> &outParticles, const float spreadRadius)
{
    anchor = anchorPos;

    const float dx = targetPos.x - anchor.x;
    const float dy = targetPos.y - anchor.y;

    rotationDegrees = atan2f(dy, dx) * 180.0f / PI;

    if (cooldownTimer > 0.0f)
    {
        cooldownTimer -= delta;
    }

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && cooldownTimer <= 0.0f)
    {
        const float rad = rotationDegrees * PI / 180.0f;
        const Vector2 endPos = { anchor.x + cosf(rad) * length, anchor.y + sinf(rad) * length };

        Vector2 target = targetPos;
        if (spreadRadius > 0.0001f)
        {
            target = RandomPointInCircleW(targetPos, spreadRadius);
        }

        const Vector2 dir = Vector2Subtract(target, endPos);
        const float lenDir = Vector2Length(dir);
        Vector2 vel;
        if (lenDir > 0.0001f)
        {
            vel = Vector2Scale(dir, 1.0f / lenDir * bulletSpeed);
        }
        else
        {
            vel = { bulletSpeed, 0 };
        }

        bullets.emplace_back(endPos, vel, 0.0f);
        cooldownTimer = cooldown;
    }

    for (auto it = bullets.begin(); it != bullets.end(); )
    {
        if (!it->Update(delta, envItems, outParticles))
        {
            it = bullets.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Weapon::Draw() const
{
    const Rectangle rec = { anchor.x, anchor.y - thickness * 0.5f + 3, length, thickness };
    const Vector2 origin = { 0.0f, thickness * 0.5f };

    DrawRectanglePro(rec, origin, rotationDegrees, BLACK);
    DrawCircleV(anchor, thickness * 0.6f, BLACK);

    for (const auto &b : bullets)
    {
        b.Draw();
    }
}

    [[nodiscard]] bool Weapon::IsCooling() const
    {
        return cooldownTimer > 0.0f;
    }
