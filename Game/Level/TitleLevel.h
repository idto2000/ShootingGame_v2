#pragma once
#include "Level/Level.h"

using namespace Wanted;

class TitleLevel : public Level
{
    RTTI_DECLARATIONS(TitleLevel, Level)

public:
    TitleLevel();
    virtual ~TitleLevel();

    enum class MenuOption
    {
        GameStart = 0,
        Credit,
        Exit,
        Count
    };

protected:
    virtual void Tick(float deltaTime) override;
    virtual void Draw() override;

private:
    void LoadTitleAsset();

private:
    int currentSelection = 0;
    bool isShowCredit = false;

    // [설정] 최대 줄 수와 가로 길이
    static const int MAX_TITLE_LINES = 40;
    static const int MAX_LINE_WIDTH = 256;

    // 타이틀 그림을 저장할 2차원 배열
    char titleImage[MAX_TITLE_LINES][MAX_LINE_WIDTH] = {};

    // 실제로 읽은 줄 수
    int titleLineCount = 0;

    // 메뉴 텍스트
    const char* menuText[3] = { "GAME START", "CREDIT", "EXIT" };
};