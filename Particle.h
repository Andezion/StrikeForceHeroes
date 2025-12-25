#pragma once

#include "raylib.h"

class Particle
{
public:
    Particle() = default;
    Particle(const Vector2 &p, const Vector2 &v, float life, float r, Color c);

    bool Update(float delta);
    void Draw() const;

    [[nodiscard]] bool IsAlive() const
    {
        return life > 0.0f;
    }

private:
    Vector2 pos{};
    Vector2 vel{};
    float life = 0.0f;
    float radius = 2.0f;
    Color color = GRAY;
};
