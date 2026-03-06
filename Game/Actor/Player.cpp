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

#include<iostream>
#include<cmath>//Player의 움직임을 부드럽게 하기 위한 수학함수

// 전역 변수 초기화.
Player* Player::instance = nullptr;


Player::Player()
	: super("<=A=>", Vector2::Zero, Color::Green), orignalAcceleration(0.0f),
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
	//현재 연사가 아닌 경우 
	if(!isBursting)
	{
		mainTimer.Tick(deltaTime);
		if (mainTimer.IsTimeOut())
		{
			mainTimer.Reset();

			//발사시작
			isBursting = true;
			burstCountCurrent = 0;

			//점사 타이머 리셋
			burstTimer.Reset();
		}
	}
	else
	{
		burstTimer.Tick(deltaTime);

		// 첫 발이거나, 타이머가 다 됐을 때 발사
		if (burstCountCurrent == 0 || burstTimer.IsTimeOut())
		{
			burstTimer.Reset();

			// 현재 키 입력 상태 확인
			bool up = Input::Get().GetKey(VK_UP);
			bool down = Input::Get().GetKey(VK_DOWN);
			bool left = Input::Get().GetKey(VK_LEFT);
			bool right = Input::Get().GetKey(VK_RIGHT);
			
			// 키 입력에 따른 발사 각도(콘솔 좌표계 기준 270도 위쪽)
			float fireAngle = 270.0f;

			if (up && right)
			{
				fireAngle = 315.0f;
			}
			else if (up && left)
			{
				fireAngle = 225.0f;
			}
			else if (down && right)
			{
				fireAngle = 45.0f;
			}
			else if (down && left)
			{
				fireAngle = 135.0f;
			}
			else if (up)
			{
				fireAngle = 270.0f;
			}
			else if (down)
			{
				fireAngle = 90.0f;
			}
			else if (left)
			{
				fireAngle = 180.0f;
			}
			else if (right)
			{
				fireAngle = 0.0f;
			}
			else
			{
				fireAngle = 270.0f;
			}

			//총알 생성 위치 조정(플레이어 중앙)
			Vector2 bulletPos(position.x + (width / 2), position.y);

			//현재 총알 생성(각도/ 속도)
			GetOwner()->AddNewActor(new PlayerBullet(bulletPos, fireAngle, bulletSpeed));

			burstCountCurrent++;

			if (burstCountCurrent >= burstCountTotal)
			{
				isBursting = false;
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
