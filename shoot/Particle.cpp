#include "Particle.h"
#include "raylib.h"
#include "raymath.h"

Particle::Particle(const Vector2 &p, const Vector2 &v, const float life, const float r, const Color c)
    : pos(p), vel(v), life(life), radius(r), color(c) {}

bool Particle::Update(const float delta)
{
    if (life <= 0.0f)
    {
        return false;
    }

    life -= delta;

    vel = Vector2Scale(vel, 0.98f);
    vel.y += 80.0f * delta;
    pos = Vector2Add(pos, Vector2Scale(vel, delta));

    return life > 0.0f;
}

void Particle::Draw() const
{
    if (life <= 0.0f)
    {
        return;
    }

    const float alpha = fmaxf(0.0f, life / 1.0f);
    DrawCircleV(pos, radius, Fade(color, alpha));
}
