#include "PathEffect.h"

PathEffect::PathEffect(const Vector2& position, const char* image, Color color)
	: Actor(image, position, color)
{
	sortingOrder = 200;

	// 수명 타이머 설정(0.15초)
	lifeTimer.SetTargetTime(0.3f);
	lifeTimer.Reset();
}

PathEffect::~PathEffect()
{
}

void PathEffect::Tick(float deltaTime)
{
	super::Tick(deltaTime);

	// 타이머 갱신 및 수명이 다하면 해제
	lifeTimer.Tick(deltaTime);

	if (lifeTimer.IsTimeOut() == true)
	{
		Destroy();
	}
}
