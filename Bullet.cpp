#include "Bullet.h"
#include "raylib.h"
#include "raymath.h"
#include "Game.h"
#include "Particle.h"

#include <cmath>
#include <ctime>
#include <random>

static float RandomFloat(const float a, const float b)
{
    static std::mt19937 rng(static_cast<unsigned>(time(nullptr)));
    std::uniform_real_distribution dist(a, b);
    return dist(rng);
}

static Vector2 RandomPointInCircle(const Vector2 &center, const float r)
{
    const float t = RandomFloat(0.0f, 2.0f * PI);
    const float u = RandomFloat(0.0f, 1.0f);

    const float rad = r * sqrtf(u);
    return Vector2{ center.x + cosf(t)*rad, center.y + sinf(t)*rad };
}

Bullet::Bullet(const Vector2 &startPos, const Vector2 &aimTarget, const float speed, const float spreadRadius)
    : pos(startPos), speed(speed)
{
    Vector2 target = aimTarget;
    if (spreadRadius > 0.0001f)
    {
        target = RandomPointInCircle(aimTarget, spreadRadius);
    }

    const Vector2 dir = Vector2Subtract(target, startPos);
    if (const float len = Vector2Length(dir); len > 0.0001f)
    {
        vel = Vector2Scale(dir, 1.0f / len * speed);
    }
    else
    {
        vel = { speed, 0 };
    }
}

bool Bullet::Update(const float delta, const std::vector<EnvItem> &envItems, std::vector<Particle> &outParticles)
{
    if (!active)
    {
        return false;
    }

    pos = Vector2Add(pos, Vector2Scale(vel, delta));

    for (const auto &[rect, blocking, color] : envItems)
    {
        if (!blocking)
        {
            continue;
        }

        if (CheckCollisionCircleRec(pos, radius, rect))
        {
            constexpr int count = 10;
            for (int i = 0; i < count; ++i)
            {
                const float ang = RandomFloat(0.0f, 2.0f * PI);
                const float spd = RandomFloat(40.0f, 240.0f);

                const Vector2 v = { cosf(ang) * spd, sinf(ang) * spd };
                outParticles.emplace_back(pos, v, RandomFloat(0.3f, 0.9f), RandomFloat(1.0f, 3.0f), DARKGRAY);
            }
            active = false;
            return false;
        }
    }

    if (pos.x < -5000 || pos.x > 5000 || pos.y < -5000 || pos.y > 5000)
    {
        active = false;
        return false;
    }

    return true;
}

void Bullet::Draw() const
{
    if (!active)
    {
        return;
    }

    constexpr float len = 12.0f;
    constexpr float halfLen = len * 0.5f;
    const float thickness = radius * 2.0f;
    const float rot = atan2f(vel.y, vel.x) * 180.0f / PI;

    const Rectangle rec = { pos.x - halfLen, pos.y - thickness * 0.5f, len, thickness };
    const Vector2 origin = { halfLen, thickness * 0.5f };
    DrawRectanglePro(rec, origin, rot, DARKGRAY);
}
