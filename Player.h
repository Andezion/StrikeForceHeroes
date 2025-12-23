#pragma once
#include "raylib.h"
#include <vector>

struct EnvItem; 

class Player
{
public:
	Player();

	Vector2 position;
	float speed;
	bool canJump;

	void Update(float delta, const std::vector<EnvItem>& envItems);
};


