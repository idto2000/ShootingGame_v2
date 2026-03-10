#include "Player.h"
#include "Actor/PlayerBullet.h"
#include "Core/Input.h"
#include "Engine/Engine.h"
#include "Level/Level.h"
#include "Level/TitleLevel.h"
#include "Render/Renderer.h"
#include "Level/GameLevel.h"
#include "Actor/EnemyDestroyEffect.h"
//#include "Actor/Shield.h"

#include "Actor/Enemy.h"
#include "Actor/obstacle.h"
#include "Actor/EnemyBullet.h"

#include "Util/PathFinder.h"
#include "HomngBullet.h"
#include "Actor/PathEffect.h"
//#include "Partition/QuadTree.h"

#include<iostream>
#include<cmath>//Player의 움직임을 부드럽게 하기 위한 수학함수
#include<algorithm>

// 전역 변수 초기화.
Player* Player::instance = nullptr;


Player::Player()
	: super("<=A=>", Vector2::Zero, Color::Yellow), orignalAcceleration(0.0f),
	orignalMaxSpeed(0.0f)
{
	instance = this;
	facingX = 0.0f;
	facingY = -1.0f;

	// 플레이어의 Width, Height 값 설정(플레이어를 중앙에 배치).
	xReal = static_cast<float>((Engine::Get().GetWidth() / 2) - (width / 2));
	yReal = static_cast<float>((Engine::Get().GetHeight() / 2) - (height / 2));
	SetPosition(Vector2(static_cast<int>(xReal), static_cast<int>(yReal)));
	
	//기본 무기 설정
	SetWeaponMode(WeaponMode::singleShot);
	
	//점사 관련 변수 초기화 
	isBursting = false;
	burstCountCurrent = 0;

	// 객체가 초기화되면 자기 자신의 주소를 저장.
	instance = this;

	// 플레이어의 출력 우선순위
	sortingOrder = 150;

	// 재밍 타이머 초기화
	jammingEffectTimer.SetTargetTime(0.05f);
	jammingEffectTimer.Reset();

	// 유도탄 발사 쿨 타임 초기화(1초)
	homingCooldownTimer.SetTargetTime(1.0f);
	homingCooldownTimer.Reset();
}

Player::~Player()
{
	// [추가] 사라질 때 등록 해제
	if (instance == this)
	{
		instance = nullptr;
	}
}

Player& Player::Get()
{
	// 싱글턴(Singleton).
	// 이 함수는 콘텐츠 프로젝트에서 접근함.
	// 따라서 엔진은 이미 초기화 완료 상태.
	if (!instance)
	{
		//return *nullptr;
		std::cout << "Error: Player::Get(). instance is null\n";

		// 디버그 모드에서만 동작함.
		// 자동으로 중단점 걸림.
		__debugbreak();
	}

	// Lazy-Pattern.
	// 이펙티브 C++에 나옴.
	//static Input instance;
	return *instance;
}

void Player::ActivetSpeedBoost(float duration, float boostMultiplier)
{

	if (!isSpeedBuffActive)
	{
		isSpeedBuffActive = true;
		orignalMaxSpeed = maxSpeed;
		orignalAcceleration = acceleration;

		maxSpeed *= boostMultiplier;
		acceleration *= boostMultiplier;

		isSpeedBuffActive = true;
	}

	speedBuffTimer.SetTargetTime(duration);
	speedBuffTimer.Reset();
}

void Player::UpdateWeaponByScore(int score)
{
	if (score >= 20)
	{
		if (currentMode != WeaponMode::SeriesShot)
		{
			SetWeaponMode(WeaponMode::SeriesShot);
		}
	}
	else if(score >= 10)
	{
		if (currentMode != WeaponMode::TripleBurst)
		{
			SetWeaponMode(WeaponMode::TripleBurst);
		}
	}
	else
	{
		if (currentMode != WeaponMode::singleShot)
		{
			SetWeaponMode(WeaponMode::singleShot);
		}
	}
}

void Player::OnDamaged()
{
	// 액터 제거.
	Destroy();

	// 이펙트 생성 (재생을 위해).
	GetOwner()->AddNewActor(new EnemyDestroyEffect(position));
}

