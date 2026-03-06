#include "PlayerBullet.h"

#include <cmath>

const float PI = 3.141592f;

PlayerBullet::PlayerBullet(const Vector2& position,
	float angle, float speed)
	: super("@", position, Color::Blue)
{
	this->moveSpeed = speed;

	// 위치 설정
	this->xPosition = static_cast<float>(position.x);
	this->yPosition = static_cast<float>(position.y);

	float radian = angle * (PI / 180.0f);

	// 삼각함수를 이용한 속도 계산
	this->xVelocity = std::cosf(radian) * moveSpeed;
	this->yVelocity = std::sinf(radian) * moveSpeed;

}

PlayerBullet::~PlayerBullet()
{
}

void PlayerBullet::Tick(float deltaTime)
{
	super::Tick(deltaTime);

	// 현재 위치에 계산된 속도를 누적하여 이동
	xPosition += xVelocity * deltaTime;
	yPosition += yVelocity * deltaTime;

	// 경계검사(상하좌우 모두 체크)
	if (yPosition < 0.0f || yPosition >= 120.0f ||
		xPosition < 0.0f || xPosition >= 200.0f)
	{
		Destroy();
		return;
	}

	// 계산된 실수 좌표를 정수로 반환하여 실제 위치 갱신
	SetPosition(Vector2(static_cast<int>(std::round(xPosition)),
		static_cast<int>(std::round(yPosition))));
}
