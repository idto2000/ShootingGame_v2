#include "TitleLevel.h"
#include "GameLevel.h"
#include "Engine/Engine.h"
#include "Core/Input.h"
#include "Render/Renderer.h"
#include <cstdio>
#include <cstring>
#include <algorithm> // max 함수 사용을 위해

TitleLevel::TitleLevel()
{
	// [중요] 배열 초기화
	memset(titleImage, 0, sizeof(titleImage));
	titleLineCount = 0;

	LoadTitleAsset();
}

TitleLevel::~TitleLevel()
{
}

void TitleLevel::LoadTitleAsset()
{
	FILE* file = nullptr;
	// 경로 확인: 프로젝트 실행 환경에 따라 ../Assets 또는 Assets
	fopen_s(&file, "../Assets/Title.txt", "r");

	if (file)
	{
		char buffer[256];
		while (titleLineCount < MAX_TITLE_LINES && fgets(buffer, sizeof(buffer), file))
		{
			// 줄바꿈 문자 제거
			size_t len = strlen(buffer);
			if (len > 0 && buffer[len - 1] == '\n')
			{
				buffer[len - 1] = '\0';
			}

			// 배열에 저장
			strcpy_s(titleImage[titleLineCount], MAX_LINE_WIDTH, buffer);
			titleLineCount++;
		}
		fclose(file);
	}
	else
	{
		strcpy_s(titleImage[0], "FILE NOT FOUND");
		strcpy_s(titleImage[1], "CHECK PATH: ../Assets/Title.txt");
		titleLineCount = 2;
	}
}

void TitleLevel::Tick(float deltaTime)
{
	Level::Tick(deltaTime);

	if (isShowCredit)
	{
		if (Input::Get().GetKeyDown(VK_ESCAPE) || Input::Get().GetKeyDown(VK_RETURN))
		{
			isShowCredit = false;
		}
		return;
	}

	if (Input::Get().GetKeyDown(VK_UP))
	{
		currentSelection--;
		if (currentSelection < 0) currentSelection = (int)MenuOption::Count - 1;
	}

	if (Input::Get().GetKeyDown(VK_DOWN))
	{
		currentSelection++;
		if (currentSelection >= (int)MenuOption::Count) currentSelection = 0;
	}

	if (Input::Get().GetKeyDown(VK_RETURN) || Input::Get().GetKeyDown(VK_SPACE))
	{
		switch ((MenuOption)currentSelection)
		{
		case MenuOption::GameStart:
			Engine::Get().SetNewLevel(new GameLevel());
			break;
		case MenuOption::Credit:
			isShowCredit = true;
			break;
		case MenuOption::Exit:
			Engine::Get().QuitEngine();
			break;
		}
	}
}

void TitleLevel::Draw()
{
	Level::Draw();

	int width = Engine::Get().GetWidth();
	int height = Engine::Get().GetHeight();

	// 1. 크레딧 화면
	if (isShowCredit)
	{
		Renderer::Get().Submit("=== CREDIT ===", Vector2(width / 2 - 7, height / 2 - 4));
		Renderer::Get().Submit("Developer : Jonn Hyung Kang", Vector2(width / 2 - 12, height / 2));
		Renderer::Get().Submit("[ Press Enter ]", Vector2(width / 2 - 8, height / 2 + 4));
		return;
	}

	// 2. 타이틀 화면 그리기 (위치 보정 로직 적용)

	// [핵심 1] 가장 긴 줄을 찾아서 전체 박스를 중앙 정렬함
	// (한 줄씩 따로 정렬하면 그림이 찌그러짐)
	int maxLen = 0;
	for (int i = 0; i < titleLineCount; ++i)
	{
		int len = (int)strlen(titleImage[i]);
		if (len > maxLen) maxLen = len;
	}

	// 전체 그림의 시작 X 좌표 (화면 중앙 - 그림 절반)
	int globalStartX = width / 2 - maxLen / 2;

	// 혹시 음수면 0으로
	if (globalStartX < 0) globalStartX = 0;

	int startY = 3; // 상단 여백

	for (int i = 0; i < titleLineCount; ++i)
	{
		char* line = titleImage[i];
		if (line[0] == '\0') continue;

		// 모든 줄을 똑같은 X 위치에서 시작해야 그림이 안 깨짐
		Renderer::Get().Submit(line, Vector2(globalStartX, startY + i));
	}

	// 3. 메뉴 그리기 (버퍼 재사용 문제 해결)
	int menuStartY = startY + titleLineCount + 3;

	// [핵심 2] 메뉴 개수(3개)만큼 버퍼를 따로 만들어야 함
	// [3][64] -> 메뉴 3개, 각 글자 최대 64자
	static char menuBuffers[3][64];

	for (int i = 0; i < 3; ++i)
	{
		// 각각 다른 버퍼(menuBuffers[i])에 글자를 씀
		if (i == currentSelection)
		{
			sprintf_s(menuBuffers[i], "> %s <", menuText[i]);
		}
		else
		{
			sprintf_s(menuBuffers[i], "  %s  ", menuText[i]);
		}

		// 중앙 정렬
		int len = (int)strlen(menuBuffers[i]);
		int textX = width / 2 - len / 2;

		// 렌더러에 제출
		Renderer::Get().Submit(menuBuffers[i], Vector2(textX, menuStartY + (i * 2)));
	}
}