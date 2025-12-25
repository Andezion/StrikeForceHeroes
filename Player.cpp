#include "Player.h"
#include "Game.h"
#include "raylib.h"

#include <cmath>

#define G 600
#define PLAYER_JUMP_SPD 500.0f
#define PLAYER_HOR_SPD 400.0f

Player::Player()
	: position{100.0f, 500.0f}, speed(0.0f), canJump(false)
{
}

void Player::Update(const float delta, const std::vector<EnvItem>& envItems)
{
	constexpr float halfWidth = 10.0f;
	constexpr float fullHeight = 60.0f;

	if (IsKeyDown(KEY_A))
	{
		position.x -= PLAYER_HOR_SPD * delta;
	}
	if (IsKeyDown(KEY_D))
	{
		position.x += PLAYER_HOR_SPD * delta;
	}

	if (IsKeyPressed(KEY_W) && canJump)
	{
		speed = -PLAYER_JUMP_SPD;
		canJump = false;
	}

	speed += G * delta;
	position.y += speed * delta;

	Rectangle playerRect = {
		position.x - halfWidth,
		position.y - fullHeight,
		halfWidth * 2.0f,
		fullHeight
	};

	bool grounded = false;

	for (const auto &[rect, blocking, color] : envItems)
	{
		if (!blocking)
		{
			continue;
		}

		if (const Rectangle &rectangle = rect; CheckCollisionRecs(rectangle, playerRect))
		{
			const float overlapLeft = playerRect.x + playerRect.width - rectangle.x;
			const float overlapRight = rectangle.x + rectangle.width - playerRect.x;
			const float overlapTop = playerRect.y + playerRect.height - rectangle.y;
			const float overlapBottom = rectangle.y + rectangle.height - playerRect.y;

			const float minOverlap = fminf(
				fminf(overlapLeft, overlapRight),
				fminf(overlapTop, overlapBottom)
			);

			if (minOverlap == overlapTop)
			{
				position.y = rectangle.y;
				speed = 0.0f;

				grounded = true;
			}
			else if (minOverlap == overlapBottom)
			{
				position.y = rectangle.y + rectangle.height + fullHeight;
				speed = 0.0f;
			}
			else if (minOverlap == overlapLeft)
			{
				position.x = rectangle.x - halfWidth;
			}
			else
			{
				position.x = rectangle.x + rectangle.width + halfWidth;
			}

			playerRect.x = position.x - halfWidth;
			playerRect.y = position.y - fullHeight;
		}
	}

	canJump = grounded;
}
