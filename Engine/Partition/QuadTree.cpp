#include "QuadNode.h"
#include "Actor/Actor.h"
#include "QuadTree.h"

namespace Wanted
{
	Wanted::QuadTree::QuadTree(const Bounds& bounds)
	{
		// 루트 노드 생성
		root = new QuadNode(bounds);

		// 쿼드 트리 강제 생성
		root->InitialSubdivde();
	}

	Wanted::QuadTree::~QuadTree()
	{
		SafeDelete(root);
	}

	void Wanted::QuadTree::Insert(Actor* actor)
	{
		// 예외 처리.
		if (!actor)
		{
			return;
		}

		//액터 트리에 추가(실제 작업은 QuadNode::Insert에서 수행함)
		root->Insert(actor);
	}

	std::vector<Actor*> Wanted::QuadTree::Query(Actor* queryActor)
	{
		// 예외 처리.
		if (!queryActor)
		{
			return { };
		}

		// 1. 질의 할 액터의 현재 영역 계산
		Bounds queryBounds((int)queryActor->GetPosition().x, (int)queryActor->GetPosition().y,
			queryActor->GetWidth(), 1);

		// 2. 겹칠 가능성이 있는 노드들 먼저 확인
		std::vector<QuadNode*> possibleNodes;
		root->Query(queryBounds, possibleNodes);

		// 3. 찾은 노드들 안에 있는 액터들을 순회하며 실제 겹침 판정
		std::vector<Actor*> intersects;
		for (QuadNode* const node : possibleNodes)
		{
			for (Actor* const actor : node->GetActor())
			{
				// 자신과의 충돌 제외
				if (actor == queryActor)
				{
					continue;
				}

				//실제 영역 겹침 확인
				Bounds actorBounds((int)actor->GetPosition().x, (int)actor->GetPosition().y,
					actor->GetWidth(), 1);
				if (actorBounds.Instersects(queryBounds))
				{
					intersects.emplace_back(actor);
				}
			}
		}

		// 결과값 최종 반환.
		return intersects;
	}

	int QuadTree::GetTotalActorCount()
	{
		if (root)
		{
			return root->GetTotalActorCount();
		}
		return 0;
	}

	int QuadTree::GetTotoalNodeCount()
	{
		if (root)
		{
			return root->GetTotalNodeCount();
		}
		return 0;
	}

	void QuadTree::DebugDraw() const
	{
		if (root)
		{
			root->DebugDraw();
		}
	}
}