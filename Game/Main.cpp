#include <iostream>
#include "Engine/Engine.h"
#include "Level/GameLevel.h"
#include "Level/TitleLevel.h"

using namespace Wanted;

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Engine engine;


	engine.SetNewLevel(new TitleLevel());
	engine.Run();
}