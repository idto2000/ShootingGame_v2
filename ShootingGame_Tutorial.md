# 콘솔 게임 엔진 제작 학습 노트

**머리말**

이 학습 노트의 목표는 단순한 코드 복사-붙여넣기가 아닌, 게임 엔진이 어떤 원리로 동작하는지, 각 코드가 왜 필요한지를 스스로 이해하며 자신만의 작은 게임 엔진을 완성하는 것입니다. C++의 기본 문법(클래스, 상속, 포인터 등)에 대한 사전 지식이 있다고 가정하고 진행하지만, 어려운 개념은 최대한 풀어서 설명하겠습니다.

---

### **Chapter 1: 프로젝트 기틀 다지기 및 첫 화면 출력**

가장 먼저, 우리의 코드를 담을 집을 짓는 과정입니다. Visual Studio에서 프로젝트를 설정하고, 콘솔 화면에 무언가를 그리기 위한 가장 기본적인 준비를 합니다.

#### **1.1. Visual Studio 프로젝트 생성 및 구성**

엔진의 핵심 기능과 실제 게임 로직을 분리하는 것은 좋은 설계의 첫걸음입니다. 우리는 `Engine` 프로젝트(DLL)와 `Game` 프로젝트(EXE)를 만들어 이 구조를 따를 것입니다.

1.  **새 솔루션 만들기:**
    *   Visual Studio 2022를 열고 `새 프로젝트 만들기`를 선택합니다.
    *   `빈 솔루션`을 검색하여 선택하고, `다음`을 누릅니다.
    *   솔루션 이름을 `ShootingGame`으로 지정하고 원하는 위치에 저장한 뒤 `만들기`를 클릭합니다.

2.  **Engine 프로젝트 (DLL) 생성:**
    *   오른쪽의 `솔루션 탐색기`에서 방금 만든 `솔루션 'ShootingGame'`을 마우스 오른쪽 버튼으로 클릭합니다.
    *   `추가` > `새 프로젝트`로 이동합니다.
    *   `동적 연결 라이브러리(DLL)`를 검색하여 선택하고 `다음`을 누릅니다.
    *   프로젝트 이름을 `Engine`으로 설정하고 `만들기`를 클릭합니다.

3.  **Game 프로젝트 (실행 파일) 생성:**
    *   다시 `솔루션 'ShootingGame'`을 마우스 오른쪽 버튼으로 클릭합니다.
    *   `추가` > `새 프로젝트`로 이동합니다.
    *   `콘솔 앱`을 검색하여 선택하고 `다음`을 누릅니다.
    *   프로젝트 이름을 `Game`으로 설정하고 `만들기`를 클릭합니다.

4.  **프로젝트 종속성 설정:**
    *   `Game` 프로젝트는 `Engine` 프로젝트의 기능을 가져다 써야 합니다. 이 관계를 "종속성"이라고 합니다.
    *   `솔루션 탐색기`에서 `Game` 프로젝트를 마우스 오른쪽 버튼으로 클릭하고 `빌드 종속성` > `프로젝트 종속성`으로 이동합니다.
    *   `다음에 종속:` 체크박스 목록에서 `Engine`을 체크하고 `확인`을 누릅니다.
    *   이제 `Game` 프로젝트를 빌드하기 전에 항상 최신 버전의 `Engine` 프로젝트가 먼저 빌드됩니다.

5.  **참조 설정:**
    *   `Game` 프로젝트에서 `Engine`의 코드를 사용하려면 참조를 추가해야 합니다.
    *   `Game` 프로젝트의 `참조`를 마우스 오른쪽 버튼으로 클릭하고 `참조 추가`를 선택합니다.
    *   `프로젝트` 탭에서 `Engine`을 체크하고 `확인`을 누릅니다.

6.  **시작 프로젝트 설정:**
    *   우리가 최종적으로 실행할 파일은 `Game.exe`입니다.
    *   `Game` 프로젝트를 마우스 오른쪽 버튼으로 클릭하고 `시작 프로젝트로 설정`을 선택합니다.

