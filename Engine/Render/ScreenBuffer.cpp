#include "ScreenBuffer.h"
#include <iostream>
#include <Windows.h> // SMALL_RECT, SetConsoleWindowInfo 사용을 위해 필수

namespace Wanted
{
	ScreenBuffer::ScreenBuffer(const Vector2& screenSize)
		: screenSize(screenSize)
	{
		// Console Output 생성.
		buffer = CreateConsoleScreenBuffer(
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			nullptr,
			CONSOLE_TEXTMODE_BUFFER,
			nullptr
		);

		// 예외 처리.
		if (buffer == INVALID_HANDLE_VALUE)
		{
			MessageBoxA(
				nullptr,
				"ScreenBuffer - Failed to create buffer.",
				"Buffer creation error",
				MB_OK
			);
			__debugbreak();
		}

		CONSOLE_FONT_INFOEX cfi;
		cfi.cbSize = sizeof(cfi);
		cfi.nFont = 0;
		cfi.dwFontSize.X = 4; // 글자 가로 크기 4픽셀
		cfi.dwFontSize.Y = 6; // 글자 세로 크기 4픽셀
		cfi.FontFamily = FF_DONTCARE;
		cfi.FontWeight = FW_NORMAL;
		wcscpy_s(cfi.FaceName, L"Terminal");

		// 기본 도화지가 아닌, 'buffer'에 설정.
		SetCurrentConsoleFontEx(buffer, FALSE, &cfi);

		// [수정된 핵심 로직] ---------------------------------------------
		// 1. 윈도우 크기를 먼저 최소(1x1)로 줄여서 버퍼 크기 제약을 피함
		SMALL_RECT minimalRect = { 0, 0, 1, 1 };
		SetConsoleWindowInfo(buffer, TRUE, &minimalRect);

		// 2. 버퍼(Screen Buffer) 크기를 목표 크기로 설정
		COORD newSize;
		newSize.X = static_cast<short>(screenSize.x);
		newSize.Y = static_cast<short>(screenSize.y);

		if (!SetConsoleScreenBufferSize(buffer, newSize))
		{
			// 실패 시 에러 메시지 출력 (디버깅용)
			std::cerr << "Failed to set console buffer size. Error: " << GetLastError() << "\n";
			__debugbreak();
		}

		// 3. 윈도우(Window Info) 크기를 목표 크기로 설정
		SMALL_RECT targetRect;
		targetRect.Left = 0;
		targetRect.Top = 0;
		targetRect.Right = static_cast<short>(screenSize.x - 1);
		targetRect.Bottom = static_cast<short>(screenSize.y - 1);

		if (!SetConsoleWindowInfo(buffer, TRUE, &targetRect))
		{
			std::cerr << "Failed to set console window info. Error: " << GetLastError() << "\n";
			__debugbreak();
		}
		// ----------------------------------------------------------------

		// 커서 끄기.
		CONSOLE_CURSOR_INFO info;
		GetConsoleCursorInfo(buffer, &info);

		// 끄도록 설정.
		info.bVisible = false;
		SetConsoleCursorInfo(buffer, &info);
	}

	ScreenBuffer::~ScreenBuffer()
	{
		// 버퍼 해제.
		if (buffer)
		{
			CloseHandle(buffer);
		}
	}

	void ScreenBuffer::Clear()
	{
		// 실제로 화면을 지우고 난 뒤에 
		// 몇 글자를 썼는지 반환 받는데 사용.
		DWORD writtenCount = 0;

		// 콘솔 버퍼에 있는 화면 지우기.
		FillConsoleOutputCharacterA(
			buffer,
			' ',
			screenSize.x * screenSize.y,
			Vector2::Zero,
			&writtenCount
		);
	}

	void ScreenBuffer::Draw(CHAR_INFO* charInfo)
	{
		// 설정할 버퍼의 크기
		SMALL_RECT writeRegion = {};
		writeRegion.Left = 0;
		writeRegion.Top = 0;
		writeRegion.Right = static_cast<short>(screenSize.x - 1);
		writeRegion.Bottom = static_cast<short>(screenSize.y - 1);

		// 버퍼에 전달 받은 글자 배열 설정.
		WriteConsoleOutputA(
			buffer,
			charInfo,
			screenSize,
			Vector2::Zero,
			&writeRegion
		);
	}
}