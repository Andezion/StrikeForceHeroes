#include "Weapon.h"
#include "raylib.h"
#include "raymath.h"
#include "Game.h" 
#include <cmath>

Weapon::Weapon(const float length, const float thickness, const float bulletSpeed, const float cooldown)
    : anchor{0.0f, 0.0f}, length(length), thickness(thickness), rotationDegrees(0.0f),
      cooldown(cooldown), cooldownTimer(0.0f), bulletSpeed(bulletSpeed), bullets()
{
}

void Weapon::Update(const float delta, const Vector2 &anchorPos, const Vector2 &targetPos, const std::vector<EnvItem> &envItems, const float spreadRadius)
{
    anchor = anchorPos;

    const float dx = targetPos.x - anchor.x;
    const float dy = targetPos.y - anchor.y;

    rotationDegrees = atan2f(dy, dx) * 180.0f / PI;

    if (cooldownTimer > 0.0f) cooldownTimer -= delta;

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && cooldownTimer <= 0.0f)
    {
        const float rad = rotationDegrees * PI / 180.0f;
        const Vector2 endPos = { anchor.x + cosf(rad) * length, anchor.y + sinf(rad) * length };

        bullets.emplace_back(endPos, targetPos, bulletSpeed, spreadRadius);
        cooldownTimer = cooldown;
    }

    for (auto it = bullets.begin(); it != bullets.end(); )
    {
        if (!it->Update(delta, envItems))
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
