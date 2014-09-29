#pragma once

namespace fastbird
{
	class Vec2I
	{
	public:
		int x, y;

		Vec2I()
		{
		}

		Vec2I(int _x, int _y)
			: x(_x), y(_y)
		{
		}

		//-------------------------------------------------------------------
		Vec2I operator* (int s) const
		{
			return Vec2I(x*s, y*s);
		}

		Vec2I operator/ (int s) const
		{
			return Vec2I(x/s, y/s);
		}

		Vec2I operator+ (int s) const
		{
			return Vec2I(x+s, y+s);
		}

		Vec2I operator- (int s) const
		{
			return Vec2I(x-s, y-s);
		}

		//-------------------------------------------------------------------
		Vec2I operator* (const Vec2I& v) const
		{
			return Vec2I(x*v.x, y*v.y);
		}

		Vec2I operator/ (const Vec2I& v) const
		{
			return Vec2I(x/v.x, y/v.y);
		}

		Vec2I operator+ (const Vec2I& v) const
		{
			return Vec2I(x+v.x, y+v.y);
		}

		Vec2I operator- (const Vec2I& v) const
		{
			return Vec2I(x-v.x, y-v.y);
		}

		//-------------------------------------------------------------------
		bool operator== (const Vec2I& v) const
		{
			return x == v.x && y == v.y;
		}

		bool operator!=(const Vec2I& v) const
		{
			return !operator==(v);
		}

		static const Vec2I ZERO;
	};
}