void Player::Tick(float deltaTime)
{
	super::Tick(deltaTime);

	// 종료 처리.
	if (Input::Get().GetKeyDown(VK_ESCAPE))
	{
		// 게임 종료.
		//QuitGame();
		Engine::Get().SetNewLevel(new TitleLevel());
	}

	// 경과 시간 업데이트.
	timer.Tick(deltaTime);

	//대각선 이동시 빨라지는 현상 제어
	// 기존 가속도 임시 저장
	float currentAccel = acceleration;

	// 눌려진 키 검사
	bool isInputX = Input::Get().GetKey(VK_LEFT) || Input::Get().GetKey(VK_RIGHT);
	bool isInputY = Input::Get().GetKey(VK_UP) || Input::Get().GetKey(VK_DOWN);

	// 대각선으로 이동하는 키가 눌려지면 가속도 값 보정
	if (isInputX && isInputY)
	{
		acceleration = acceleration * 0.7071f;
	}

	// 입력 감지 변수.
	//bool isInputX = false;
	//bool isInputY = false;

	//무기 교체 입력
	if (Input::Get().GetKeyDown(VK_SPACE))
	{
		WeaponChange();
	}

	// 좌우 방향키 입력처리.
	if (Input::Get().GetKey(VK_LEFT))
	{
		MoveLeft(deltaTime);
		//isInputX = true;
	}

	if (Input::Get().GetKey(VK_RIGHT))
	{
		MoveRight(deltaTime);
		//isInputX = true;
	}

	// 상하 방향키 입력처리.
	if (Input::Get().GetKey(VK_UP))
	{
		MoveUp(deltaTime);
		//isInputY = true;
	}

	if (Input::Get().GetKey(VK_DOWN))
	{
		MoveDown(deltaTime);
		//isInputY = true;
	}

	acceleration = currentAccel;

	// 스피드 버프 타이머
	if (isSpeedBuffActive)
	{
		speedBuffTimer.Tick(deltaTime);
		if (speedBuffTimer.IsTimeOut())
		{
			//시간이 종료되면 원래 속도로 돌아감
			maxSpeed = orignalMaxSpeed;
			acceleration = orignalAcceleration;
			isSpeedBuffActive = false;
		}
	}


	// 코인을 사용한 아이템 구현_1 (폭탄)
	if (Input::Get().GetKeyDown('1'))
	{
		// 코인이 있는지 확인한다. 
		if (GameLevel::Get().coin >= 5)
		{
			GameLevel::Get().coin -= 5;

			GameLevel::Get().KillAllEemies();
		}
	}

	// 코인을 사용한 아이템 구현_2 (속도 업)
	if (Input::Get().GetKeyDown('2'))
	{
		// 코인이 있는지 확인한다. 
		if (GameLevel::Get().coin >= 5)
		{
			GameLevel::Get().coin -= 5;
			//속도 버프 조정(지속시간, 배속).
			Player::Get().ActivetSpeedBoost(5.01f, 2.3f);
		}
	}

	//// 코인을 사용한 아이템 구현_3 (쉴드)
	//if (Input::Get().GetKeyDown('3'))
	//{
	
	//	// 코인이 있는지 확인한다. 
	//	if (GameLevel::Get().coin >= 10 && !HasShield())
	//	{
	//		GameLevel::Get().coin -= 10;

	//		//쉴드 액터 생성하고 플레이어를 따라 다니게 설정
	//		myShield = new Shield(this);

	//		GetOwner()->AddNewActor(myShield);			
	//	}
	//}	

	//관성 적용
	if (isInputX == false)
	{
		xVelocity = ApplyFricition(xVelocity
			, friction, deltaTime);
	}

	if (isInputY == false)
	
	
	{
		yVelocity = ApplyFricition(yVelocity, friction, deltaTime);
	}

	//현재 속도의 제곱을 구합니다.(피타고라스 정의)
	float currenSpeedSq = (xVelocity * xVelocity) + (yVelocity * yVelocity);

	//현재 속도가 최대 속도를 넘어서면 비율을 맞춰 줄여준다.
	if (currenSpeedSq > maxSpeed * maxSpeed)
	{
		//현재 실제 속력을 구한다.(sqrt: 루트를 프로그래밍에서 계산해주는 함수)
		float currentSpeed = sqrt(currenSpeedSq);

		// (최대 속도 / 현재 속도) 비율만큼 X, Y 속도를 줄임
	   // 이렇게 하면 방향은 유지된 채 속도만 부드럽게 줄어듭니다.
		float scale = maxSpeed / currentSpeed;

		xVelocity *= scale;
		yVelocity *= scale;
	}

	//// 2개의 키가 입력되어 대각선 이동한다면 (true: *0.0707f) 아니면 (false: maxSpeed) 속도 그래도를 사용해라. 
	//float currentMax = (isInputX && isInputY) ? maxSpeed * 0.707f : maxSpeed;

	//if (xVelocity > currentMax)
	//{
	//	xVelocity = currentMax;
	//}
	//else if (xVelocity < -currentMax)
	//{
	//	xVelocity = -currentMax;
	//}

	//if (yVelocity > currentMax)
	//{
	//	yVelocity = currentMax;
	//}
	//else if (yVelocity < -currentMax)
	//{
	//	yVelocity = -currentMax;
	//}

	/*xReal += xVelocity * deltaTime;
	yReal += yVelocity * deltaTime;*/

	float yAspectRatio = 0.98f;
	xReal += xVelocity * deltaTime;
	yReal += (yVelocity * yAspectRatio) * deltaTime;

	//경계 검사 및 위치 확정
	ApplyBoundaries();
	SetPosition(Vector2(static_cast<int>(std::round(xReal)), 
		static_cast<int>(std::round(yReal))));

	// 총알 발사 함수
	ProcessFiring(deltaTime);

	// 유도탄 재밍 및 발사 
	ProcessHomingMissile(deltaTime);

	// 총알을 키 입력에 따른 발사로 변경


	// 스페이스 키를 활용해 탄약 발사.
	//if (fireMode == FireMode::OneShot)
	//{
	//	if (Input::Get().GetKeyDown(VK_SPACE))
	//	{
	//		Fire();
	//	}
	//}
	//else if (fireMode == FireMode::Repeat)
	//{
	//	if (Input::Get().GetKey(VK_SPACE))
	//	{
	//		FireInterval();
	//	}
	//}

	//// 발사 모드 전환.
	//if (Input::Get().GetKeyDown('R'))
	//{
	//	int mode = static_cast<int>(fireMode);
	//	mode = 1 - mode;
	//	fireMode = static_cast<FireMode>(mode);
	//}

}

