#pragma once

#include "Actor/Actor.h"
#include "Util/Timer.h"

namespace Wanted
{
	class ItemSpawner : public Actor
	{
		RTTI_DECLARATIONS(ItemSpawner, Actor)

	public:
		ItemSpawner();

	private:
		virtual void Tick(float deltaTime) override;
		void SpawnItem(float deltaTime);

	private:
		Timer timer;
		float spawnTime = 2.5f;

	};


}

