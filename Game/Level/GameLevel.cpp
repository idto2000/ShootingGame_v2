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


#include <iostream>

//정적 변수 초기화
GameLevel* GameLevel::instance = nullptr;

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

	// 쿼드 트리 갱신
	

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
	// 플레이어 탄약과 적 액터 필터링.
	std::vector<Actor*> bullets;
	std::vector<Enemy*> enemies;

	// 액터 필터링.
	for (Actor* const actor : actors)
	{
		if (actor->IsTypeOf<PlayerBullet>())
		{
			bullets.emplace_back(actor);
			continue;
		}

		if (actor->IsTypeOf<Enemy>())
		{
			enemies.emplace_back(actor->As<Enemy>());
		}
	}

	// 판정 안해도 되는지 확인.
	if (bullets.size() == 0 || enemies.size() == 0)
	{
		return;
	}

	// 충돌 판정.
	for (Actor* const bullet : bullets)
	{
		for (Enemy* const enemy : enemies)
		{
			// AABB 겹침 판정.
			if (bullet->TestIntersect(enemy))
			{
				enemy->OnDamaged();
				bullet->Destroy();

				// 점수 추가.
				score += 1;
				if (!isPlayerDead)
				{
					//점수가 올랐으니 무기 상태 체크
					Player::Get().UpdateWeaponByScore(score);
				}
					continue;
			}
		}
	}
}

void GameLevel::ProcessCollisionPlayerAndEnemyBullet()
{
	// 액터 필터링을 위한 변수.
	Player* player = nullptr;
	std::vector<Actor*> bullets;
	
	// 액터 필터링.
	for (Actor* const actor : actors)
	{
		if (!player && actor->IsTypeOf<Player>())
		{
			player = actor->As<Player>();
			continue;
		}

		if (actor->IsTypeOf<EnemyBullet>())
		{
			bullets.emplace_back(actor);
		}
	}

	// 판정 처리 안해도 되는지 확인.
	if (bullets.size() == 0 || !player)
	{
		return;
	}

	// 충돌 판정.
	for (Actor* const bullet : bullets)
	{
		if (bullet->TestIntersect(player))
		{
			score -= 5;
			bullet->Destroy();

			if (score < 0)
			{
				score = 0;

				// 플레이어 죽음 설정.
				isPlayerDead = true;

				// 죽은 위치 저장.
				playerDeadPosition = player->GetPosition();

				// 액터 제거 처리.
				player->OnDamaged();
				bullet->Destroy();
			}
			else
			{
				//점수가 깎였으니 무기 상태 체크
				player->UpdateWeaponByScore(score);
			}
		
			//한 프레임에 한 번만 피격 처리 
			break;
		}
	}
}

void GameLevel::ProcessCollisionPlayerBulletAndObstacle()
{

	// 플레이어 탄약과 장애물 액터 필터링.
	std::vector<Actor*> bullets;
	std::vector<Obstacle*> obstacles;

	// 액터 필터링.
	for(Actor* const actor : actors)
	{
		if (actor->IsTypeOf<PlayerBullet>())
		{
			bullets.emplace_back(actor);
			continue;
		}

		if (actor->IsTypeOf<Obstacle>())
		{
			obstacles.emplace_back(actor->As<Obstacle>());
		}
	}

	// 판정 안해도 되는지 확인.
	if(bullets.size() == 0 || obstacles.size() == 0)
	{
		return;
	}

	// 충돌 판정.
	for(Actor* const bullet : bullets)
	{
		for(Obstacle* const obstacle : obstacles)
		{
			// AABB 겹침 판정.
			if(bullet->TestIntersect(obstacle))
			{
				//장애물 데미지 함수 호출
				obstacle->TakeDamaged();
				bullet->Destroy();

				// 점수 추가.
				score += 1;

				if(!isPlayerDead)
				{
					Player::Get().UpdateWeaponByScore(score);
				}
				continue;
			}
		}
	}
}

void GameLevel::ProcessCollisionPlayerAndObstacle()
{
	// 액터 필터링을 위한 변수.
	Player* player = nullptr;
	std::vector<Actor*> obstacles;

	// 액터 필터링.
	for (Actor* const actor : actors)
	{
		if (!player && actor->IsTypeOf<Player>())
		{
			player = actor->As<Player>();
			continue;
		}

		if (actor->IsTypeOf<Obstacle>())
		{
			obstacles.emplace_back(actor);
		}
	}

	// 판정 처리 안해도 되는지 확인.
	if (obstacles.size() == 0 || !player)
	{
		return;
	}

	// 충돌 판정.
	for (Actor* const obstacle : obstacles)
	{
		if (obstacle->TestIntersect(player))
		{
			// 장애물과 부딪히면 점수 감소
			score -= 5;

			// 장애물 파괴
			obstacle->Destroy();

			if (score < 0)
			{
				score = 0;

				// 플레이어 죽음 설정.
				isPlayerDead = true;

				// 죽은 위치 저장.
				playerDeadPosition = player->GetPosition();

				// 플레이어 제거는 Tick에서 처리하므로 주석 유지
				player->OnDamaged();
			}
			else
			{
				// 점수가 깎였으니 무기 상태 체크
				player->UpdateWeaponByScore(score);
			}

			// 한 프레임에 한 번만 충돌 처리
			break;
		}
	}
}

void GameLevel::ProcessCollisionPlayerAndItem()
{

	//Player* player = nullptr;
	// 플레이어 탄약과 장애물 액터 필터링.
	std::vector<Actor*> players;
	std::vector<Item*> items;

	// 액터 필터링.
	for (Actor* const actor : actors)
	{
		if (actor->IsTypeOf<Player>())
		{
			players.emplace_back(actor);
			continue;
		}

		if (actor->IsTypeOf<Item>())
		{
			items.emplace_back(actor->As<Item>());
		}
	}

	// 판정 안해도 되는지 확인.
	if (players.size() == 0 || items.size() == 0)
	{
		return;
	}

	// 충돌 판정.
	for (Actor* const player : players)
	{
		for (Item* const item : items)
		{
			// AABB 겹침 판정.
			if (player->TestIntersect(item))
			{
				//아이템  충돌 함수 호출
				item->TakeDamaged();
				
				// 점수 추가.
				coin += 5;
				continue;
			}
		}
	}
}

void GameLevel::ProcessCollisionPlayerBulletAndEnemyBullet()
{
	// 플레이어 탄약과 적 탄약 액터 필터링.
	std::vector<Actor*> pBullets;
	std::vector<Actor*> eBullets;

	// 현재 레벨의 모든 액터를 순회하며 필터링.
	for (Actor* const actor : actors)
	{
		if (!actor->IsActive())
		{
			continue;
		}
		if (actor->IsTypeOf<PlayerBullet>())
		{
			// 플레이어 총알 저장
			pBullets.emplace_back(actor);
		}
		else if (actor->IsTypeOf<EnemyBullet>())
		{
			// 적 총알 저장
			eBullets.emplace_back(actor);
		}
	}

	// 한쪽이라도 없으면 종료
	if (pBullets.empty() || eBullets.empty())
	{
		return;
	}

	// AABB 충돌 판정
	for (Actor* const pb : pBullets)
	{
		for (Actor* const eb : eBullets)
		{
			// 플레이어 총알과 적 총알이 겹치는지 확인
			if (pb->TestIntersect(eb))
			{
				pb->Destroy();
				eb->Destroy();

				break;
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