//deltaTime으로 계산했기 때문에 컴퓨터 사양을 타지 않음
void Player::MoveRight(float deltaTime)
{
	facingX = 0.0f;
	facingY = -1.0f;

	xVelocity += acceleration * deltaTime;
}

void Player::MoveLeft(float deltaTime)
{
	facingX = 0.0f;
	facingY = -1.0f;

	xVelocity -= acceleration * deltaTime;
}

void Player::MoveUp(float deltaTime)
{
	facingX = 0.0f;
	facingY = -1.0f;

	yVelocity -= acceleration * deltaTime;
}

void Player::MoveDown(float deltaTime)
{
	facingX = 0.0f;
	facingY = -1.0f;

	yVelocity += acceleration * deltaTime;
}

//bool Player::HasShield()
//{
//	return false;
//}
//
//void Player::BreakShield()
//{
//	if (myShield != nullptr)
//	{
//		// 쉴드 액터를 화면에서 제거
//		myShield->Destroy();
//
//		// 내 변수 비우기
//		myShield = nullptr;
//	}
//}

//// 현재 이동 속도는 정수기반으로 세밀한 조정이 불가능함.
//// 프레임 의존때문에 컴퓨터 속도에 따라 속도가 달라짐(Tick마다 실행함)
//void Player::MoveRight()
//{
//	// 오른쪽 이동 처리.
//	position.x += 1;
//
//	// 좌표 검사.
//	// "<-=A=->"
//	if (position.x + width > Engine::Get().GetWidth())
//	{
//		position.x -= 1;
//	}
//}
//
//void Player::MoveLeft()
//{
//	// 왼쪽 이동 처리.
//	position.x -= 1;
//
//	// 좌표 검사.
//	if (position.x < 0)
//	{
//		position.x = 0;
//	}
//}
//
//void Player::MoveUp()
//{
//	position.y -= 1;
//
//	// 좌표 검사.
//	if (position.y < 0)
//	{
//		position.y = 0;
//	}
//}
//
//void Player::MoveDown()
//{
//	position.y += 1;
//
//	//좌표 검사.
//	if (position.y + 1 > Engine::Get().GetHeight())
//	{
//		// 화면 높이를 벗어났다면 마지막 칸에 고정
//		position.y = Engine::Get().GetHeight() - 1;
//	}
//}


