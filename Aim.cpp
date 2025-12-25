#include "Aim.h"
#include "raylib.h"
#include "raymath.h"

Aim::Aim(const Type type)
    : type(type) {}

float Aim::RemapClamped(const float value, const float inMin, const float inMax,
    const float outMin, const float outMax)
{
    if (inMax <= inMin)
    {
        return outMin;
    }

    float t = (value - inMin) / (inMax - inMin);

    if (t < 0.0f)
    {
        t = 0.0f;
    }
    if (t > 1.0f)
    {
        t = 1.0f;
    }

    return outMin + t * (outMax - outMin);
}

void Aim::Update(const Vector2& playerWorld, const Vector2& cursorWorld, const Camera2D& camera)
{
    const float dist = Vector2Distance(playerWorld, cursorWorld);

    const float minDist = 20.0f;
    const float maxDist = 700.0f;
    const float minRadius = 8.0f;
    const float maxRadius = 60.0f;

    radius = RemapClamped(dist, minDist, maxDist, minRadius, maxRadius);
    innerRadius = radius * 0.55f;

    cursorScreen = GetWorldToScreen2D(cursorWorld, camera);
}

void Aim::Draw() const
{
    DrawCircleLines((int)roundf(cursorScreen.x), (int)roundf(cursorScreen.y), radius, color);
    DrawCircleLines((int)roundf(cursorScreen.x), (int)roundf(cursorScreen.y), innerRadius, color);

    const float gap = innerRadius + 6.0f;
    const float lineLen = radius + 8.0f;


    DrawLineV({ cursorScreen.x, cursorScreen.y - lineLen }, { cursorScreen.x, cursorScreen.y - gap }, color);

    DrawLineV({ cursorScreen.x, cursorScreen.y + gap }, { cursorScreen.x, cursorScreen.y + lineLen }, color);

    DrawLineV({ cursorScreen.x - lineLen, cursorScreen.y }, { cursorScreen.x - gap, cursorScreen.y }, color);

    DrawLineV({ cursorScreen.x + gap, cursorScreen.y }, { cursorScreen.x + lineLen, cursorScreen.y }, color);


    const float tickLen = 8.0f;
    DrawLineV({ cursorScreen.x - radius, cursorScreen.y - tickLen }, { cursorScreen.x - radius, cursorScreen.y + tickLen }, color);
    DrawLineV({ cursorScreen.x + radius, cursorScreen.y - tickLen }, { cursorScreen.x + radius, cursorScreen.y + tickLen }, color);
    DrawLineV({ cursorScreen.x - tickLen, cursorScreen.y - radius }, { cursorScreen.x + tickLen, cursorScreen.y - radius }, color);
    DrawLineV({ cursorScreen.x - tickLen, cursorScreen.y + radius }, { cursorScreen.x + tickLen, cursorScreen.y + radius }, color);
}
