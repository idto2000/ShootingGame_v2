#include "EnemySpawner.h"
#include "Actor/Enemy.h"
#include "Util/Util.h"
#include "Level/Level.h"

// 적 생성할 때 사용할 글자 값.
// 여기에서 static은 private.
static const char* enemyType[] =
{
	";:^:;",
	"zZwZz",
	"oO@Oo",
	"<-=->",
	")qOp(",
};

// 각 EnemyType에 1:1로 대응하여 공격 패턴 배열
static Enemy::AttackPattern enemyPatterns[] =
{
	Enemy::AttackPattern::Single, // 0번 외형은 단발
	Enemy::AttackPattern::Single, // 1번 외형은 단발
	Enemy::AttackPattern::Radial, // 2번 외형(보스 느낌)은 360도 방사형
	Enemy::AttackPattern::Triple, // 3번 외형은 3발 부채꼴
	Enemy::AttackPattern::Triple  // 4번 외형은 3발 부채꼴
};

EnemySpawner::EnemySpawner()
{
	// 적 생성 타이머 설정.
	timer.SetTargetTime(Util::RandomRange(2.0f, 3.0f));
}

void EnemySpawner::Tick(float deltaTime)
{
	super::Tick(deltaTime);

	SpawnEnemy(deltaTime);
}

void EnemySpawner::SpawnEnemy(float deltaTime)
{
	// 타이머 업데이트.
	timer.Tick(deltaTime);

	// 경과 시간 확인.
	if (!timer.IsTimeOut())
	{
		return;
	}

	// 타이머 초기화.
	timer.Reset();

	// 적생성.
	// 적 개수 파악.
	static int length 
		= sizeof(enemyType) / sizeof(enemyType[0]);

	// 랜덤 인덱스.
	int index = Util::Random(0, length - 1);

	// 생성 y 위치.
	int yPosition = Util::Random(1, 100);


	// 패턴 전달
	GetOwner()->AddNewActor(new Enemy(enemyType[index], yPosition,
		enemyPatterns[index]));
}
