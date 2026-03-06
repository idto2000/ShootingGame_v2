#pragma once

#include "Actor/Actor.h"

using namespace Wanted;

class EnemyBullet : public Actor
{
	RTTI_DECLARATIONS(EnemyBullet, Actor)

public:
	// 총알 생성 및 발사 각도 추가
	EnemyBullet(
		const Vector2& position,
		float angle,
		float moveSpeed = 60.0f
	);

private:
	virtual void Tick(float deltaTime) override;

private:
	// 이동 처리를 위한 변수.
	float moveSpeed = 0.0f;

	// 좌표와 속도 변수.
	float xPositoin = 0.0f;
	float yPosition = 0.0f;
	float xVelocity = 0.0f;
	float yVelocity = 0.0f;

	//// y 이동 위치 처리를 위한 float 변수.
	//float yPosition = 0.0f;
};
