#include "GameLevel.h"
#include "Actor/Player.h"
#include "Actor/Enemy.h"
#include "Actor/PlayerBullet.h"
#include "Actor/EnemyBullet.h"
#include "Actor/EnemySpawner.h"
#include "Actor/MouseTester.h"
#include "Render/Renderer.h"
#include "Engine/Engine.h"
#include "Actor/ObstacleSpawner.h"
#include "Actor/Obstacle.h"
#include "Actor/Item.h"
#include "Actor/ItemSpawner.h"
#include "Level/TitleLevel.h"
#include "Util/Util.h"
#include "Actor/Background.h"
#include "Core/Input.h"

#include "Partition/Bounds.h"
#include "Partition/QuadTree.h"
#include "Partition/QuadNode.h"

#include <iostream>

//정적 변수 초기화
GameLevel* GameLevel::instance = nullptr;
using namespace Wanted;

GameLevel::GameLevel()
{

	instance = this;
	restartDelayTime = 0.0f;

	//배경: 원근감을 위해 3개의(Layer)로 나눈다.
	int screenWidth = Engine::Get().GetWidth();
	int screenHeight = Engine::Get().GetHeight();

	//레이어 1
	for (int i = 0; i < 50; ++i)
	{
		int x = Util::Random(0, screenWidth-1);
	    int y = Util::Random(0, screenHeight-1);

		AddNewActor(new Background(x, y, Util::RandomRange(2.0f, 5.0f), ".", Color::Red));
	}

	//레이어 2
	for (int i = 0; i < 20; ++i)
	{
		int x = Util::Random(0, screenWidth - 1);
		int y = Util::Random(0, screenHeight - 1);

		AddNewActor(new Background(x, y, Util::RandomRange(8.0f, 12.0f), ".", Color::Green));
	}

	//레이어 3
	for (int i = 0; i < 5; ++i)
	{
		int x = Util::Random(0, screenWidth - 1);
		int y = Util::Random(0, screenHeight - 1);

		AddNewActor(new Background(x, y, Util::RandomRange(20.0f, 30.0f), ".", Color::Blue));
	}

	// Player 액터 추가.
	AddNewActor(new Player());
	
	// 적 생성기 추가.
	AddNewActor(new EnemySpawner());

	// Test: 마우스 테스터 추가.
	AddNewActor(new MouseTester());

	// 장애물 추가
	AddNewActor(new ObstacleSpawner());

	// 아이템 추가
	AddNewActor(new ItemSpawner());
}

GameLevel::~GameLevel()
{
	isPlayerDead = false;
	// [추가 3] 내가 사라질 때 instance도 비워줌
	if (instance == this)
	{
		instance = nullptr;
	}
}

GameLevel& GameLevel::Get()
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

void GameLevel::Tick(float deltaTime)
{
	super::Tick(deltaTime);

	//플레이어가 죽었다면 타이틀 레벨로 이동
	if (isPlayerDead)
	{
		restartDelayTime += deltaTime;

		if (restartDelayTime >= 3.0f)
		{
			Engine::Get().SetNewLevel(new TitleLevel());
		}
		return;
	}

	// 키 입력을 하면 시각화 모드 On/ Off
	if (Input::Get().GetKeyDown('G'))
	{
		isShowQuadTree = !isShowQuadTree;
	}

	// 이전에 그리고 남은 트기가 있을 경우 지움
	if (quadTree)
	{
		delete quadTree;
		quadTree = nullptr;
	}

	// 쿼드 트리 생성 및 갱신
	// 화면 전체 불러오기
	Bounds screenBounds(0, 0, Engine::Get().GetWidth(), Engine::Get().GetHeight());
	quadTree = new QuadTree(screenBounds);

	// 레벨에 있는 모든 액터들을 쿼드 트리에 넣기
	for (Actor* actor : actors)
	{
		if (actor->IsActive())
		{
			// Background 타입이 아닌 경우에만 트리 삽입
			if (!actor->IsTypeOf<Background>())
			{
				quadTree->Insert(actor);
			}
		}
	}

	// 충돌 판정 처리.
	ProcessCollisionPlayerBulletAndEnemy();
	ProcessCollisionPlayerAndEnemyBullet();
	ProcessCollisionPlayerBulletAndObstacle();
	ProcessCollisionPlayerAndObstacle();
	ProcessCollisionPlayerAndItem();
	ProcessCollisionPlayerBulletAndEnemyBullet();
}

