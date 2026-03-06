#include "Item.h"
#include "Engine/Engine.h"
#include "Level/Level.h"
#include "Util/Util.h"

const char* Item::ModelTypes[] = { "+", "-", "@", "&", "%" };
const int Item::ModelCount = sizeof(Item::ModelTypes) / sizeof(Item::ModelTypes[0]);

Item::Item(const char* image, int x, int y, float speed, Color color)
	: super(image)
{
	this->moveSpeed = speed;

	this->xReal = static_cast<float>(x);
	this->yReal = static_cast<float>(y);
	
	//랜더링 좌표 설정
	SetPosition(Vector2(x, y));

	//칼라 적용
	this->color = color;
}

Item::~Item()
{
}

void Item::Tick(float deltaTime)
{
	// 아래로 이동
	yReal += moveSpeed * deltaTime;

	// 화면 하단 이탈 검사
	if (yReal >= Engine::Get().GetHeight())
	{
		Destroy();
	}

	// 좌표 동기화 (float -> int)
	SetPosition(Vector2(static_cast<int>(xReal), static_cast<int>(yReal)));
}

void Item::TakeDamaged()
{
	// 액터 제거.
	Destroy();
}
