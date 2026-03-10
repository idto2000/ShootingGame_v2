#pragma once

#include "Actor/Actor.h"
#include "Util/Timer.h"
#include <vector>

using namespace Wanted;

class HomingBullet : public Actor
{
	RTTI_DECLARATIONS(HomingBullet, Actor)

public:

	// 생성할때 시작위치와 재밍했던 타겟
	HomingBullet(const Vector2& startPos, Actor* initialTarget);

	virtual ~HomingBullet();

	virtual void Tick(float deltaTime) override;

private:

	// 타겟이 죽었을 때 새로운 적 찾는 레이더 기능
	Actor* FindNewTarget();

private:

	Actor* currentTarget = nullptr;

	// 유도탄 비행 속도
	float speed = 120.0f;
	float xReal = 0.0f;
	float yReal = 0.0f;

	// 방향 유지르 위한 변수
	float dirX = 0.0f;
	float dirY = -1.0f;

	// 자동 폭팔 타이머
	Timer fuelTimer;

	// 경로를 다시 찾는 내비게이션 갱신 주기
	Timer pathUpdateTimer;

	// A*
	std::vector<Vector2> currentPath;
};