// 무기 변경 모드가 추가되었고 무기 속성을 제어할 수 있도록 변경하였음. 
//void Player::Fire()
//{
//	// 경과 시간 초기화.
//	//elapsedTime = 0.0f;
//	timer.Reset();
//
//	// 위치 설정.
//	Vector2 bulletPosition(
//		position.x + (width / 2), 
//		position.y
//	);
//
//	// 액터 생성.
//	GetOwner()->AddNewActor(new PlayerBullet(bulletPosition, bulletSpeed));
//}

//void Player::FireInterval()
//{
//	// 발사 가능 여부 확인.
//	if (!CanShoot())
//	{
//		return;
//	}
//
//	// 발사.
//	Fire();
//}

bool Player::CanShoot() const
{
	// 경과 시간 확인.
	// 발사 간격보다 더 많이 흘렀는지.
	return timer.IsTimeOut();
}

Actor* Player::FindClosestEnemyOnly()
{
	const std::vector<Actor*>& allActors = GameLevel::Get().GetActors();
	Actor* bestTarget = nullptr;
	float minDistance = 99999.0f; 

	float pCenterX = (float)this->GetPosition().x + (this->GetWidth() / 2.0f);
	float pCenterY = (float)this->GetPosition().y + (this->GetHeight() / 2.0f);
	Vector2 pCenter((int)pCenterX, (int)pCenterY);

	for (Actor* actor : allActors)
	{
		// 적(Enemy)이 아니면 완전히 무시합니다.
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

// =========================================================
// [복구됨] Shift 재밍 시각화 및 Z키 발사 로직 (1번만 작성)
// =========================================================
void Player::ProcessHomingMissile(float deltaTime)
{
	homingCooldownTimer.Tick(deltaTime);
	jammingEffectTimer.Tick(deltaTime);

	// [재밍 모드] Shift 키 누름 + 쿨타임 지남 + 탄약 있음
	if (Input::Get().GetKey(VK_SHIFT) == true && homingCooldownTimer.IsTimeOut()
		== true && homingAmmo > 0)
	{
		isJamming = true;
		//lockedTarget = FindClosestEnemyOnly();

		if (lockedTarget == nullptr || lockedTarget->IsActive() == false)
		{
			lockedTarget = FindClosestEnemyOnly();
		}

		// 화면에 적이 있을 때만 궤적(녹색 길)을 그립니다.
		if (lockedTarget != nullptr)
		{
			Vector2 tPos = lockedTarget->GetPosition();
			Vector2 leftPos(tPos.x - 2, tPos.y); // 적의 왼쪽
			Vector2 rightPos(tPos.x + lockedTarget->GetWidth() + 1, tPos.y); // 적의 오른쪽

			Renderer::Get().Submit("[", leftPos, Color::Red, 250);
			Renderer::Get().Submit("]", rightPos, Color::Red, 250);

			float pCenterX = (float)this->GetPosition().x + (this->GetWidth() / 2.0f);
			float pCenterY = (float)this->GetPosition().y + (this->GetHeight() / 2.0f);
			Vector2 startPos((int)pCenterX, (int)pCenterY);

			float tCenterX = (float)lockedTarget->GetPosition().x + (lockedTarget->GetWidth() / 2.0f);
			float tCenterY = (float)lockedTarget->GetPosition().y + (lockedTarget->GetHeight() / 2.0f);
			Vector2 targetPos((int)tCenterX, (int)tCenterY);

			// A* 두뇌를 돌려 최적 경로 계산
			std::vector<Vector2> path = PathFinder::FindPath(startPos, targetPos);

			// 0.05초 간격으로 경로 위에 이펙트 뿌리기
			if (GameLevel::Get().IsShowAStar() == true)
			{
				if (jammingEffectTimer.IsTimeOut() == true && path.empty() == false)
				{
					jammingEffectTimer.Reset();
					for (int i = 0; i < (int)path.size(); ++i)
					{
						GetOwner()->AddNewActor(new PathEffect(path[i]));
					}
				}
			}
		}
	}
	else
	{
		isJamming = false;
		lockedTarget = nullptr;
	}

	// [발사] 재밍(록온) 상태에서 'Z' 키를 누르면 발사!
	if (isJamming == true && Input::Get().GetKeyDown('Z') == true)
	{
		if (lockedTarget != nullptr)
		{
			// 플레이어 중앙 좌표 계산
			float pCenterX = (float)this->GetPosition().x + (this->GetWidth() / 2.0f);
			float pCenterY = (float)this->GetPosition().y + (this->GetHeight() / 2.0f);

			Vector2 spawnPos((int)pCenterX, (int)pCenterY);

			GetOwner()->AddNewActor(new HomingBullet(this->GetPosition(), lockedTarget));
			
			homingAmmo = homingAmmo - 1; // 탄약 1개 소모
			homingCooldownTimer.Reset(); // 1초간 도배 방지 락온
			isJamming = false;
			lockedTarget = nullptr;
		}
	}
}

float Player::ApplyFricition(float currentVelocity, float friction, float deltaTime)
{
	//마찰력 구하기
	float amount = friction * deltaTime;

	//현재 속도가 양수 일때 (오른쪽, 아래) 관성 적용
	if (currentVelocity > 0.0f)
	{
		if (currentVelocity - amount < 0.0f)
		{
			return 0.0f;
		}
		else
		{
			return currentVelocity - amount;
		}
	}

	////현재 속도가 음수 일때 (왼쪽, 아래) 관성 적용
	else if (currentVelocity < 0.0f)
	{
		if (currentVelocity + amount > 0.0f)
		{
			return 0.0f;
		}
		else
		{
			return currentVelocity + amount;
		}
	}
	
	//속도가 0일 경우
	return 0.0;
}

//경계 체크 함수
void Player::ApplyBoundaries()
{
	// 가로(width) 경계 검사
	if (xReal < 0.0f)
	{
		xReal = 0.0f;
		xVelocity = 0.0f;
	}
	else if (xReal + (float)width > (float)Engine::Get().GetWidth())
	{
		xReal = (float)Engine::Get().GetWidth() - (float)width;
		xVelocity = 0.0f;
	}

	// 세로(heigth) 경계 검사
	if (yReal < 0.0f)
	{
		yReal = 0.0f;
		yVelocity = 0.0f;
	}

	//화면 아래 끝에 도달했을 때 콘솔의 '0'인텍스 문제로 화면을 벗어나 -1을 해주었음
	else if (yReal + (float)height >= (float)Engine::Get().GetHeight()-1.0f)
	{
		//화면 전체 높이에서 캐릭터 높이를 뺀다. 
		yReal = (float)Engine::Get().GetHeight() - (float)height-1.0f;
		yVelocity = 0.0f;
	}
}

Actor* Player::FindClosestTarget()
{
	//// 1. GameLevle에 존재하는 쿼드 트리 가져옴.
	//QuadTree* qTree = GameLevel::Get().GetQuadTree();
	//
	//// 예외 처리
	//if (!qTree)
	//{
	//	return nullptr;
	//}

	// GameLevel에 있는 모은 액터를 가져옴
	const std::vector<Actor*>& allActors = GameLevel::Get().GetActors();
	
	//// 2. 위 변경된 크리고 화면 전체 검사.
	//std::vector<Actor*> nearActors = qTree->Query(this);

	// 검사가 끝나면 플레이어 위치와 크기 원상 복구

	Actor* bestTarget = nullptr;

	// 숫자가 낮을 수록 우선순위가 높음
	int bestPriority = 999; 
	float minDistance = 99999.0f; // 최단 거리

	// 내(Player) 몸의 정중앙 좌표 X, Y 구하기
	float pCenterX = (float)this->GetPosition().x + (this->GetWidth() / 2.0f);
	float pCenterY = (float)this->GetPosition().y + (this->GetHeight() / 2.0f);
	Vector2 pCenter((int)pCenterX, (int)pCenterY);

	// 검사
	for (Actor* actor : allActors)
	{
		// 나를 비롯한 비활성화 액터 제외
		if (!actor || !actor->IsActive() || actor == this)
		{
			continue;
		}

		int currentPriority = 999;

		// 타입에 따른 우선순위 배정
		if (actor->IsTypeOf<Enemy>())
		{
			currentPriority = 1;
		}
		else if(actor->IsTypeOf<Obstacle>())
		{
			currentPriority = 2;
		}
		/*else if(actor->IsTypeOf<EnemyBullet>())
		{
			currentPriority = 3;
		}*/

		// 우선 순위들 중 최단 거리 검색
		if (currentPriority <= 3)
		{
			// 타겟의 몸 정중앙 좌표 X, Y 구하기
			float tCenterX = (float)actor->GetPosition().x + (actor->GetWidth() / 2.0f);
			float tCenterY = (float)actor->GetPosition().y + (actor->GetHeight() / 2.0f);
			Vector2 tCenter((int)tCenterX, (int)tCenterY);

			// 모서리가 아닌 '중심점과 중심점' 사이의 정확한 거리 계산
			float dist = Vector2::Distance(pCenter, tCenter);

			// 찾은 액터와 거리가 까가우면 타겟
			if ( dist< minDistance)
			{
				minDistance = dist;
				bestPriority = currentPriority;
				bestTarget = actor;
			}
			// 찾은 액터와 거리가 같다면 우선순위에 따름
			else if (dist == minDistance &&
				currentPriority <bestPriority)
			{
				bestPriority = currentPriority;
				bestTarget = actor;
			}
		}
	}
	return bestTarget;
}

struct TargetInfo
{
	Actor* actor;
	int priority;
	float distance;
};

std::vector<class Actor*> Player::FindMultipleTargets(int count)
{
	const std::vector<Actor*>& allActors = GameLevel::Get().GetActors();
	std::vector<TargetInfo> validTargets;

	float pCenterX = (float)this->GetPosition().x + (this->GetWidth() / 2.0f);
	float pCenterY = (float)this->GetPosition().y + (this->GetHeight() / 2.0f);
	Vector2 pCenter((int)pCenterX, (int)pCenterY);

	// 1. 전체 액터를 대상으로 거리 계산
	for (Actor* actor : allActors)
	{
		if (!actor || !actor->IsActive() || actor == nullptr)
		{
			continue;
		}

		int currentProiority = 999;

		// 찾은 액터를 확인 우선순위 적용
		if (actor->IsTypeOf<Enemy>())
		{
			currentProiority = 1;
		}
		else if (actor->IsTypeOf<Obstacle>())
		{
			currentProiority = 2;
		}
		/*else if (actor->IsTypeOf<EnemyBullet>())
		{
			currentProiority = 3;
		}*/

		if (currentProiority <= 3)
		{
			float pCenterX = (float)this->GetPosition().x + (this->GetWidth() / 2.0f);
			float pCenterY = (float)this->GetPosition().y + (this->GetHeight() / 2.0f);
			Vector2 pCenter((int)pCenterX, (int)pCenterY);			
			
			float dist = Vector2::Distance
			(this->GetPosition(), actor->GetPosition());
			
			TargetInfo info;
			info.actor = actor;
			info.priority = currentProiority;
			info.distance = dist;

			validTargets.push_back(info);
		}
	}

	// 2. 수집된 대상을 1순위: 최단 거리, 2순위: 우선순위로 정렬
	// std::sort(시작, 끝, 정렬_규칙)-> algorithm에서 제공하는 정렬 함수
	// []람다식: a, b를 비교해 true를 반환하며 a를 b보다 앞줄에 세운다.
	std::sort(validTargets.begin(), validTargets.end(),
		[](const TargetInfo& a, const TargetInfo& b)
		{
			// 거리가 같다면
			if (a.distance == b.distance)
			{
				return a.priority < b.priority;
			}
			return a.distance < b.distance;		
		});

	// 3. 앞에서부터 요청한 개수(count)만큼만 잘라서 반환합니다.
	std::vector<Actor*> result;
	int limit = (int)validTargets.size() < count ? (int)validTargets.size() : count;
	
	for (int i = 0; i < limit; ++i)
	{
		result.push_back(validTargets[i].actor);
	}
	return result;
}

void Player::SetWeaponMode(WeaponMode mode)
{
	currentMode = mode;

	switch (currentMode)
	{
	case Player::WeaponMode::singleShot:
		burstCountTotal = 1;
		fireInterval = 1.0f;
		bulletSpeed = 150.0f;
		burstDelay = 0.0f;
		break;

	case Player::WeaponMode::TripleBurst:
		burstCountTotal = 3;
		fireInterval = 0.8f;
		bulletSpeed = 180.0f;
		burstDelay = 0.15f;
		break;

	case Player::WeaponMode::SeriesShot:
		burstCountTotal = 7;
		fireInterval = 1.5f;
		bulletSpeed = 220.0f;
		burstDelay = 0.1f;
		break;
	}

	mainTimer.SetTargetTime(fireInterval);
	mainTimer.Reset();

	burstTimer.SetTargetTime(burstDelay);
	burstTimer.Reset();

	//연사 상태 초기화
	isBursting = false;
}

void Player::ProcessFiring(float deltaTime)
{
	// 1. 1발(단발) 및 3발(부채꼴) 모드: 모으지 않고 '즉시 동시 발사'
	if (currentMode == WeaponMode::singleShot || currentMode == WeaponMode::TripleBurst)
	{
		mainTimer.Tick(deltaTime);
		if (mainTimer.IsTimeOut())
		{
			mainTimer.Reset();
			Vector2 bulletPos(position.x + (width / 2), position.y);

			if (currentMode == WeaponMode::singleShot)
			{
				// [패턴 1] 1발: 가장 가까운 적에게 스마트 타겟팅
				float fireAngle = 270.0f;
				Actor* target = FindClosestTarget();
				if (target)
				{
					float dx = static_cast<float>(target->GetPosition().x - this->GetPosition().x);
					float dy = static_cast<float>(target->GetPosition().y - this->GetPosition().y);
					fireAngle = std::atan2(dy, dx) * (180.0f / 3.141592f);
				}
				GetOwner()->AddNewActor(new PlayerBullet(bulletPos, fireAngle, bulletSpeed));
			}
			else
			{
				// [패턴 2] 3발: 부채꼴 샷건
				float centerAngle = 270.0f;
				Actor* target = FindClosestTarget();
				if (target)
				{
					float dx = static_cast<float>(target->GetPosition().x - this->GetPosition().x);
					float dy = static_cast<float>(target->GetPosition().y - this->GetPosition().y);
					centerAngle = std::atan2(dy, dx) * (180.0f / 3.141592f);
				}

				// 타겟을 향한 중앙 각도를 기준으로 -15도, 0도, +15도 방향 3발 흩뿌리기
				float angles[3] = { centerAngle - 15.0f, centerAngle, centerAngle + 15.0f };
				for (int i = 0; i < 3; ++i)
				{
					GetOwner()->AddNewActor(new PlayerBullet(bulletPos, angles[i], bulletSpeed));
				}
			}
		}
	}
	// 2. 7발(다중 록온) 모드: 0.1초 간격으로 타겟을 바꿔가며 '순차적 연사'
	else if (currentMode == WeaponMode::SeriesShot)
	{
		if (!isBursting)
		{
			mainTimer.Tick(deltaTime);
			if (mainTimer.IsTimeOut())
			{
				mainTimer.Reset();
				isBursting = true;
				burstCountCurrent = 0;
				burstTimer.Reset();
			}
		}
		else
		{
			burstTimer.Tick(deltaTime);
			if (burstCountCurrent == 0 || burstTimer.IsTimeOut())
			{
				burstTimer.Reset();
				Vector2 bulletPos(position.x + (width / 2), position.y);
				float fireAngle = 270.0f;

				// 매 발사(0.1초)마다 최대 7개의 타겟을 다시 찾음
				std::vector<Actor*> targets = FindMultipleTargets(7);

				if (!targets.empty())
				{
					// 현재 몇 번째 쏘는 총알인지에 따라 타겟을 다르게 배정 (Round-Robin)
					Actor* target = targets[burstCountCurrent % targets.size()];
					float dx = static_cast<float>(target->GetPosition().x - this->GetPosition().x);
					float dy = static_cast<float>(target->GetPosition().y - this->GetPosition().y);
					fireAngle = std::atan2(dy, dx) * (180.0f / 3.141592f);
				}

				GetOwner()->AddNewActor(new PlayerBullet(bulletPos, fireAngle, bulletSpeed));
				burstCountCurrent++;

				// 7발을 다 쏘면 연사 종료
				if (burstCountCurrent >= burstCountTotal)
				{
					isBursting = false;
				}
			}
		}
	}
}

void Player::WeaponChange()
{
	//현재 모드를 int형으로 형변환.
	int modeNum = (int)currentMode;

	//다음 모드로 변경
	modeNum = (modeNum + 1) % 3;

	SetWeaponMode((WeaponMode)modeNum);
}