void GameLevel::Draw()
{
	super::Draw();

	// 시각화가 On이라면 트리 경계선 '+'을 그린다.
	if (isShowQuadTree && quadTree)
	{
		quadTree->DebugDraw();
		Renderer::Get().Submit("QUADTREE DEBUG : ON", Vector2(1, 2), Color::Green);
	}

	if (isPlayerDead)
	{

		//화면의 중앙에 메시지 출력		 
		int x = Engine::Get().GetWidth()/2-5;
		int y = Engine::Get().GetHeight()/2;
		Renderer::Get().Submit("!GAME OVER!", Vector2(x, y));

		
		
		// 플레이어 죽음 메시지 Renderer에 제출.
		Renderer::Get().Submit("Wait 3 Seconds...", Vector2(x-2, y+2));

		// 점수 보여주기.
		//ShowScore();

		// 화면에 바로 표시.
		//Renderer::Get().PresentImmediately();

		// 프로그램 정지.
		//Sleep(2000);

		// 게임 종료.
		//Engine::Get().QuitEngine();
	}

	// 점수 보여주기.
	ShowScore();
}

void GameLevel::KillAllEemies()
{
	for (Actor* actor : actors)
	{
		if (actor->IsTypeOf<Enemy>() || actor->IsTypeOf<EnemyBullet>() || actor->IsTypeOf<Obstacle>())
		{
			actor->Destroy();
			score += 2;
		}
	}
}

void GameLevel::PlayerShield()
{
}

void GameLevel::PlayerSpeedBuster()
{
}

void GameLevel::ProcessCollisionPlayerBulletAndEnemy()
{
	// 예외처리
	if (!quadTree)
	{
		return;
	}

	// 플레이어 총알만 모아둠
	std::vector<Actor*> bullets;
	for (Actor* const actor : actors)
	{
		if (actor->IsTypeOf<PlayerBullet>() && actor->IsActive())
		{
			bullets.emplace_back(actor);
		}
	}

	if (bullets.empty())
	{
		return;
	}

	// 쿼드 트리를 이용한 충돌 검사
	for (Actor* const bullet : bullets)
	{
		// 같은 구역에 있는 액터들만 쿼드 트리에게 요청
		std::vector<Actor*> nearActors = quadTree->Query(bullet);

		for (Actor* const nearActors : nearActors)
		{
			// 적과 충돌 검사
			if (nearActors->IsTypeOf <Enemy>())
			{
				//??
				Enemy* enemy = nearActors->As<Enemy>();

				if (bullet->TestIntersect(enemy))
				{
					enemy->OnDamaged();
					bullet->Destroy();

					// 점수 추가 및 무기 업데이트
					score += 1;
					if (!isPlayerDead)
					{
						Player::Get().UpdateWeaponByScore(score);
					}

					break; // 이 총알은 파괴되었으므로, 다음 총알 검사로 넘어감
				}
			}
		}
	}
}

void GameLevel::ProcessCollisionPlayerAndEnemyBullet()
{
	if (!quadTree) return;

	// 1. 플레이어 찾기
	Player* player = nullptr;
	for (Actor* const actor : actors)
	{
		if (actor->IsTypeOf<Player>() && actor->IsActive())
		{
			player = actor->As<Player>();
			break;
		}
	}

	if (!player) return;

	// 2. 쿼드 트리로 플레이어 주변의 액터만 검색
	std::vector<Actor*> nearActors = quadTree->Query(player);

	for (Actor* const nearActor : nearActors)
	{
		if (nearActor->IsTypeOf<EnemyBullet>())
		{
			// 정밀 충돌 판정
			if (nearActor->TestIntersect(player))
			{
				score -= 5;
				nearActor->Destroy();

				if (score < 0)
				{
					score = 0;
					isPlayerDead = true;
					playerDeadPosition = player->GetPosition();
					player->OnDamaged();
				}
				else
				{
					player->UpdateWeaponByScore(score);
				}
				break; // 한 프레임에 한 번만 피격
			}
		}
	}
}

