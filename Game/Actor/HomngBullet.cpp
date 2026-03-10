#include "HomngBullet.h"
#include "Actor/Enemy.h"
#include "Actor/EnemyDestroyEffect.h"
#include "Util/PathFinder.h"
#include "Level/GameLevel.h"
#include "Engine/Engine.h"
#include "Actor/PathEffect.h"


#include <cmath>


HomingBullet::HomingBullet(const Vector2& startPos, Actor* initialTarget)
    : Actor("M", startPos, Color::Red) 
{
    currentTarget = initialTarget;

    xReal = (float)startPos.x;
    yReal = (float)startPos.y;

	sortingOrder = 140;

    // 연료 설정(3초)
	fuelTimer.SetTargetTime(3.0f);
	fuelTimer.Reset();

    // 길찾기 갱신 주기(0.1초)
    pathUpdateTimer.SetTargetTime(0.1f);
    pathUpdateTimer.Reset();
}

HomingBullet::~HomingBullet()
{
}

void HomingBullet::Tick(float deltaTime)
{
	super::Tick(deltaTime);

	// 1. 연료 검사 (3초 지나면 공중 폭발)
	fuelTimer.Tick(deltaTime);
	if (fuelTimer.IsTimeOut() == true)
	{
		Destroy();
		return;
	}

	// 2. 화면 밖으로 나가면 파괴
	if (xReal < 0 || xReal > Engine::Get().GetWidth() ||
		yReal < 0 || yReal > Engine::Get().GetHeight())
	{
		Destroy();
		return;
	}

	// 3. 타겟 유효성 검사 및 자동 타겟 전환(Target Transfer)
	if (currentTarget == nullptr || currentTarget->IsActive() == false)
	{
		currentTarget = FindNewTarget(); // 기존 타겟이 죽었으면 새 적을 찾음!
	}

	//  '정중앙'을 기준으로 모든 것을 계산합니다!
	float bCenterX = xReal + (GetWidth() / 2.0f);
	float bCenterY = yReal + (GetHeight() / 2.0f);
	Vector2 bCenter((int)bCenterX, (int)bCenterY);

	// 4. 내비게이션 경로 갱신 (0.1초 마다)
	pathUpdateTimer.Tick(deltaTime);
	if (currentTarget != nullptr && pathUpdateTimer.IsTimeOut() == true)
	{
		pathUpdateTimer.Reset();

		//Vector2 myPos((int)xReal, (int)yReal);
		Vector2 targetPos = currentTarget->GetPosition();

		// 적의 중앙을 향하도록 좌표 설정
		targetPos.x += currentTarget->GetWidth() / 2;
		targetPos.y += currentTarget->GetHeight() / 2;

		// 중앙 좌표를 넘겨주어 경로를 찾아오게 한다.
		currentPath = PathFinder::FindPath(bCenter, targetPos);

		// A* 시각화가 켜져있다면, 유도탄이 실시간으로 계산하는 경로도 뿌려줍니다!
		if (GameLevel::Get().IsShowAStar() == true)
		{
			for (int i = 0; i < (int)currentPath.size(); ++i)
			{
				GetOwner()->AddNewActor(new PathEffect(currentPath[i], ".", Color::Yellow));
			}
		}
	}

	// 5. 비행(이동) 로직 계산
	if (currentTarget != nullptr && currentPath.empty() == false)
	{
		// A*가 알려준 바로 앞의 노드를 향해 방향을 틂
		Vector2 nextNode;
		int targetIndex = 0;
		if (currentPath.size() > 2)
		{
			targetIndex = 2;;
		}
		else if(currentPath.size() > 1)
		{
			targetIndex = 1;
		}

		nextNode = currentPath[targetIndex];

		float dx = (float)nextNode.x - bCenterX;
		float dy = (float)nextNode.y - bCenterY;

		float length = std::sqrt(dx * dx + dy * dy);
		if (length > 0.0f)
		{
			dirX = dx / length;
			dirY = dy / length;
		}
	}

	// 실제 이동 적용
	xReal += dirX * speed * deltaTime;
	yReal += dirY * speed * deltaTime;

	SetPosition(Vector2((int)xReal, (int)yReal));

	// 6. 충돌 검사 (Enemy와 부딪히면 파괴)
	const std::vector<Actor*>& actors = GameLevel::Get().GetActors();
	for (Actor* actor : actors)
	{
		if (actor != nullptr && actor->IsActive() == true && actor->IsTypeOf<Enemy>())
		{
			float tCenterX = actor->GetPosition().x + actor->GetWidth() / 2.0f;
			float tCenterY = actor->GetPosition().y + actor->GetHeight() / 2.0f;
			Vector2 tCenter((int)tCenterX, (int)tCenterY);

			if (TestIntersect(actor) == true || Vector2::Distance(bCenter, tCenter) <= 15.0f)
			{
				Enemy* enemy = dynamic_cast<Enemy*>(actor);
				if (enemy != nullptr)
				{
					enemy->OnDamaged(); // 적에게 데미지
				}
				Destroy(); // 유도탄 자신도 파괴
				return;
			}
		}
	}
}

Actor* HomingBullet::FindNewTarget()
{
	const std::vector<Actor*>& allActors = GameLevel::Get().GetActors();
	Actor* bestTarget = nullptr;
	float minDistance = 99999.0f;

	float pCenterX = xReal + (GetWidth() / 2.0f);
	float pCenterY = yReal + (GetHeight() / 2.0f);
	Vector2 pCenter((int)pCenterX, (int)pCenterY);

	for (Actor* actor : allActors)
	{
		if (actor != nullptr && actor->IsActive() == true && actor->IsTypeOf<Enemy>())
		{
			float tCenterX = (float)actor->GetPosition().x + (actor->GetWidth() / 2.0f);
			float tCenterY = (float)actor->GetPosition().y + (actor->GetHeight() / 2.0f);
			Vector2 tCenter((int)tCenterX, (int)tCenterY);

			float dist = Vector2::Distance(pCenter, tCenter);
			if (dist < minDistance)
			{
				minDistance = dist;
				bestTarget = actor;
			}
		}
	}
	return bestTarget;
}
