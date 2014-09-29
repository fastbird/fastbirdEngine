#include <CommonLib/StdAfx.h>
#include <CommonLib/Math/fbMath.h>
#include "Vec2.h"
namespace fastbird
{
	const Vec2 Vec2::UNIT_X(1.f, 0);
	const Vec2 Vec2::UNIT_Y(0, 1.f);
	const Vec2 Vec2::ZERO(0, 0);

	bool Vec2::operator<(const Vec2& other) const
	{
		if (x < other.x)
			return true;
		else if (IsEqual(x, other.x, EPSILON))
			return y<other.y;

		return false;
	}
}
