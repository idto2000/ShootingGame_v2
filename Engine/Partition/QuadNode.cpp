#include "QuadNode.h"
#include "Actor/Actor.h"
#include "Render/Renderer.h"
#include "Math/Vector2.h"
#include "Engine/Engine.h"

const int MAX_NODE_CAPACITY = 1;


namespace Wanted
{
	// 정적 변수 초기화
	bool QuadNode::isShowActorNames = false;

	QuadNode::QuadNode(const Bounds& bounds, int depth)
		: bounds(bounds), depth(depth)
	{
	}
	QuadNode::~QuadNode()
	{
		// [수정] 소멸자에서 자식 노드들을 물리적으로 delete 합니다.
		// delete는 해당 객체의 소멸자를 다시 호출하므로 트리 전체가 재귀적으로 삭제됩니다.
		if (topLeft) { delete topLeft; topLeft = nullptr; }
		if (topRight) { delete topRight; topRight = nullptr; }
		if (bottomLeft) { delete bottomLeft; bottomLeft = nullptr; }
		if (bottomRight) { delete bottomRight; bottomRight = nullptr; }

		actors.clear();
	}

	// 액터를 쿼드트리에 추가하는 함수
	void QuadNode::Insert(Actor* actor)
	{
		if (!actor)
		{
			return;
		}

		// 액터가 위치한 영역 계산
		Bounds actorBounds(actor->GetPosition().x,
			actor->GetPosition().y,
			actor->GetWidth(), 1);

		//1. 분할된 상태라면 자식에게 삽입 시도
		if (IsDivide())
		{
			NodeIndex result = TestRegion(actorBounds);

			if (result == NodeIndex::Straddling)
			{
				actors.emplace_back(actor);
			}
			else if (result != NodeIndex::QutOfArea)
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

				return;
			}
		}

		//2. 아직 분할되지 않음 상태라면 일단 현재 노드가 보관
		actors.emplace_back(actor);

		//3. 2개 이상 있있을때 분할
		if (actors.size() > 0 && depth < 5)
		{
			if (Subdivide())
			{
				// 가지고 있는 액터 자식에게 재배치
				auto it = actors.begin();
				while (it != actors.end())
				{
					Actor* existingActor = *it;
					Bounds b((int)existingActor->GetPosition().x,
						(int)existingActor->GetPosition().y,
						(int)existingActor->GetWidth(), 1);
					NodeIndex res = TestRegion(b);

					// 자식 노드 한 곳에 완전히 쏙 들어가는 놈만 밑으로 보냄
					if (res != NodeIndex::Straddling && res != NodeIndex::QutOfArea)
					{
						if (res == NodeIndex::TopLeft)
						{
							topLeft->Insert(existingActor);
						}
						else if (res == NodeIndex::TopRight)
						{
							topRight->Insert(existingActor);
						}
						else if (res == NodeIndex::BottomLeft)
						{
							bottomLeft->Insert(existingActor);
						}
						else if (res == NodeIndex::BottomRight)
						{
							bottomRight->Insert(existingActor);
						}

						// 현재 노드 리스트에서 제거
						it = actors.erase(it);
					}
					else
					{
						++it;
					}
				}
			}
		}

		//// 4분면 중 어디에 들어가는지 확인
		//NodeIndex result = TestRegion(actorBounds);

		//// 두 개 이상 영역에 겹치는 경우에는 현재 노드에 추가.
		//if (result == NodeIndex::Straddling)
		//{
		//	actors.emplace_back(actor);
		//}

		//// 포함될 경우 자식 노드로 보냄
		//else if (result != NodeIndex::QutOfArea)
		//{
		//	// 아직 자식 노드가 없다면 분할
		//	if (Subdivide())
		//	{
		//		if (result == NodeIndex::TopLeft)
		//		{
		//			topLeft->Insert(actor);
		//		}
		//		else if (result == NodeIndex::TopRight)
		//		{
		//			topRight->Insert(actor);
		//		}
		//		else if (result == NodeIndex::BottomLeft)
		//		{
		//			bottomLeft->Insert(actor);
		//		}
		//		else if (result == NodeIndex::BottomRight)
		//		{
		//			bottomRight->Insert(actor);
		//		}
		//	}
		//	else
		//	{
		//		actors.emplace_back(actor);
		//	}
		//}
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