이제 `Engine`과 `Game`이라는 두 개의 분리된 공간이 마련되었습니다. 다음 단계부터는 `Engine` 프로젝트에 코드를 채워나가겠습니다.

#### **1.2. 화면에 점 찍기: `Vector2`와 `Renderer` 기초**

콘솔 화면의 모든 위치는 X, Y 좌표로 표현할 수 있습니다. 이를 다루기 위한 간단한 자료구조와 화면에 문자를 그려줄 `Renderer`의 기초를 만들어 봅시다.

##### **`Engine` 프로젝트에 `Vector2.h` 파일 추가**

```cpp
// Engine/Vector2.h
#pragma once

struct Vector2
{
    // --- 생성자 ---
    // 기본 생성자: (0, 0)으로 초기화
    Vector2()
        : X(0), Y(0)
    {
    }

    // 인자를 받는 생성자: 주어진 x, y 값으로 초기화
    Vector2(int x, int y)
        : X(x), Y(y)
    {
    }

    // --- 멤버 변수 ---
    int X;
    int Y;
};
```

##### **`Engine` 프로젝트에 `Renderer.h` 및 `Renderer.cpp` 파일 추가**

```cpp
// Engine/Renderer.h
#pragma once
#include "Vector2.h" // Vector2를 사용하기 위해 포함
#include <Windows.h> // Windows API 사용을 위해 포함

class Renderer
{
public:
    Renderer();
    ~Renderer();

    void Initialize();
    void Render();
    void Clear();
    void Draw(Vector2 Pos, char Image);

private:
    HANDLE ConsoleHandle;
    COORD CursorCoord;
    char ScreenBuffer[25][50];
};
```

```cpp
// Engine/Renderer.cpp
#include "Renderer.h"
#include <iostream>

Renderer::Renderer()
{
    ConsoleHandle = nullptr;
    CursorCoord = {0, 0};
}

Renderer::~Renderer()
{
}

void Renderer::Initialize()
{
    ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_CURSOR_INFO CursorInfo;
    CursorInfo.dwSize = 1;
    CursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(ConsoleHandle, &CursorInfo);

    Clear();
}

void Renderer::Render()
{
    SetConsoleCursorPosition(ConsoleHandle, CursorCoord);

    for (int y = 0; y < 25; ++y)
    {
        for (int x = 0; x < 50; ++x)
        {
            std::cout << ScreenBuffer[y][x];
        }
        std::cout << std::endl;
    }
}

void Renderer::Clear()
{
    for (int y = 0; y < 25; ++y)
    {
        for (int x = 0; x < 50; ++x)
        {
            ScreenBuffer[y][x] = ' ';
        }
    }
}

void Renderer::Draw(Vector2 Pos, char Image)
{
    if (Pos.X < 0 || Pos.X >= 50 || Pos.Y < 0 || Pos.Y >= 25)
    {
        return;
    }
    ScreenBuffer[Pos.Y][Pos.X] = Image;
}
```

#### **1.3. `Game` 프로젝트에서 테스트하기**

```cpp
// Game/Game.cpp
#include <iostream>
#include "Renderer.h" // Engine 프로젝트의 Renderer.h를 포함

int main()
{
    Renderer MyRenderer;
    MyRenderer.Initialize();
    MyRenderer.Draw(Vector2(25, 12), 'A');
    MyRenderer.Render();
    system("pause");
    return 0;
}
```
---

### **Chapter 2: 액터 모델 – 게임 오브젝트에 생명 불어넣기**

#### **2.1. 게임 오브젝트의 설계도: `Actor` 클래스**

##### **`Engine` 프로젝트에 `Actor.h` 및 `Actor.cpp` 파일 추가**

