#include "EnemyBullet.h"
#include "Engine/Engine.h"

#include <cmath>

const float PI = 3.14f;

EnemyBullet::EnemyBullet(
	const Vector2& position, 
	float angle,
	float moveSpeed)
	: super("*", position, Color::Red),
	moveSpeed(moveSpeed), 
	xPositoin(static_cast<float>(position.x)),
	yPosition(static_cast<float>(position.y))
{
	//sortingOrder = 5;
	
	// 각도 변환.
	float radian = angle * (PI / 180.f);

	// 삼각함수를 이용해 속도 계산.
	xVelocity = std::cos(radian) * moveSpeed;
	yVelocity = std::sin(radian) * moveSpeed;
}

void EnemyBullet::Tick(float deltaTime)
{
	super::Tick(deltaTime);

	// 위치와 속도 업데이트.
	yPosition += yVelocity * deltaTime;
	xPositoin += xVelocity * deltaTime;

	// 경계검사 총알이 화면을 벗어나면 삭제(상하좌우 모두 체크)
	if (yPosition >= Engine::Get().GetHeight() ||
		yPosition < 0 || xPositoin >= Engine::Get().GetWidth() ||
		xPositoin < 0)
	{
		Destroy();
		return;
	}

	// 위치 설정.(반올림 정수를 픽셀로 변환)
	SetPosition(Vector2(static_cast<int>(std::round(xPositoin)),
		static_cast<int>(std::round(yPosition))));
}
