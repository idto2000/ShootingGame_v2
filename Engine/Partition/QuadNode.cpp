#include "QuadNode.h"
#include "Actor/Actor.h"
#include "Render/Renderer.h"
#include "Math/Vector2.h"
#include "Engine/Engine.h"

namespace Wanted
{
	QuadNode::QuadNode(const Bounds& bounds, int depth)
		: bounds(bounds), depth(depth)
	{
	}
	QuadNode::~QuadNode()
	{
		Clear();
	}

	// 액터를 쿼드트리에 추가하는 함수
	void QuadNode::Insert(Actor* actor)
	{
		if (!actor)
		{
			return;
		}

		// 액터가 위치한 영역 계산
		Bounds actorBounds((int)actor->GetPosition().x, (int)actor->GetPosition().y,
			actor->GetWidth(), 1);

		// 4분면 중 어디에 들어가는지 확인
		NodeIndex result = TestRegion(actorBounds);

		// 두 개 이상 영역에 겹치는 경우에는 현재 노드에 추가.
		if (result == NodeIndex::Straddling)
		{
			actors.emplace_back(actor);
		}

		// 포함될 경우 자식 노드로 보냄
		else if (result != NodeIndex::QutOfArea)
		{
			// 아직 자식 노드가 없다면 분할
			if (Subdivide())
			{
				if (result == NodeIndex::TopLeft)
				{
					topLeft->Insert(actor);
				}
				else if (result == NodeIndex::TopRight)
				{
					topRight->Insert(actor);
				}
				else if (result == NodeIndex::BottomLeft)
				{
					bottomLeft->Insert(actor);
				}
				else if (result == NodeIndex::BottomRight)
				{
					bottomRight->Insert(actor);
				}
			}
			else
			{
				actors.emplace_back(actor);
			}
		}
	}

	// 특정 영역과 겹치는 모든 액터르 찾는다.
	void QuadNode::Query(const Bounds& range, std::vector<QuadNode*>& possiblNode)
	{
		// 현재 노드를 추가하고 이후 과정 진행
		possiblNode.emplace_back(this);

		if (!IsDivide())
		{
			return;
		}
		std::vector<NodeIndex> quads = GetQuads(range);

		// 영역과 겹치는 액터가 있는지 확인
		for (const NodeIndex& index : quads)
		{
			if (index == NodeIndex::TopLeft)
			{
				topLeft->Query(range, possiblNode);
			}
			else if (index == NodeIndex::TopRight)
			{
				topRight->Query(range, possiblNode);
			}
			else if (index == NodeIndex::BottomLeft)
			{
				bottomLeft->Query(range, possiblNode);
			}
			else if (index == NodeIndex::BottomRight)
			{
				bottomRight->Query(range, possiblNode);
			}
		}
	}

	void QuadNode::Clear()
	{
		actors.clear();

		if (IsDivide())
		{
			topLeft->Clear();
			topRight->Clear();
			bottomLeft->Clear();
			bottomRight->Clear();

			ClearChildren();
		}
	}
	int QuadNode::GetTotalActorCount() const
	{
		int count = (int)actors.size();

		// 자식이 있다면
		if (IsDivide())
		{
			count += topLeft->GetTotalActorCount();
			count += topRight->GetTotalActorCount();
			count += bottomLeft->GetTotalActorCount();
			count += bottomRight->GetTotalActorCount();
		}
		return count;

	}

	int QuadNode::GetTotalNodeCount() const
	{
		int count = 1;

		// 자기 자신
		if (IsDivide())
		{
			count += topLeft->GetTotalActorCount();
			count += topRight->GetTotalActorCount();
			count += bottomLeft->GetTotalActorCount();
			count += bottomRight->GetTotalActorCount();
		}
		return count;
	}