```cpp
// Engine/Actor.h
#pragma once
#include "Vector2.h"
#include <memory>

class Level;

class Actor
{
public:
    Actor();
    virtual ~Actor();

    virtual void BeginPlay();
    virtual void Tick(float DeltaTime);
    virtual void Draw();

    void Destroy();
    void SetLevel(Level* InLevel) { OwningLevel = InLevel; }
    Level* GetWorld() const { return OwningLevel; }

    Vector2 Pos;
    char Image;
    bool IsActive;
    bool HasBeganPlay;
    bool DestroyRequested;

protected:
    Level* OwningLevel;
};
```

```cpp
// Engine/Actor.cpp
#include "Actor.h"
#include <iostream>

Actor::Actor()
    : Pos(0, 0), Image(' '), IsActive(true), HasBeganPlay(false), DestroyRequested(false), OwningLevel(nullptr)
{
}

Actor::~Actor()
{
}

void Actor::BeginPlay()
{
    HasBeganPlay = true;
}

void Actor::Tick(float DeltaTime)
{
}

void Actor::Draw()
{
}

void Actor::Destroy()
{
    DestroyRequested = true;
    IsActive = false;
}
```

#### **2.2. 액터들의 무대: `Level` 클래스**

##### **`Engine` 프로젝트에 `Level.h` 및 `Level.cpp` 파일 추가**

```cpp
// Engine/Level.h
#pragma once
#include <vector>
#include <memory>
#include "Actor.h"
#include "Renderer.h"

class Level
{
public:
    Level();
    virtual ~Level();

    virtual void BeginPlay();
    virtual void Tick(float DeltaTime);
    virtual void Draw(Renderer* InRenderer);

    void AddNewActor(Actor* NewActor);
    void ProcessAddAndDestroyActors();

    const std::vector<std::unique_ptr<Actor>>& GetActors() const { return Actors; }

protected:
    std::vector<std::unique_ptr<Actor>> Actors;
    std::vector<std::unique_ptr<Actor>> AddActorQueue;
};
```

```cpp
// Engine/Level.cpp
#include "Level.h"
#include <algorithm>
#include <iostream>

Level::Level()
{
}

Level::~Level()
{
    Actors.clear();
    AddActorQueue.clear();
}

void Level::BeginPlay()
{
}

void Level::Tick(float DeltaTime)
{
    for (const auto& actor : Actors)
    {
        if (actor && actor->IsActive)
        {
            if (!actor->HasBeganPlay)
            {
                actor->BeginPlay();
            }
            actor->Tick(DeltaTime);
        }
    }
}

void Level::Draw(Renderer* InRenderer)
{
    for (const auto& actor : Actors)
    {
        if (actor && actor->IsActive)
        {
            InRenderer->Draw(actor->Pos, actor->Image);
        }
    }
}

void Level::AddNewActor(Actor* NewActor)
{
    if (NewActor)
    {
        NewActor->SetLevel(this);
        AddActorQueue.emplace_back(NewActor);
    }
}

void Level::ProcessAddAndDestroyActors()
{
    Actors.erase(std::remove_if(Actors.begin(), Actors.end(),
        [](const std::unique_ptr<Actor>& actor) {
            return actor->DestroyRequested;
        }),
        Actors.end());

    if (!AddActorQueue.empty())
    {
        for (auto& newActor : AddActorQueue)
        {
            Actors.push_back(std::move(newActor));
        }
        AddActorQueue.clear();
    }
}
```

#### **2.3. `Game` 프로젝트에서 `Actor`와 `Level` 테스트하기**

##### **`Game` 프로젝트에 `TestActor.h` 및 `TestActor.cpp` 파일 추가**
```cpp
// Game/TestActor.h
#pragma once
#include "Actor.h"

class TestActor : public Actor
{
public:
    TestActor();
    virtual ~TestActor() override;

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
};
```
```cpp
// Game/TestActor.cpp
#include "TestActor.h"
#include <iostream>

TestActor::TestActor()
{
    Image = '#';
    Pos = Vector2(1, 1);
}

TestActor::~TestActor()
{
}

void TestActor::BeginPlay()
{
    Actor::BeginPlay();
    std::cout << "TestActor BeginPlay!" << std::endl;
}

void TestActor::Tick(float DeltaTime)
{
    Actor::Tick(DeltaTime);
}
```

