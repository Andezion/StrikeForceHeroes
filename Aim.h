#pragma once

#include "raylib.h"

class Aim
{
public:
    enum class Type { Default = 0 };

    Aim(Type type = Type::Default);

    void Update(const Vector2& playerWorld, const Vector2& cursorWorld, const Camera2D& camera);
    void Draw() const;

private:
    Type type;
    Vector2 cursorScreen{};
    float radius = 10.0f;
    float innerRadius = 6.0f;
    Color color = RED;

    static float RemapClamped(float value, float inMin, float inMax, float outMin, float outMax);
};
