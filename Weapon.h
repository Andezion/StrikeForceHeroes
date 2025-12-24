#pragma once
#include "raylib.h"

class Weapon
{
public:
    explicit Weapon(float length = 80.0f, float thickness = 6.0f, Color color = BLACK);

    void Update(const Vector2 &anchorPos, const Vector2 &targetPos);
    void Draw() const;

private:
    Vector2 anchor;
    float length;
    float thickness;
    float rotationDegrees;
    Color color;
};
