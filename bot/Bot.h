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
    Bot(Vector2 startPos, float difficulty = 0.5f, float aggression = 0.5f);

    Vector2 position;
    float   speed;
    bool    canJump;

    int health;
    int maxHealth;

    float difficulty;
    float aggression;

    float damageMultiplier;
    float maxIdleTime;
    float visionRadius;

    Weapon weapon;

    bool showVisionDebug = true;

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
    float patrolDir;

    Vector2 lastPlayerPos = { 0.0f, 0.0f };
    bool    lastHasLOS    = false;

    [[nodiscard]] bool HasLineOfSight(Vector2 playerPos,
                                      const std::vector<EnvItem>& envItems) const;

    static constexpr float ATTACK_RANGE = 450.0f;
    static constexpr float HOR_SPEED    = 290.0f;
    static constexpr float JUMP_SPEED   = 500.0f;
    static constexpr float GRAVITY      = 600.0f;
};
