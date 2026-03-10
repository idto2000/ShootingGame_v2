#pragma once

#include "Actor/Actor.h"
#include "Util/Timer.h"
//#include "Actor/Shield.h"
#include <vector>

using namespace Wanted;

class Player : public Actor
{
	
	// 무기 타입 정의.
	enum class WeaponMode
	{
		None = -1,
		singleShot = 0,
		TripleBurst = 1,
		SeriesShot = 2
	};

	RTTI_DECLARATIONS(Player, Actor)

public:
	Player();
	~Player();

	static Player& Get();
	bool isSpeedBuffActive = false;//현재 속도 버프 상태

	//외부에서 호출할 버프 활성화 함수
	void ActivetSpeedBoost(float duration, float boostMultiplier);
	bool IsSpeedBuffActive() const { return isSpeedBuffActive; }

	// 스코어에 알맞게 무기를 자동 설정(10 이상 3발, 20 이상 7발, 10이하 1발)
	void UpdateWeaponByScore(int score);

	//데미지
	void OnDamaged();

	static Player* GetInstance()
	{
		return instance;
	}

public:

	// 유도탄 탄약 
	int homingAmmo = 999;
	const int MAX_HOMING_AMMO = 999;

private:
	virtual void Tick(float deltaTime) override;
		
	// 오른쪽으로 이동하는 함수.
	void MoveRight(float deltaTime);
	
	// 왼쪽으로 이동하는 함수.
	void MoveLeft(float deltaTime);

	// 위쪽으로 이동하는 함수
	void MoveUp(float deltaTime);

	// 아래쪽으로 이동하는 함수
	void MoveDown(float deltaTime);

private:
	////쉴드를 그리는 함사
	//bool HasShield()
	//{
	//	return myShield != nullptr;
	//}
	//
	////쉴드를 지우는 함수
	//void BreakShield();

	//// 쉴드를 가리키는 포인터
	//Player* ownerPlayer = nullptr;

private:
	// 무기 설정 변경 함수
	void SetWeaponMode(WeaponMode mode);

	// 실제 발사 처리 함수
	void ProcessFiring(float deltaTime);

	// 무기 교체 함수
	void WeaponChange();

	// 발사 가능여부 확인 함수.
	bool CanShoot() const;

private:

	// 재밍관련 변수 및 함수
	bool isJamming = false;

	// 락온 타겟 초기화
	class Actor* lockedTarget = nullptr;

	// 궤적 이펙트를 그리는 주기(0.05초)
	Timer jammingEffectTimer;

	// 도배 방지용 발사 쿨 타임(1초)
	Timer homingCooldownTimer;

	// Enemy만 추적하는 레이더
	class Actor* FindClosestEnemyOnly();

	// 재밍 동작 및 유도탄 발사 처리
	void ProcessHomingMissile(float deltaTime);

private:
	float ApplyFricition(float currentVelocity, float friction, float deltaTime);
	void ApplyBoundaries();

	// 쿼드트리를 사용해 가장 가까운 우선순위가 높은 타겟을 찾는 함수
	class Actor* FindClosestTarget();

	// 7발 모드용 다중 타겟 탐색 함수
	std::vector<class Actor*> FindMultipleTargets(int count);
  
private:
	
	//쉴드를 갖고 있는 포인터
	//Shield* myShield = nullptr;
	
	// 타이머 변수.
	Timer timer;

	// 연사 시간 간격.
	float fireInterval = 1.5f;

	//속도 설정.
	float moveSpeed = 0.0f;

	//속도 설정(x축, y축)
	float xVelocity = 0.0f;
	float yVelocity = 0.0f;

	//관성 설정(가속도, 마찰력, 이동속도 제한)
	float acceleration = 80.0f;
	float friction = 40.0f;
	float maxSpeed = 100.0f;


	float orignalMaxSpeed;
	float orignalAcceleration;
	Timer speedBuffTimer;


	//위치 설정.
	float xReal = 0.0f;
	float yReal = 0.0f;

	WeaponMode currentMode = WeaponMode::None; // 현재 무기 모드

	// 1. 속도 및 설정
	int burstCountTotal = 1;       // 한 번 발사 시 나가는 총알 개수 (1, 3, 7)
	float bulletSpeed = 0.0f;

	// 2. 점사(Burst) 제어 변수
	bool isBursting = false;       // 현재 연사 중인지 여부
	int burstCountCurrent = 0;     // 현재 몇 발 쐈는지 카운트
	float burstDelay = 0.08f;      // 연사 시 총알 사이의 간격 (타-타-탕 할때의 간격)

	// 3. 타이머
	Timer mainTimer;  // 큰 주기의 발사 타이머 (재장전)
	Timer burstTimer; // 연사 사이의 짧은 타이머

	// 전역적으로 접근하도록 만들기 위한 전역 변수.
	static Player* instance;

	//딜레이 시간 함수();
	float restartDelayTime = 0.0f;


private:
	//회전 초기 각도 및 회전 속도 값
	float facingX = 0.0f;
	float facingY = -1.0f;
	
};