##### **`Game` 프로젝트의 `Game.cpp` 수정**
```cpp
// Game/Game.cpp
#include <iostream>
#include <Windows.h>
#include "Renderer.h"
#include "Level.h"
#include "TestActor.h"

int main()
{
    Renderer MyRenderer;
    MyRenderer.Initialize();

    Level MyLevel;

    MyLevel.AddNewActor(new TestActor());

    TestActor* anotherActor = new TestActor();
    anotherActor->Pos = Vector2(5, 5);
    anotherActor->Image = 'O';
    MyLevel.AddNewActor(anotherActor);

    bool IsRunning = true;
    while (IsRunning)
    {
        MyLevel.Tick(0.016f);
        MyLevel.ProcessAddAndDestroyActors();
        MyRenderer.Clear();
        MyLevel.Draw(&MyRenderer);
        MyRenderer.Render();
        Sleep(16);

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        {
            IsRunning = false;
        }
    }

    std::cout << "게임 종료." << std::endl;
    return 0;
}
```

---

### **Chapter 3: 엔진의 심장 – 게임 루프와 시간 제어**

#### **3.1. 모든 것을 지휘하는 `Engine` 클래스**

##### **`Engine` 프로젝트에 `Engine.h` 및 `Engine.cpp` 파일 추가**
```cpp
// Engine/Engine.h
#pragma once
#include <memory>
#include <chrono>

class Renderer;
class Level;

class Engine
{
public:
    static Engine* GetInstance()
    {
        if (Instance == nullptr)
        {
            Instance = new Engine();
        }
        return Instance;
    }

    void Run();
    void Stop();
    void SetLevel(Level* NewLevel);
    Renderer* GetRenderer() const { return MyRenderer.get(); }

private:
    Engine();
    ~Engine();

    void GameLoop();
    void Tick();

    static Engine* Instance;
    std::unique_ptr<Renderer> MyRenderer;
    std::unique_ptr<Level> CurrentLevel;
    bool bIsRunning;
    std::chrono::steady_clock::time_point LastTime;
    float DeltaTime;
};
```
```cpp
// Engine/Engine.cpp
#include "Engine.h"
#include "Renderer.h"
#include "Level.h"
#include "Input.h"
#include <Windows.h>

Engine* Engine::Instance = nullptr;

Engine::Engine()
    : bIsRunning(false), DeltaTime(0.0f)
{
    MyRenderer = std::make_unique<Renderer>();
}

Engine::~Engine()
{
}

void Engine::Run()
{
    bIsRunning = true;
    MyRenderer->Initialize();
    LastTime = std::chrono::steady_clock::now();
    GameLoop();
}

void Engine::Stop()
{
    bIsRunning = false;
}

void Engine::SetLevel(Level* NewLevel)
{
    if (NewLevel)
    {
        CurrentLevel.reset(NewLevel);
    }
}

void Engine::GameLoop()
{
    while (bIsRunning)
    {
        Tick();
        Input::Tick();
        
        if (CurrentLevel)
        {
            CurrentLevel->Tick(DeltaTime);
            CurrentLevel->ProcessAddAndDestroyActors();
        }

        MyRenderer->Clear();
        if (CurrentLevel)
        {
            CurrentLevel->Draw(MyRenderer.get());
        }
        MyRenderer->Render();
    }
}

void Engine::Tick()
{
    auto currentTime = std::chrono::steady_clock::now();
    std::chrono::duration<float> elapsedTime = currentTime - LastTime;
    DeltaTime = elapsedTime.count();
    LastTime = currentTime;

    if (DeltaTime < 1.0f / 200.0f)
    {
        Sleep(1);
    }
}
```

#### **3.2. 체계적인 입력 관리: `Input` 클래스**

