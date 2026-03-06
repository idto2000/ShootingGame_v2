#include "MouseTester.h"
#include "Core/Input.h"
#include "Render/Renderer.h"
#include "Engine/Engine.h"
#include <iostream>
#include "Actor/Player.h"

MouseTester::MouseTester()
	: super(" ", Vector2(Engine::Get().GetWidth() / 2, Engine::Get().GetHeight() - 1))
{
}

void MouseTester::Tick(float deltaTime)
{
	super::Tick(deltaTime);

	if (Input::Get().GetKeyDown(VK_LBUTTON))
	{
		// Player::Get() 대신 GetInstance() 사용
		Player* player = Player::GetInstance();

		// 플레이어가 있을 때만 실행
		if (player != nullptr)
		{
			// 마우스 입력 테스트.
			if (Input::Get().GetMouseButton(0))
			{
				Vector2 mousePosition = Input::Get().MousePosition();
				char buffer[256] = {};
				sprintf_s(
					buffer,
					256,
					"cursor: (%d, %d)",
					mousePosition.x,
					mousePosition.y
				);

				ChangeImage(buffer);
			}
		}
	}

}
