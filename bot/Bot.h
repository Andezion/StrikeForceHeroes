#pragma once
#include "raylib.h"
#include <vector>
#include "Weapon.h"

struct EnvItem;
class Particle;

enum class BotState {
    IDLE,
    PATROL,
    CHASE,
    ATTACK
};

class Bot
{
public:
    // difficulty: 0.0 (easy) .. 1.0 (hard)
    //   - damage multiplier: 0.5 + difficulty * 1.5  => [0.5 .. 2.0]
    //   - max idle time:     3.0 - difficulty * 2.5  => [0.5 .. 3.0] sec
    // aggression: 0.0 (passive) .. 1.0 (very aggressive)
    //   - vision radius:    200 + aggression * 600   => [200 .. 800] units
    Bot(Vector2 startPos, float difficulty = 0.5f, float aggression = 0.5f);

    Vector2 position;
    float   speed;
    bool    canJump;

    int health;
    int maxHealth;

    float difficulty;
    float aggression;

    // Derived stats (computed in constructor)
    float damageMultiplier;
    float maxIdleTime;
    float visionRadius;

    Weapon weapon;

    [[nodiscard]] Rectangle GetRect() const;
    [[nodiscard]] bool IsDead() const { return health <= 0; }

    void Update(float delta, const std::vector<EnvItem>& envItems,
                Vector2 playerPos, std::vector<Particle>& particles);
    void Draw() const;

    [[nodiscard]] BotState GetState() const { return state; }

private:
    BotState state;
    float idleTimer;
    float patrolTimer;
    float patrolDir;   // -1.0 = left, +1.0 = right

    static constexpr float ATTACK_RANGE = 450.0f;
    static constexpr float HOR_SPEED    = 290.0f;
    static constexpr float JUMP_SPEED   = 500.0f;
    static constexpr float GRAVITY      = 600.0f;
};