##### **`Engine` 프로젝트에 `Input.h` 및 `Input.cpp` 파일 추가**
```cpp
// Engine/Input.h
#pragma once

class Input
{
public:
    static bool GetKey(int KeyCode);
    static bool GetKeyDown(int KeyCode);
    static bool GetKeyUp(int KeyCode);
    static void Tick();

private:
    static bool CurrentKeyStates[256];
    static bool PrevKeyStates[256];
};
```
```cpp
// Engine/Input.cpp
#include "Input.h"
#include <Windows.h>

bool Input::CurrentKeyStates[256] = { false, };
bool Input::PrevKeyStates[256] = { false, };

bool Input::GetKey(int KeyCode)
{
    return CurrentKeyStates[KeyCode];
}

bool Input::GetKeyDown(int KeyCode)
{
    return CurrentKeyStates[KeyCode] && !PrevKeyStates[KeyCode];
}

bool Input::GetKeyUp(int KeyCode)
{
    return !CurrentKeyStates[KeyCode] && PrevKeyStates[KeyCode];
}

void Input::Tick()
{
    for (int i = 0; i < 256; ++i)
    {
        PrevKeyStates[i] = CurrentKeyStates[i];
    }

    for (int i = 0; i < 256; ++i)
    {
        CurrentKeyStates[i] = (GetAsyncKeyState(i) & 0x8000) != 0;
    }
}
```

#### **3.3. `Engine`과 `Game` 최종 정리**

##### **`Game.cpp`의 `main` 함수 최종 수정**
```cpp
// Game/Game.cpp
#include "Engine.h"
#include "Level.h"
#include "TestActor.h"
#include "Input.h"

class GameLevel : public Level
{
public:
    virtual void BeginPlay() override
    {
        Level::BeginPlay();
        AddNewActor(new TestActor());
    }

    virtual void Tick(float DeltaTime) override
    {
        Level::Tick(DeltaTime);
        if (Input::GetKeyDown(VK_ESCAPE))
        {
            Engine::GetInstance()->Stop();
        }
    }
};

int main()
{
    Engine* MyEngine = Engine::GetInstance();
    MyEngine->SetLevel(new GameLevel());
    MyEngine->Run();
    return 0;
}
```

---
### **Chapter 4: 슈팅 게임 구현 – 엔진 위에 게임 만들기**

#### **4.1~4.3: `Player`, `PlayerBullet`, `Enemy`, `EnemyBullet` 클래스 구현**
(각 파일의 전체 코드는 부록을 참고하세요)

*   `Player`: 키보드 입력('W', 'A', 'S', 'D')으로 움직이고, 스페이스바로 `PlayerBullet`을 발사합니다.
*   `PlayerBullet`: 생성된 위치에서 위로('`|`') 날아갑니다. 화면을 벗어나면 스스로 파괴됩니다.
*   `Enemy`: 지정된 위치에서 아래로('`@`') 내려오며, 주기적으로 `EnemyBullet`을 발사합니다. 화면을 벗어나면 스스로 파괴됩니다.
*   `EnemyBullet`: 생성된 위치에서 아래로('`v`') 날아갑니다. 화면을 벗어나면 스스로 파괴됩니다.

#### **4.4. 충돌 처리 (Collision Detection)**
`Level::Tick()` 함수 내부에 `CheckCollisions()` 함수를 추가하고 호출합니다. 이 함수는 모든 액터 쌍의 위치가 같은지 검사하고, 같을 경우 두 액터를 모두 파괴합니다.

```cpp
// Engine/Level.cpp 의 Tick 함수에 추가
void Level::Tick(float DeltaTime)
{
    // ... 액터들 Tick() 호출 ...
    CheckCollisions();
}

// Engine/Level.cpp 에 추가
void Level::CheckCollisions()
{
    for (size_t i = 0; i < Actors.size(); ++i)
    {
        if (!Actors[i] || !Actors[i]->IsActive) continue;
        for (size_t j = i + 1; j < Actors.size(); ++j)
        {
            if (!Actors[j] || !Actors[j]->IsActive) continue;
            if (Actors[i]->Pos.X == Actors[j]->Pos.X && Actors[i]->Pos.Y == Actors[j]->Pos.Y)
            {
                Actors[i]->Destroy();
                Actors[j]->Destroy();
            }
        }
    }
}
```

