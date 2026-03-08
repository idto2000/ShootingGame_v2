#pragma once

#include "QuadNode.h"
#include "Common/Common.h"

#include <vector>

namespace Wanted
{
	// 전방 선언(Actor 클래스 사용하기 위함)
	class Actor;

	class WANTED_API QuadTree
	{
	public:

		// Bounds를 받아 루드 노드를 생성 
		QuadTree(const Bounds& bounds);
		~QuadTree();

		// 액터 추가
		void Insert(Actor* actor);

		// 특정 액터를 기준으로 주변에 충돌 가능성이 있는 액터 목록을 반환합니다.
		std::vector<Actor*> Query(Actor* queryActor);

		// Getter
		inline QuadNode* GetRoot() const { return root; }

	public:

		//[검증 수행]
		int GetTotalActorCount();

		//[검증 수행]
		int GetTotoalNodeCount();

		// 쿼드트리 시각화
		void DebugDraw() const;
		
	private:

		// 트리의 최상단 노드
		QuadNode* root = nullptr;
	};
}