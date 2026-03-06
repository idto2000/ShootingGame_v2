#include "ObstacleSpawner.h"
#include "Obstacle.h"
#include "Level/Level.h" 
#include "Engine/Engine.h"
#include "Util/Util.h"


namespace Wanted
{
	static const char* ObstacleType[] =
	{
		";:^:;",
		"zZwZz",
		"oO@Oo",
		"<-=->",
		")qOp(",
	};

	ObstacleSpawner::ObstacleSpawner()
	{
		// 적 생성 타이머 설정.
		timer.SetTargetTime(Util::RandomRange(2.0f, 3.0f));
	}

	void ObstacleSpawner::Tick(float deltaTime)
	{
		super::Tick(deltaTime);

		SpawnObstacle(deltaTime);
	}

	void ObstacleSpawner::SpawnObstacle(float deltaTime)
	{
		// 타이머 업데이트.
		timer.Tick(deltaTime);

		// 경과 시간 확인.
		if (!timer.IsTimeOut())
		{
			return;
		}

		// 타이머 초기화.
		timer.Reset();

		static int length = sizeof(ObstacleType) / sizeof(ObstacleType[0]);

		// 랜덤 인덱스.
		int index = Util::Random(0, length - 1);

		// 생성 y 위치.
		int screenWight = Engine::Get().GetWidth();

		// 장애물 길이를 고려하여 여유값 지정
		int xPostion = Util::Random(0, screenWight - 6);

		//하강 속도를 랜덤하게 생성(0.5. ~ 2.5)
		float randomSpeed = Util::RandomRange(0.5f, 3.5f);

		// 체력 랜덤하게 
		int randomHP = Util::Random(1, 5);

		// 적 생성 요청.
		GetOwner()->AddNewActor(new Obstacle
		(ObstacleType[index], xPostion, randomSpeed, randomHP));
	}

}