#### **4.5. `GameLevel` 업데이트**
`GameLevel::Tick`에서 주기적으로 랜덤한 위치에 `Enemy`를 생성하고, `BeginPlay`에서 `Player`를 생성하도록 수정합니다.

```cpp
// Game/Game.cpp의 GameLevel 클래스 수정 예시
class GameLevel : public Level
{
    // ...
    virtual void BeginPlay() override
    {
        Level::BeginPlay();
        AddNewActor(new Player()); // 플레이어 생성
    }

    virtual void Tick(float DeltaTime) override
    {
        // ...
        // 일정 시간마다 랜덤 위치에 Enemy 생성하는 로직 추가
    }
    // ...
};
```

---
### **Chapter 5: 게임 완성도 높이기 – UI, 점수, 그리고 더 나은 충돌 처리**

#### **5.1. 지능적인 충돌 처리 시스템 구축**

1.  **`Actor.h`에 `EActorType` enum 추가:** `PLAYER`, `ENEMY` 등 액터 종류를 정의합니다.
2.  **`Actor.h`에 가상 함수 추가:**
    *   `virtual EActorType GetType() { return EActorType::NONE; }`
    *   `virtual void OnCollision(Actor* Other) {}`
3.  **파생 클래스에서 `GetType()` 오버라이드:** 각 `Player`, `Enemy` 등에서 자신의 타입을 반환하도록 재정의합니다.
4.  **`Level::CheckCollisions` 수정:** 두 액터의 위치가 같으면, 서로의 `OnCollision()` 함수를 호출해주도록 변경합니다. (파괴 로직 제거)
5.  **각 액터의 `OnCollision` 구현:** 각 액터가 `Other->GetType()`을 확인하여 충돌한 상대에 따라 다른 행동을 하도록 로직을 작성합니다.
    *   `Player::OnCollision`: 상대가 `ENEMY` 또는 `ENEMY_BULLET`이면 `Destroy()` 호출
    *   `Enemy::OnCollision`: 상대가 `PLAYER` 또는 `PLAYER_BULLET`이면 `Destroy()` 호출하고 점수 추가
    *   `PlayerBullet::OnCollision`: 상대가 `ENEMY`이면 `Destroy()` 호출

#### **5.2. 게임 상태 관리: 점수와 게임 오버**

*   **`GameLevel` 클래스에 변수 추가:** `int Score;`, `bool bIsGameOver;`
*   **`GameLevel`에 함수 추가:** `AddScore(int)`, `GameOver()`
*   `Enemy::OnCollision`에서 `GameLevel::AddScore()` 호출
*   `Player::OnCollision`에서 `GameLevel::GameOver()` 호출
*   `GameLevel::Tick`에서 `bIsGameOver`가 `true`이면 게임 로직(적 스폰 등)을 멈추고 'R'키로 재시작하는 로직 추가

#### **5.3. UI(사용자 인터페이스) 표시**

*   `GameLevel::Draw` 함수를 오버라이드합니다.
*   부모 `Level::Draw`를 먼저 호출하여 모든 액터를 그립니다.
*   `std::to_string`을 사용하여 점수를 문자열로 변환하고, `Renderer::Draw`를 반복 호출하여 화면 좌측 상단에 점수를 그립니다.
*   `bIsGameOver`가 `true`이면 "GAME OVER", "Press 'R' to Restart" 메시지를 화면 중앙에 그립니다.

---
축하합니다! 이제 기본적인 기능을 갖춘 C++ 콘솔 슈팅 게임 엔진과 게임이 완성되었습니다. 이 학습 노트를 기반으로 더 다양한 기능(사운드, 보스, 아이템 등)을 추가하며 자신만의 프로젝트를 발전시켜 나가시길 바랍니다.
