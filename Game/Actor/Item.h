#pragma once

#include "Actor/Actor.h"
#include "Util/Timer.h"


using namespace Wanted;

class Item : public Actor
{
	RTTI_DECLARATIONS(Item, Actor)

public:
	// [추가] 외부에서 공유해서 쓸 아이템 모양 데이터 선언
	static const char* ModelTypes[];
	static const int ModelCount;

public:

	Item(const char* image, int x, int y, float speed, Color color);
	~Item();
	
	//Tick
	virtual void Tick(float deltaTime) override;

	// 플레이어와 충돌했을 때 
	void TakeDamaged();

private:

	float moveSpeed = 0.0f;

	float yReal = 0.0f;
	float xReal = 0.0f;


};

