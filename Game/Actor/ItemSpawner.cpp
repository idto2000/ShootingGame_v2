#include "ItemSpawner.h"
#include "Item.h"
#include "Level/Level.h" 
#include "Engine/Engine.h"
#include "Util/Util.h"

// 네임스페이스 안에서 정의하거나, 함수 앞에 Wanted::를 붙여야 합니다.
// 가독성을 위해 namespace 블록을 사용하는 것이 좋습니다.
namespace Wanted
{
    static const char* ItemType[] =
    {
        "+", "-", "@", "&", "%"
    };

    ItemSpawner::ItemSpawner()
    {
        // 아이템 생성 타이머 설정 (2~3초 간격)
        timer.SetTargetTime(Util::RandomRange(5.0f, 5.0f));
    }

    void ItemSpawner::Tick(float deltaTime)
    {
        // 부모 클래스 Tick 호출 (super가 정의되어 있다고 가정)
        super::Tick(deltaTime);

        SpawnItem(deltaTime);
    }

    void ItemSpawner::SpawnItem(float deltaTime)
    {
        // 타이머 업데이트
        timer.Tick(deltaTime);

        // 시간이 되지 않았으면 리턴
        if (!timer.IsTimeOut())
        {
            return;
        }

        // 타이머 리셋
        timer.Reset();

        // 아이템 타입 배열 길이 계산
        static int length = sizeof(ItemType) / sizeof(ItemType[0]);

        // 랜덤 인덱스 선택
        int index = Util::Random(0, length - 1);

        // 화면 너비 가져오기 
        int screenWidth = Engine::Get().GetWidth();

        // 생성 X 위치 
        // 아이템의 폭(대략 1칸)을 고려해 -1을 해줍니다.
        int randomX = Util::Random(0, screenWidth - 1);

        // 하강 속도 랜덤 설정
        float randomSpeed = Util::RandomRange(1.0f, 5.5f);

        // 아이템 생성 요청
        // 중요: Item 생성자가 (이미지, x, y, 속도) 순서로 변경되었습니다.
        // 1. 이미지: ItemType 배열 사용
        // 2. X위치: randomX
        // 3. Y위치: 0 (화면 맨 위)
        // 4. 속도: randomSpeed
        GetOwner()->AddNewActor(new Item(ItemType[index], randomX, 0, randomSpeed, Color::Yellow));
    }
}