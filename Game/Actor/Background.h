#pragma once

#include "Actor/Actor.h"

using namespace Wanted;

class Background : public Actor
{
	RTTI_DECLARATIONS(Background, Actor)

public:
	Background(int x, int y, float speed, const char* image, Color color);
	~Background();


private:

	virtual void Tick(float deltaTime) override;


private:


	float moveSpeed = 0.0f;
	float yReal = 0.0f;
};

