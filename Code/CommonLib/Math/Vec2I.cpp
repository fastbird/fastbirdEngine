#include <CommonLib/StdAfx.h>
#include <CommonLib/Math/Vec2I.h>

namespace fastbird
{
	const Vec2I Vec2I::ZERO(0, 0);
	bool Vec2I::operator<(const Vec2I& v) const
	{
		if (x < v.x)
		{
			return true;
		}
		else if (x == v.x)
		{
			if (y<v.y)
			{
				return true;
			}
		}

		return false;
	}

}