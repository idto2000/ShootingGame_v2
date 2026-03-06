#pragma once

#include "Actor/Actor.h"
#include "Util/Timer.h"

using namespace Wanted;

class Enemy : public Actor
{
	RTTI_DECLARATIONS(Enemy, Actor)

		//
		enum class AttackPattern
	{
		Single, // 1발 발사
		Triple, // 3발 부체꼴 발사
		Radial  // 360도 방사형 발사
	};

	// 이동 방향 열거형.
	enum class MoveDirection
	{
		None = -1,
		Left,
		Right
	};

public:
	// 패턴(pattern)을 입력받을 건데, 없을 경우 'Single'이 기본값.
	Enemy(const char* image = "(oOo)", int yPosition = 5,
		AttackPattern= AttackPattern::Single);
	~Enemy();

	// Tick.
	virtual void Tick(float deltaTime) override;

	// 대미지 받았을 때 처리할 함수.
	void OnDamaged();

private:
	// 이동 방향 열거형.
	MoveDirection direction = MoveDirection::None;

	// 패던을 기억하는 함수
	AttackPattern attackPattern = AttackPattern::Single;

	// 좌우 이동 처리를 위한 변수.
	float xPosition = 0.0f;
	float moveSpeed = 5.0f;

	// 발사 타이머.
	Timer timer;
};