		/*if (IsDivide())
		{
			topLeft->Clear();
			topRight->Clear();
			bottomLeft->Clear();
			bottomRight->Clear();
		}*/
	}

	int QuadNode::GetTotalActorCount() const
	{
		// 현재 노드가 가진 액터 수
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
			count += topLeft->GetTotalNodeCount();
			count += topRight->GetTotalNodeCount();
			count += bottomLeft->GetTotalNodeCount();
			count += bottomRight->GetTotalNodeCount();
		}
		return count;
	}

	void QuadNode::DebugDraw() const
	{
		if (depth > 1 && GetTotalActorCount() == 0)
		{
			return;
		}

		// 1. 공통 좌표 계산
		int x = static_cast<int>(bounds.X());
		int y = static_cast<int>(bounds.Y());
		int w = static_cast<int>(bounds.Width());
		int h = static_cast<int>(bounds.Height());

		// 쿼드트리 시각화 그리드 표시
		int sw = Engine::Get().GetWidth();
		int sh = Engine::Get().GetHeight();

		// 쿼드 트리 그리드 표시
		Color treeColor = Color::DarkGray;
		int treeOrder = 80;

		// 경계선(x+w, y+h)까지 루프를 돌려 인접한 칸과 선을 겹치게 그립니다. (한 줄 효과)
		// 가로선 (-)
		for (int i = 0; i <= w; ++i)
		{
			int curX = x + i;
			if (curX < 0 || curX >= sw)
			{
				continue;
			}
			if (y >= 0 && y < sh)
			{
				Renderer::Get().Submit("-", Vector2(curX, y),
					treeColor, treeOrder);
			}

			if (y + h >= 0 && y + h < sh && y + h < Engine::Get().GetHeight())
			{
				Renderer::Get().Submit("-", Vector2(curX, (y + h)),
					treeColor, treeOrder);
			}
		}

		// 세로선 (|)
		for (int j = 0; j <= h; ++j)
		{
			int curY = y + j;
			if (curY < 0 || curY >= sh)
			{
				continue;
			}
			if (x >= 0 && x < sw)
			{
				Renderer::Get().Submit("|", Vector2(x, curY),
					treeColor, treeOrder);
			}
			if (x + w >= 0 && x + w < sw && x + w < Engine::Get().GetWidth())
			{
				Renderer::Get().Submit("|", Vector2((x + w), curY),
					treeColor, treeOrder);
			}
		}

		// 6. 충돌 액터 격자 표시
		if (!actors.empty())
		{
			for (int k = 0; k < (int)actors.size(); ++k)
			{
				// 현재 유효한 액터 또는 살아있는 액터 체크
				if (actors[k] != nullptr && actors[k]->IsActive())
				{
					// 충돌 액터의 실제 좌표를 가져옴
					int ax = (int)actors[k]->GetPosition().x;
					int ay = (int)actors[k]->GetPosition().y;
					int aw = actors[k]->GetWidth();

					int boxX = ax - 1;
					int boxY = ay - 1;
					int boxW = aw + 1;

					// 액터 전용 초록색 충돌 박스 렌더링
					for (int i = 0; i <= boxW; ++i)
					{
						Renderer::Get().Submit("-", Vector2(boxX + i, boxY), Color::Green, 90);     // 윗면
						Renderer::Get().Submit("-", Vector2(boxX + i, boxY + 2), Color::Green, 90); // 아랫면
					}
					Renderer::Get().Submit("|", Vector2(boxX, boxY + 1), Color::Green, 90);         // 좌측면
					Renderer::Get().Submit("|", Vector2(boxX + boxW, boxY + 1), Color::Green, 90);  // 우측면

					// 충돌 박스 모서리 포인트 (흰색 +)
					Renderer::Get().Submit("+", Vector2(boxX, boxY), Color::White, 91);
					Renderer::Get().Submit("+", Vector2(boxX + boxW, boxY), Color::White, 91);
					Renderer::Get().Submit("+", Vector2(boxX, boxY + 2), Color::White, 91);
					Renderer::Get().Submit("+", Vector2(boxX + boxW, boxY + 2), Color::White, 91);

					// 키 입력
					if (isShowActorNames)
					{
						// GameLevel에서 키 입력받고 표시
						Renderer::Get().Submit(actors[k]->GetImage(), Vector2(ax, boxY-1), Color::Yellow, 110);
					}
				}
			}
			
		}

		//if (!actors.empty())
		//{
		//	for (int k=0; k < (int)actors.size(); ++k)
		//	{
		//		// 액터가 활성화인 상태만 접근
		//		if (actors[k] != nullptr && actors[k]->IsActive())
		//		{
		//			Renderer::Get().Submit(actors[k]->GetImage(),
		//				Vector2(x + 1, y + 1 + k), Color::Yellow, 110);
		//		}
		//	}
		//}

		// 정적 변수가 true일 때만 액터 이름을 출력
		if (isShowActorNames && !actors.empty())
		{

			for (int k = 0; k < (int)actors.size(); ++k)
			{
				// 액터가 유효하고 살아 있을 때만 출력
				if (actors[k] != nullptr && actors[k]->IsActive())
				{
					Renderer::Get().Submit(actors[k]->GetImage(),
						Vector2(x + 1, y + 1 + k), Color::Yellow, 110);
				}				
			}
		}

		// 모든 레벨의 그리드가 겹쳐 보이게
		if (IsDivide())
		{
			if (topLeft) topLeft->DebugDraw();
			if (topRight) topRight->DebugDraw();
			if (bottomLeft) bottomLeft->DebugDraw();
			if (bottomRight) bottomRight->DebugDraw();
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
		/*if (IsDivide())
		{
			SafeDelete(topLeft);
			SafeDelete(topRight);
			SafeDelete(bottomLeft);
			SafeDelete(bottomRight);
		}*/
	}


}
