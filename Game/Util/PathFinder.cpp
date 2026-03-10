#include "PathFinder.h"
#include "Engine/Engine.h"
#include "Level/GameLevel.h"
#include "Actor/EnemyBullet.h"

#include<cmath>
#include<algorithm>

static void SafeDeleteNodes(std::vector<PathNode*>& list)
{
	for (PathNode* node : list)
	{
		if (node != nullptr)
		{
			delete node;
		}
	}
	list.clear();
}

std::vector<Vector2> PathFinder::FindPath(
    const Vector2& startPos, const Vector2& targetPos)
{
	int startGridX = (int)startPos.x / GRID_SIZE;
	int startGridY = (int)startPos.y / GRID_SIZE;
	int targetGridX = (int)targetPos.x / GRID_SIZE;
	int targetGridY = (int)targetPos.y / GRID_SIZE;

	std::vector<Vector2> finalPath;

	// 타겟이 화면 밖이거나 비정상 위치면 직전 경로 반환
	if (IsInRange(targetGridX, targetGridY) == false)
	{
		finalPath.push_back((targetPos));
		return finalPath;
	}

	std::vector<PathNode*> openList;
	std::vector<PathNode*> closedList;

	// 빈 객체를 만들고 값을 하나씩 대입한다.
	PathNode* startNode = new PathNode();
	startNode->x = startGridX;
	startNode->y = startGridY;
	startNode->gCost = 0.0f;
	startNode->hCost = 0.0f;
	startNode->parent = nullptr;

	openList.push_back(startNode);

	// 8방향 및 방향에 따른 비용 설정
	struct Direction
	{
		int x;
		int y;
		float cost;
	};

	Direction dirs[8];
	// 상하 방향 코스트 계산
	dirs[0].x = 0;  dirs[0].y = -1; dirs[0].cost = 1.0f;
	dirs[1].x = 0;  dirs[1].y = 1;  dirs[1].cost = 1.0f;
	dirs[2].x = -1; dirs[2].y = 0;  dirs[2].cost = 1.0f;
	dirs[3].x = 1;  dirs[3].y = 0;  dirs[3].cost = 1.0f;

	// 좌우 방향 코스트 계산
	dirs[4].x = -1; dirs[4].y = -1; dirs[4].cost = 1.414f;
	dirs[5].x = 1;  dirs[5].y = -1; dirs[5].cost = 1.414f;
	dirs[6].x = -1; dirs[6].y = 1;  dirs[6].cost = 1.414f;
	dirs[7].x = 1;  dirs[7].y = 1;  dirs[7].cost = 1.414f;

	bool pathFound = false;
	PathNode* targetRechedNode = nullptr;

	while (openList.empty() == false)
	{
		int lowestIndex = 0;
		PathNode* currentNode = openList[0];

		//가장 비용이 싼 노드 찾기
		for (int i = 1; i < (int)openList.size(); ++i)
		{
			float currentF = GetFCost(currentNode);
			float nexF = GetFCost(openList[i]);

			if (nexF < currentF)
			{
				currentNode = openList[i];
				lowestIndex = i;
			}
			else if (nexF == currentF &&
				openList[i]->hCost < currentNode->hCost)
			{
				currentNode = openList[i];
				lowestIndex = i;
			}
		}

		// 타겟에 도달했는지 확인
		if (currentNode->x == targetGridX &&
			currentNode->y == targetGridY)
		{
			pathFound = true;
			targetRechedNode = currentNode;
			break;
		}

		openList.erase(openList.begin() + lowestIndex);
		closedList.push_back(currentNode);

		// 8방향 이웃 탐색
		for (int i = 0; i < 8; ++i)
		{
			int newX = currentNode->x + dirs[i].x;
			int newY = currentNode->y + dirs[i].y;

			// 화면 밖이거나 장애물이면 무시
			if (IsInRange(newX, newY) == false)
			{
				continue;
			}
			if (IsWalkable(newX, newY) == false)
			{
				continue;
			}

			// 닫힌 리스트 검사
			bool inClosed = false;
			for (PathNode* node : closedList)
			{
				if (node->x == newX && node->y == newY)
				{
					inClosed = true;
					break;
				}
			}

			if (inClosed == true)
			{
				continue;
			}

			float newGCost = currentNode->gCost + dirs[i].cost;

			// 열린 리스트 검사
			PathNode* neighbor = nullptr;
			for (PathNode* node : openList)
			{
				if (node->x == newX && node->y == newY)
				{
					neighbor = node;
					break;
				}
			}

			// 새로운 길 발견
			if (neighbor == nullptr)
			{
				neighbor = new PathNode();
				neighbor->x = newX;
				neighbor->y = newY;
				neighbor->gCost = newGCost;
				neighbor->hCost = CalculateHeurisic(newX, newY,
					targetGridX, targetGridY);
				neighbor->parent = currentNode;
				
				openList.push_back(neighbor);
			}

			// 더 좋은 길 발견
			else if (newGCost < neighbor->gCost)
			{
				neighbor->gCost = newGCost;
				neighbor->parent = currentNode;
			}
		}
	}
	// 경로 역추적
	if (pathFound == true && targetRechedNode != nullptr)
	{
		PathNode* curr = targetRechedNode;
		while (curr != startNode && curr != nullptr)
		{
			// 그리드 좌표를 픽셀 좌표로 다시 복원 (칸의 중앙점)
			float posX = (float)(curr->x * GRID_SIZE + (GRID_SIZE / 2));
			float posY = (float)(curr->y * GRID_SIZE + (GRID_SIZE / 2));

			finalPath.push_back(Vector2((int)posX, (int)posY));
			curr = curr->parent;
		}

		// AStar.cpp 예시에 있던 std::reverse 활용 (목표->시작을 시작->목표로 뒤집음)
		std::reverse(finalPath.begin(), finalPath.end());
	}
	else
	{
		// 길이 막혔을 경우 강행 돌파 처리
		finalPath.push_back(targetPos);
	}

	// 사용한 메모리 정리
	SafeDeleteNodes(openList);
	SafeDeleteNodes(closedList);

	return finalPath;
}

