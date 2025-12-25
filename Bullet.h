#pragma once

#include "raylib.h"
#include <vector>

struct EnvItem;
class Particle;

class Bullet
{
public:
    Bullet(const Vector2 &startPos, const Vector2 &aimTarget, float speed = 900.0f, float spreadRadius = 0.0f);

    bool Update(float delta, const std::vector<EnvItem> &envItems, std::vector<Particle> &outParticles);
    void Draw() const;

    [[nodiscard]] bool IsActive() const { return active; }

private:
    Vector2 pos{};
    Vector2 vel{};
    float speed{};
    float radius = 4.0f;
    bool active = true;
    Color color = DARKGRAY;
};
