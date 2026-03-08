#pragma once

#include "Bounds.h"
#include "Common/Common.h"

#include <vector>


namespace Wanted
{
	// 전방선언: Actor클래스의 세부 내용은 몰라도 포인터는 사용할 수 있게 한다.
	class Actor;
	
	// 메모리 삭제 헬퍼 함수 
	template<typename T>
	void SafeDelete(T*& t)
	{
		if (t)
		{
			delete t;
			t = nullptr;
		}
	}
	// 영역 구분을 위한 4분면과 경계에서 걸치는 판단에 필요한 인덱스
	enum class NodeIndex
	{
		TopLeft, TopRight,
		BottomLeft,	BottomRight,
		Straddling,   // 두개 이상의 영역에 걸쳐 있음
		QutOfArea     // 현재 노드의 영역을 벗어남
	};

	class WANTED_API QuadNode
	{
	public:

		// 현재 노드가 담당할 영역과 깊이
		QuadNode(const Bounds& bounds, int depth = 0);
		~QuadNode();

		// 액터를 쿼드트리에 삽입(테이터 타입을 Actor*로 적용하여 상속 문제 해.
		void Insert(Actor* actor);

		// 특정 영역안에 있는 액터들을 모두 찾아 리스트에 담아줌.
		// 매 프레임 액터들이 움직이므로, 매번 트리를 새로 만들기 위해 사용
		void Query(const Bounds& range, std::vector<QuadNode*>& possibleNode);

		// 현재 노드와 모든 자식 노드에 들어 있는 액터 목록을 비운다.
		// 매 프레임 액터들이 움직이므로, 매번 트리를 새로 만드릭 위해 사용
		void Clear();

		// Getter 상태 확인 함수
		inline const Bounds& GetBounds() const { return bounds; }
		inline const std::vector<Actor*>& GetActor() const { return actors; }

		// [검증수행] 현재 녿와 모든 자식 노드에 들어 있는 액터 총합 계산
		int GetTotalActorCount() const;

		// [검증수행] 생성된 총 노드 수 계산
		int GetTotalNodeCount() const;

		// 자식 노드 접근
		inline QuadNode* TopLeft() const { return topLeft; }
		inline QuadNode* TopRight() const { return topRight; }
		inline QuadNode* BottomLeft() const { return bottomLeft; }
		inline QuadNode* BottomRight() const { return bottomRight; }

	public:

		//[시각화 작업] 콘솔 화면에 현재 노드의 영역을 시각화
		void DebugDraw() const;

	private:

		// 현재 영역을 4개로 균등하게 분할하여 자식 노드 4개를 생성한다.
		bool Subdivide();

		// 영역이 분할됐는지 확인
		bool IsDivide() const;

		// 액터의 영역이 4분면 중 어디에 완벽히 포함되는지 확인
		NodeIndex TestRegion(const Bounds& Bounds);
		
		// 특정 영역이 걸쳐 있는 모든 4분면 인텍스를 반환
		std::vector<NodeIndex> GetQuads(const Bounds& bounds);

		// 생성된 4개의 자식 노드 메모리를 해제한다.
		void ClearChildren();

	private:

		// 현재 노드 깊이
		int depth = 0;
		
		// 현재 노드가 담당하는 사각형 영역 정보
		Bounds bounds;

		// 현재 노드에 포함된 액터 목록
		std::vector<Actor*> actors;

		// 4개로 쪼개진 자식 노드들의 포인터.
		QuadNode* topLeft = nullptr;
		QuadNode* topRight = nullptr;
		QuadNode* bottomLeft = nullptr;
		QuadNode* bottomRight = nullptr;
	};
}


