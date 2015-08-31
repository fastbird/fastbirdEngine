#include <CommonLib/StdAfx.h>
#include <CommonLib/Math/Vec2I.h>

namespace fastbird
{

	Vec2I::Vec2I(const Vec2& v){
		x = Round(v.x);
		y = Round(v.y);
	}

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

	Vec2I Vec2I::Scale(float f) const
	 {
		 return Vec2I(Round(x*f), Round(y*f));
	 }
}