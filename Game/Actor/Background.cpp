#include "Background.h"
#include "Engine/Engine.h"

Background::Background(int x, int y, float speed, const char* image, Color color)
	: Actor(image), moveSpeed(speed)
{
	SetPosition(Vector2(x, y));
	yReal = (float)y;

	//색상 적용
	this->color = color;
}

Background::~Background()
{
}

void Background::Tick(float deltaTime)
{
	super::Tick(deltaTime);

    yReal += moveSpeed * deltaTime;

    // 2. 화면 밖으로 나갔는지 체크 (화면 높이보다 커지면)
    if (yReal >= Engine::Get().GetHeight())
    {
        // 다시 화면 맨 위(0)로 보냄 -> 무한 스크롤 효과
        yReal = 0.0f;

        // (선택사항) X 위치를 랜덤으로 바꾸면 더 자연스러움
        // position.x = Util::Random(0, Engine::Get().GetWidth());
    }

    // 3. 정수 좌표로 변환하여 적용
    position.y = (int)yReal;
}

