#include "Bounds.h"

namespace Wanted
{
	Bounds::Bounds(int x, int y, int width, int height)
		: x(x), y(y), width(width), height(height)
	{
	}

	bool Bounds::Instersects(const Bounds& other) const
	{
		// 겹치지 않는 4가지의 경우 체크
		// 내 오른쪽 끝보다 더 오른쪽에 있을 경우
		if (other.x > MaxX())
		{
			return false;
		}
		
		// 오른쪽 끝이 내 왼쪽보다 더 왼쪽에 있을 경우
		if (other.MaxX() < x)
		{
			return false;
		}

		// 내 아래쪽 끝보다 더 아래에 있을 때
		if (other.y > MaxY())
		{
			return false;
		}

		// 아래쪽 끝이 내 위쪽보다 더 위에 있을 때
		if (other.MaxY() < y)
		{
			return false;
		}

		// 모두 해당하지 않을 경우 겹침
		return true;
	}
}

