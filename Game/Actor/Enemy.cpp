#include "Enemy.h"
#include "Util/Util.h"
#include "Engine/Engine.h"
#include "Level/Level.h"
#include "Actor/EnemyBullet.h"
#include "Actor/EnemyDestroyEffect.h"
#include "Player.h"

#include <cmath>

Enemy::Enemy(const char* image, int yPosition, AttackPattern pattern)
	: super(image), attackPattern(pattern)
{
	// 랜덤 (오른쪽 또는 왼쪽으로 이동할지 결정).
	int random = Util::Random(1, 10);

	if (random % 2 == 0)
	{
		// 화면 오른쪽에서 생성. "(oOo)"
		direction = MoveDirection::Left;
		xPosition = static_cast<float>(
			Engine::Get().GetWidth() - width - 1
			);
	}
	else
	{
		// 화면 왼쪽에서 생성.
		direction = MoveDirection::Right;
		xPosition = 0.0f;
	}

	// 이동 방향에 따른 적 위치 설정.
	SetPosition(
		Vector2(static_cast<int>(xPosition), yPosition)
	);

	// 발사 타이머 목표 시간 설정.
	timer.SetTargetTime(Util::RandomRange(1.0f, 3.0f));

	sortingOrder = 130;
}

Enemy::~Enemy()
{
}

void Enemy::Tick(float deltaTime)
{
	super::Tick(deltaTime);

	//플레이어를 호출할 수 있기에 안전하게 정리한다.
	Player* player = Player::GetInstance();
	float targetAngle = 90.0f;// 기본 방향

	if (player == nullptr)
	{
		return;
	}	

	Vector2 playerPos = player->GetPosition();
	Vector2 enemyPos = position;
	
	if (player != nullptr)
	{
		float dx = (float)(player->GetPosition().x - enemyPos.x);
		float dy = (float)(player->GetPosition().y - enemyPos.y);

		targetAngle = std::atan2f(dy, dx) * (180.0f / 3.14f);
	}

	// 이동 처리.
	float dir
		= direction == MoveDirection::Left ? -1.0f : 1.0f;
	xPosition = xPosition + moveSpeed * dir * deltaTime;

	// 좌표 검사.
	// 화면 왼쪽을 완전히 벗어났으면.
	if (xPosition + width < 0)
	{
		Destroy();
		return;
	}

	// 화면 오른쪽을 완전히 벗어났으면.
	if (xPosition > Engine::Get().GetWidth() - 1)
	{
		Destroy();
		return;
	}

	// 위치 설정.
	SetPosition(Vector2(
		static_cast<int>(xPosition),
		position.y
	));

	// 발사 타이머 업데이트.
	timer.Tick(deltaTime);
	if (!timer.IsTimeOut())
	{
		return;
	}

	// 타이머 리셋.
	timer.Reset();

	//// 조준을 위한 위치 값 체크.
	//Vector2 playerPos = player->GetPosition();
	//Vector2 enemyPos = position;

	//// 플레이이와 적 간의 위치 체크
	//float dx = (float)(playerPos.x - enemyPos.x);
	//float dy = (float)(playerPos.y - enemyPos.y);

	//// 거리 계산(수학: atan2f) 각도
	//float radian = std::atan2f(dy, dx);
	//float degree = radian * (18.0f / 3.14f);

	// 패턴별 총알 발사
	Vector2 bulletSpawnPos(position.x + width / 2, position.y);
	float speed = Util::RandomRange(40.0f, 60.0f);

	if (GetOwner() == nullptr)
	{
		return;
	}

	switch (attackPattern)
	{
	case Enemy::AttackPattern::Single:
		GetOwner()->AddNewActor(new EnemyBullet(bulletSpawnPos, targetAngle, speed));
		break;
	case Enemy::AttackPattern::Triple:
	{
		// 70도, 90도 110도 방향으로 부채꼴 발사(3발)
		float angles[3] = { -20.0f, 0.0f, 20.0f };
		for (int i = 0; i < 3; ++i)
		{
			float finalAngle = targetAngle + angles[i];
			GetOwner()->AddNewActor(new EnemyBullet(bulletSpawnPos,
				finalAngle, speed));
		}
		break;
	}		
	case Enemy::AttackPattern::Radial:
	{

		// 360도 방사형 발사(16발)
		// 360을 16으로 나누면 22.5가 나오고 이를 각 총알에 곱하면 
		int bulletCount = 8;
		float anglenStep = 360.0f / bulletCount;
		for (int i = 0; i < bulletCount; ++i)
		{
			GetOwner()->AddNewActor(new EnemyBullet(bulletSpawnPos, i * anglenStep, speed));
		}
		break;
	}
	}

	//// 탄약 발사.
	//GetOwner()->AddNewActor(new EnemyBullet(
	//	Vector2(position.x + width / 2, position.y),
	//	Util::RandomRange(10.0f, 20.0f)
	//));	
}

void Enemy::OnDamaged()
{
	// 액터 제거.
	Destroy();

	// 이펙트 생성 (재생을 위해).
	GetOwner()->AddNewActor(new EnemyDestroyEffect(position));
}
