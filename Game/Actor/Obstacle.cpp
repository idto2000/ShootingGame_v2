#include "Obstacle.h"
#include "Engine/Engine.h"
#include "Level/Level.h"
#include "Util/Util.h"
#include "Actor/EnemyDestroyEffect.h"
#include "Actor/Player.h"
#include "Actor/Item.h"

Obstacle::Obstacle(const char* image, int xPosition, float speed, int hp) : super(image)
{
	// 화면의 가로 너비 측정.
	int screenWight = Engine::Get().GetWidth();

	int minX = 0;
	int maxX = screenWight - width - 1;

	//예외처리 
	if (maxX < minX)
	{
		maxX = minX;
	}

	//측정된 너비에서 랜덤하게 위치 선정
	int randomX = static_cast<int>(Util::Random(minX, maxX));

	// 변수 초기화
	this->moveSpeed = speed;
	
	//랜덤 체력 적용
	this->hp = hp;

	//추적 로직에서 사용하는 변수 초기 위치 저장
	this->xReal = static_cast<float>(randomX);
	this->yReal = 0.0f;
	
	//변수 동기화 유지
	this->xPosition = this->xReal;
	this->yPosition = this->yReal;

	//장애물 위치 설정
	SetPosition(Vector2(randomX, 0));
}

Obstacle::~Obstacle()
{
}

void Obstacle::Tick(float deltaTime)
{
	yReal += moveSpeed * deltaTime;

	Player* player = Player::GetInstance();
	
	if (player != nullptr)
	{
		//싱글톤을 사용해 플레이어 위치 정보 가져오기
		Vector2 pPos = Player::Get().GetPosition();

		if (pPos.x > xPosition)
		{
			xPosition += trackingSpeed * deltaTime;
		}
		else if (pPos.x < (int)xPosition)
		{
			xPosition -= trackingSpeed * deltaTime;
		}

		FollowPlayer(deltaTime, pPos);
	}

	// 화면 하단 이탈 검사
	if (position.y >= Engine::Get().GetHeight())
	{
		Destroy();
	}
	
}

void Obstacle::TakeDamaged(int damage)
{
	hp -= damage;
	if (hp <= 0)
	{
		// 파괴되기 전에 아이템 생성 시도
		TrySpawnItem();

		//파괴 이펙트
		if (GetOwner())
		{
			GetOwner()->AddNewActor(new EnemyDestroyEffect(position));
		}

		//자신 삭제
		Destroy();
	}
}

void Obstacle::OnCollisionWithEnemy()
{

}

void Obstacle::FollowPlayer(float deltaTime, Vector2 PlayerPos)
{
	bool shouldChase = true;

	//플레이어가 장애물보다 위에 있거나 
	//화면 너무 높은 곳에 있다면 추척을 포기한다. 
	if (PlayerPos.y < yReal)
	{
		shouldChase = false;
	}
	
	// 추적 로직 shouldchase가 ture일때만 계산
	if (shouldChase)
	{
		float diffx = (float)PlayerPos.x - xReal;
		float diffy = (float)PlayerPos.y - yReal;

		//플레이어의 거리 계산을 위한 제곱 계산
		float distanceSquared = (diffx * diffx) + (diffy * diffy);

		// 거리 범위 밖이면 추적 안함
		if (distanceSquared > (detection * detection))
		{
			shouldChase = false;
		}

		// 추적 상태라면 이동 계산 수행
		if (shouldChase)
		{
			if (diffx > 0.1f)
			{
				xReal += moveSpeed * deltaTime;
			}
			else if (diffx < -0.1f)
			{
				xReal -= moveSpeed * deltaTime;
			}

			if (diffy > 0.1f)
			{
				yReal += moveSpeed * deltaTime;
			}
			else if (diffy < -0.1f)
			{
				yReal -= moveSpeed * deltaTime;
			}
		}
	}
	
	//경계 검사 위쪽 끝 방지
	if (yReal < 0.0f)
	{
		yReal = 0.0f;
	}

	//경계 좌우 끝 방지
	if (xReal < 0.0f)
	{
		xReal = 0.0f;
	}
	else if (xReal + (float)width > (float)Engine::Get().GetWidth())
	{
		xReal = (float)Engine::Get().GetWidth() - (float)width;
	}

	xPosition = xReal;
	yPosition = yReal;

	SetPosition(Vector2(static_cast<int>(xReal), static_cast<int>(yReal)));
}

void Obstacle::TrySpawnItem()
{
	// 0~1사이 랜덤 실수 생섷
	float randomValue = Util::RandomRange(0.0f, 1.0f);

	if (randomValue <= itemDropChance)
	{
		if (GetOwner())
		{
			int idx = Util::Random(0, Item::ModelCount -1);

			Item* newItem = new Item(Item::ModelTypes[idx], static_cast<int>(xReal), static_cast<int>(yReal),
				5.0f, Color::Green);

			GetOwner()->AddNewActor(newItem);
		}
	}
		
}




 