#pragma once

#include "Actor/Actor.h"
#include "Util/Timer.h"

using namespace Wanted;

class PathEffect:public Actor
{
	RTTI_DECLARATIONS(PathEffect, Actor);

public:
	PathEffect(const Vector2& position, 
		const char* image = "*", Color color=Color::Green);
	virtual ~PathEffect();

	virtual void Tick(float deltaTime) override;


private:

	Timer lifeTimer;


};

