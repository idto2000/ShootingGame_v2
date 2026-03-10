#pragma once
#include "Math/Vector2.h"

#include<vector>


using namespace Wanted;

struct PathNode
{
	int x;
	int y;

	float gCost;
	float hCost;

	PathNode* parent;
};

class PathFinder
{
public:

	// 길찾기 결과 반환
	static std::vector<Vector2> FindPath(const Vector2& startPos,
		const Vector2& targetPos);


private:

	// 그리드 1칸의 크기(최적화를 위해 5*5 픽셀을 1칸으로 세팅)
	static const int GRID_SIZE = 5;
	
	// 총점을 계산하는 함수
	static float GetFCost(PathNode* node);
	static float CalculateHeurisic(int startX, int startY,
		int goalX, int goalY);

	static bool IsInRange(int gridX, int gridY);
	static bool IsWalkable(int gridX, int gridY);

	PathNode* parent = nullptr;
};
