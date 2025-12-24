#include "Weapon.h"
#include "raylib.h"

#include <cmath>

Weapon::Weapon(const float length, const float thickness)
    : anchor{0.0f, 0.0f}, length(length), thickness(thickness), rotationDegrees(0.0f) {}

void Weapon::Update(const Vector2 &anchorPos, const Vector2 &targetPos)
{
    anchor = anchorPos;

    const float dx = targetPos.x - anchor.x;
    const float dy = targetPos.y - anchor.y;

    rotationDegrees = atan2f(dy, dx) * 180.0f / PI;
}

void Weapon::Draw() const
{
    const Rectangle rec = { anchor.x, anchor.y - thickness * 0.5f + 3, length, thickness };
    const Vector2 origin = { 0.0f, thickness * 0.5f };

    DrawRectanglePro(rec, origin, rotationDegrees, BLACK);

    DrawCircleV(anchor, thickness * 0.6f, BLACK);
}
