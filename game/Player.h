#pragma once
#include "raylib.h"
#include <vector>
#include "Weapon.h"

struct EnvItem; 

class Player
{
public:
    Player();

    Vector2 position;
    float speed;
    bool canJump;

    int health;
    int maxHealth;

    Weapon weapon;

    void Update(float delta, const std::vector<EnvItem>& envItems);
    void Draw();
};


