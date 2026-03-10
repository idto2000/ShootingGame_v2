#pragma once

#include "Level/Level.h"
#include "Math/Vector2.h"
#include "Partition/QuadTree.h"

using namespace Wanted;

class GameLevel : public Level
{
	// RTTI 등록.
	RTTI_DECLARATIONS(GameLevel, Level)

public:
	GameLevel();
	~GameLevel();

	static GameLevel& Get();

	// 점수 변수.
	int score = 0;
	int coin = 0;

	// [시각화] 외부에서 상태 읽기만 할 수 있는 Gettr함수
	inline bool IsShowAStar() const { return isShowAStar; }

	//코인을 사용한 아이템 함수
	void KillAllEemies();
	void PlayerShield();
	void PlayerSpeedBuster();

	// Getter
	inline class QuadTree* GetQuadTree() const { return quadTree; }
	inline const std::vector<class Actor*>& GetActors() const { return actors; }

private:
	bool isSpeedBuffActive = false;
	virtual void Tick(float deltaTime) override;
	virtual void Draw() override;
	

	// 충돌 판정 처리 함수.
	void ProcessCollisionPlayerBulletAndEnemy();
	void ProcessCollisionPlayerAndEnemyBullet();

	//장애물 충돌 판정 처리 함수
	void ProcessCollisionPlayerBulletAndObstacle();
	void ProcessCollisionPlayerAndObstacle();

	//아이템 충돌 판정 처리 함수
	void ProcessCollisionPlayerAndItem();

	// 플레이어 총알과 적 총알의 충돌 처리 함수
	void ProcessCollisionPlayerBulletAndEnemyBullet();

	// 점수 보여주는 함수.
	void ShowScore();

	//딜레이 시간 함수();
	float restartDelayTime = 0.0f;

private:	

	// 플레이어가 죽었는지 확인.
	bool isPlayerDead = false;

	// 플레이어가 죽은 위치 (Draw에서 처리하기 위해 Tick에서 저장).
	Vector2 playerDeadPosition;

	// 점수 문자열.
	char scoreString[128] = {};

	// 전역적으로 접근하도록 만들기 위한 전역 변수.
	static GameLevel* instance;

	// QuadTree 포인터 선언
	class QuadTree* quadTree = nullptr;

	// QuadTree 시각화 변수 선언
	bool isShowQuadTree = false;

	bool isShowAStar = false;
};