void GameLevel::ProcessCollisionPlayerBulletAndObstacle()
{
	if (!quadTree) return;

	std::vector<Actor*> bullets;
	for (Actor* const actor : actors)
	{
		if (actor->IsTypeOf<PlayerBullet>() && actor->IsActive())
		{
			bullets.emplace_back(actor);
		}
	}

	if (bullets.empty()) return;

	for (Actor* const bullet : bullets)
	{
		// 총알 주변 액터 검색
		std::vector<Actor*> nearActors = quadTree->Query(bullet);

		for (Actor* const nearActor : nearActors)
		{
			if (nearActor->IsTypeOf<Obstacle>())
			{
				Obstacle* obstacle = nearActor->As<Obstacle>();

				if (bullet->TestIntersect(obstacle))
				{
					obstacle->TakeDamaged();
					bullet->Destroy();

					score += 1;
					if (!isPlayerDead)
					{
						Player::Get().UpdateWeaponByScore(score);
					}
					break; // 총알 파괴, 다음 총알로
				}
			}
		}
	}
}

void GameLevel::ProcessCollisionPlayerAndObstacle()
{
	if (!quadTree) return;

	Player* player = nullptr;
	for (Actor* const actor : actors)
	{
		if (actor->IsTypeOf<Player>() && actor->IsActive())
		{
			player = actor->As<Player>();
			break;
		}
	}

	if (!player) return;

	// 플레이어 주변 액터 검색
	std::vector<Actor*> nearActors = quadTree->Query(player);

	for (Actor* const nearActor : nearActors)
	{
		if (nearActor->IsTypeOf<Obstacle>())
		{
			if (nearActor->TestIntersect(player))
			{
				score -= 5;
				nearActor->Destroy(); // 장애물 파괴

				if (score < 0)
				{
					score = 0;
					isPlayerDead = true;
					playerDeadPosition = player->GetPosition();
					player->OnDamaged();
				}
				else
				{
					player->UpdateWeaponByScore(score);
				}
				break;
			}
		}
	}
}

void GameLevel::ProcessCollisionPlayerAndItem()
{

	if (!quadTree) return;

	Player* player = nullptr;
	for (Actor* const actor : actors)
	{
		if (actor->IsTypeOf<Player>() && actor->IsActive())
		{
			player = actor->As<Player>();
			break;
		}
	}

	if (!player) return;

	// 플레이어 주변 액터 검색
	std::vector<Actor*> nearActors = quadTree->Query(player);

	for (Actor* const nearActor : nearActors)
	{
		if (nearActor->IsTypeOf<Item>())
		{
			Item* item = nearActor->As<Item>();
			if (player->TestIntersect(item))
			{
				item->TakeDamaged(); // 아이템 획득 처리
				coin += 5;
				// 플레이어가 여러 아이템을 동시에 먹을 수도 있으므로 break 생략
			}
		}
	}
}

void GameLevel::ProcessCollisionPlayerBulletAndEnemyBullet()
{
	if (!quadTree) return;

	std::vector<Actor*> pBullets;
	for (Actor* const actor : actors)
	{
		if (actor->IsTypeOf<PlayerBullet>() && actor->IsActive())
		{
			pBullets.emplace_back(actor);
		}
	}

	if (pBullets.empty()) return;

	for (Actor* const pb : pBullets)
	{
		// 내 총알 주변 액터 검색
		std::vector<Actor*> nearActors = quadTree->Query(pb);

		for (Actor* const nearActor : nearActors)
		{
			if (nearActor->IsTypeOf<EnemyBullet>())
			{
				if (pb->TestIntersect(nearActor))
				{
					// 둘 다 파괴 (상쇄)
					pb->Destroy();
					nearActor->Destroy();
					break;
				}
			}
		}
	}
	
}

void GameLevel::ShowScore()
{
	sprintf_s(scoreString, 128, "Score: %d             Coin: %d", score, coin);
	Renderer::Get().Submit(scoreString,	Vector2(1, Engine::Get().GetHeight() - 1));

	// --- 디버깅용 속도 업 상태 표시 추가 ---
	if (!isPlayerDead && Player::Get().IsSpeedBuffActive())
	{
		// 화면 좌측 상단이나 적절한 위치에 버프 상태 출력
		Renderer::Get().Submit("!!! SPEED BOOST ACTIVE !!!", Vector2(1, 1));
	}
}

