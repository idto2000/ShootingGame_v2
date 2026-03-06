#pragma once

#include "Actor/Actor.h"
#include "Util/Timer.h"

namespace Wanted
{
    class ObstacleSpawner : public Actor
    {
        RTTI_DECLARATIONS(ObstacleSpawner, Actor)

    public:

        ObstacleSpawner();

    private:

        virtual void Tick(float deltaTime) override;
        void SpawnObstacle(float deltaTime);


    private:
        Timer timer;
        float spawnTime = 2.5f; // 2.5초마다 하나씩 생성
    };
}
