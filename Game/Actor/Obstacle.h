#pragma once

#include "Actor/Actor.h"
#include "Util/Timer.h"
#include "Enemy.h"

using namespace Wanted;

class Obstacle:public Actor
{
	RTTI_DECLARATIONS(Obstacle, Actor)	
		
public:
	Obstacle(const char* image = "(-O-)", int xPosition = 0, float speed = 0, int hp=0);
	~Obstacle();

	// Tick.
	virtual void Tick(float deltaTime) override;

	// 대미지 받았을 때 처리할 함수.
	void TakeDamaged(int damage =1);
	
	// 적 캐릭터와 충돌했을때 처리할 함수.
	void OnCollisionWithEnemy();

	// Player를 추적하는 함수
	void FollowPlayer(float deltaTime, Vector2 PlayerPos);

	//아이템 생성 시도 함수
	void TrySpawnItem();

private:

	// 추적용 위치 저장 
	float xReal = 0.0f;
	float yReal = 0.0f;

	// 시야 범위 측정
	float detection = 20.0f;
	
private:

	float verticalSpeed = 3.0f;   // 아래로 내려오는 속도
	float trackingSpeed = 2.0f;   // 플레이어를 쫓아가는 속도

	float xPosition = 0.0f;
	float yPosition = 0.0f;
	float moveSpeed = 0.0f;

	float itemDropChance = 1.03f;
	int hp = 1;
};

