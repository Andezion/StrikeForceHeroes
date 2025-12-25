#pragma once
#include "raylib.h"
#include <vector>

struct EnvItem;

#include "Bullet.h"

class Weapon
{
public:
    explicit Weapon(float length = 50.0f, float thickness = 6.0f, float bulletSpeed = 900.0f, float cooldown = 1.0f);

    void Update(float delta, const Vector2 &anchorPos, const Vector2 &targetPos,
        const std::vector<EnvItem> &envItems, float spreadRadius = 0.0f);
    void Draw() const;

private:
    Vector2 anchor;
    float length;
    float thickness;
    float rotationDegrees;

    float cooldown; 
    float cooldownTimer;
    float bulletSpeed;

    std::vector<Bullet> bullets;
};