	void QuadNode::DebugDraw() const
	{
		// 1. 공통 좌표 계산
		int x = static_cast<int>(bounds.X());
		int y = static_cast<int>(bounds.Y());
		int w = static_cast<int>(bounds.Width());
		int h = static_cast<int>(bounds.Height());
		int sw = Engine::Get().GetWidth();
		int sh = Engine::Get().GetHeight();

		// ---------------------------------------------------------
		// [배경] 모든 노드의 모서리에 점(.)을 찍어 트리 구조를 표시
		// ---------------------------------------------------------
		// sortingOrder를 90으로 설정하여 배경보다는 위에, 격자보다는 아래에 위치
		Renderer::Get().Submit(".", Vector2((float)x, (float)y), Color::DarkGray, 90);

		// ---------------------------------------------------------
		// [자식 노드 호출]
		// ---------------------------------------------------------
		if (IsDivide())
		{
			if (topLeft) topLeft->DebugDraw();
			if (topRight) topRight->DebugDraw();
			if (bottomLeft) bottomLeft->DebugDraw();
			if (bottomRight) bottomRight->DebugDraw();

			// 자식이 있는 부모 노드는 여기서 종료 (실선은 말단 노드만 그림)
			return;
		}

		// ---------------------------------------------------------
		// [강조] 액터가 들어있는 '말단 방'만 실선 격자로 표시
		// ---------------------------------------------------------
		if (!actors.empty())
		{
			// 가로선 (-) - sortingOrder 100
			for (int i = 0; i < w; ++i)
			{
				int curX = x + i;
				if (curX >= 0 && curX < sw)
				{
					if (y >= 0 && y < sh)
						Renderer::Get().Submit("-", Vector2((float)curX, (float)y), Color::Green, 100);
					if (y + h - 1 >= 0 && y + h - 1 < sh)
						Renderer::Get().Submit("-", Vector2((float)curX, (float)(y + h - 1)), Color::Green, 100);
				}
			}

			// 세로선 (|) - sortingOrder 100
			for (int j = 0; j < h; ++j)
			{
				int curY = y + j;
				if (curY >= 0 && curY < sh)
				{
					if (x >= 0 && x < sw)
						Renderer::Get().Submit("|", Vector2((float)x, (float)curY), Color::Green, 100);
					if (x + w - 1 >= 0 && x + w - 1 < sw)
						Renderer::Get().Submit("|", Vector2((float)(x + w - 1), (float)curY), Color::Green, 100);
				}
			}

			// 네 모서리 (+) - sortingOrder 101 (가장 위)
			Renderer::Get().Submit("+", Vector2((float)x, (float)y), Color::White, 101);
			Renderer::Get().Submit("+", Vector2((float)(x + w - 1), (float)y), Color::White, 101);
			Renderer::Get().Submit("+", Vector2((float)x, (float)(y + h - 1)), Color::White, 101);
			Renderer::Get().Submit("+", Vector2((float)(x + w - 1), (float)(y + h - 1)), Color::White, 101);
		}
	}


	bool QuadNode::Subdivide()
	{
		// 설정한 최대 깊이에 도달하면 더 이상 쪼개지 않는다. 
		if (depth == 5)
		{
			return false;
		}

		// 이미 쪼개져 있다면 넘어 간다. 
		if (!IsDivide())
		{
			// 영역을 4개로 등분기 위한 계산
			int x = bounds.X();
			int y = bounds.Y();
			int halfWidth = bounds.Width() / 2;
			int halfHeight = bounds.Height() / 2;

			// 자식 노드 4개를 각각 영역에 맞춰 생성
			topLeft = new QuadNode(Bounds(x, y, halfWidth, halfHeight), depth + 1);
			topRight = new QuadNode(Bounds(x + halfWidth, y, halfWidth, halfHeight), depth + 1);
			bottomLeft = new QuadNode(Bounds(x, y + halfHeight, halfWidth, halfHeight), depth + 1);
			bottomRight = new QuadNode(Bounds(x + halfWidth, y + halfHeight, halfWidth,halfHeight), depth + 1);
		}
		return true;
	}

	bool QuadNode::IsDivide() const
	{
		return topLeft != nullptr;
	}

	// 액터가 어느 사분면에 속하는지 판정.
	NodeIndex QuadNode::TestRegion(const Bounds& Bounds)
	{
		std::vector<NodeIndex> quads = GetQuads(Bounds);

		if (quads.empty())
		{
			return NodeIndex::QutOfArea;
		}

		if (quads.size() == 1)
		{
			return quads[0];
		}

		return NodeIndex::Straddling;
	}

	// 특정 영역이 담당하는 모든 사분면 인덱스를 찾음
	std::vector<NodeIndex> QuadNode::GetQuads(const Bounds& bounds)
	{
		std::vector<NodeIndex> quads;

		// 현재 노드의 중앙 기준점을 계산해 전체 영역을 4개로 쪼갠다.
		int x = this->bounds.X();
		int y = this->bounds.Y();
		int halfWidth = this->bounds.Width() / 2;
		int halfHeight = this->bounds.Height() / 2;
		int centerX = x + halfWidth;
		int centerY = y + halfHeight;

		// 현재 노드가 어느 위치에 겹쳐 있는지 체크
		bool left = bounds.X() < centerX && bounds.MaxX() >= x;
		bool right = bounds.MaxX() >= centerX && bounds.X() < this->bounds.MaxX();
		bool top = bounds.Y() < centerY && bounds.MaxY() >= y;
		bool bottom = bounds.MaxY() >= centerY && bounds.Y() < this->bounds.MaxY();

		// 4분면 위치 설정
		if (top && left)
		{
			quads.emplace_back(NodeIndex::TopLeft);
		}
		if (top && right)
		{
			quads.emplace_back(NodeIndex::TopRight);
		}
		if (bottom && left)
		{
			quads.emplace_back(NodeIndex::BottomLeft);
		}
		if (bottom && right)
		{
			quads.emplace_back(NodeIndex::BottomRight);
		}

		return quads;
	}

	// 메모리 해제
	void QuadNode::ClearChildren()
	{
		if (IsDivide())
		{
			SafeDelete(topLeft);
			SafeDelete(topRight);
			SafeDelete(bottomLeft);
			SafeDelete(bottomRight);
		}
	}


}