float PathFinder::GetFCost(PathNode* node)
{
	if (node == nullptr)
	{
		return 0.0f;
	}
	return node->gCost + node->hCost;
}

float PathFinder::CalculateHeurisic(int startX, int startY, int goalX, int goalY)
{
	float dx = (float)(goalX - startX);
	float dy = (float)(goalY - startY);
	return std::sqrt(dx * dx + dy * dy);
}

bool PathFinder::IsInRange(int gridX, int gridY)
{
	int maxGridX = Engine::Get().GetWidth() / GRID_SIZE;
	int maxGridY = Engine::Get().GetHeight() / GRID_SIZE;

	if (gridX >= 0 && gridX < maxGridX &&
		gridY >= 0 && gridY < maxGridY)
	{
		return true;
	}

	return false;
}

bool PathFinder::IsWalkable(int gridX, int gridY)
{
	int realX = gridX * GRID_SIZE;
	int realY = gridY * GRID_SIZE;

	// GameLevel의 벡터를 순회하는 기존 방식 그대로 채용
	const std::vector<Actor*>& actors = GameLevel::Get().GetActors();
	for (Actor* actor : actors)
	{
		// 명시적 포인터 및 bool 검사 적용
		if (actor != nullptr && actor->IsActive() == true && actor->IsTypeOf<class EnemyBullet>())
		{
			int ax = (int)actor->GetPosition().x;
			int ay = (int)actor->GetPosition().y;
			int aw = actor->GetWidth();
			int ah = actor->GetHeight();

			// 장애물과 A* 그리드(5x5 구역)의 충돌 검사
			if (realX < ax + aw && realX + GRID_SIZE > ax &&
				realY < ay + ah && realY + GRID_SIZE > ay)
			{
				return false; // 통과 불가
			}
		}
	}
	return true; // 통과 가능
}
