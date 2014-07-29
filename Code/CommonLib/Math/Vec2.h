#pragma once

namespace fastbird
{
	class Vec2
	{
	public:
		Vec2()
		{
		}

		Vec2(float _x, float _y)
			:x(_x), y(_y)
		{
		}

		Vec2(const Vec2& other)
			: x(other.x), y(other.y)
		{
		}

		//-------------------------------------------------------------------
		Vec2 operator* (float s) const
		{
			return Vec2(x*s, y*s);
		}

		Vec2 operator/ (float s) const
		{
			return Vec2(x/s, y/s);
		}

		Vec2 operator+ (float s) const
		{
			return Vec2(x+s, y+s);
		}

		Vec2 operator- (float s) const
		{
			return Vec2(x-s, y-s);
		}

		//-------------------------------------------------------------------
		Vec2 operator* (const Vec2& v) const
		{
			return Vec2(x*v.x, y*v.y);
		}

		Vec2& operator*= (float s)
		{
			x*=s;
			y*=s;
			return *this;
		}

		Vec2& operator+= (float s)
		{
			x+=s;
			y+=s;
			return *this;
		}

		Vec2& operator-= (float s)
		{
			x-=s;
			y-=s;
			return *this;
		}

		Vec2& operator+= (const Vec2& s)
		{
			x+=s.x;
			y+=s.y;
			return *this;
		}

		Vec2 operator/ (const Vec2& v) const
		{
			return Vec2(x/v.x, y/v.y);
		}

		Vec2 operator+ (const Vec2& v) const
		{
			return Vec2(x+v.x, y+v.y);
		}

		Vec2 operator- (const Vec2& v) const
		{
			return Vec2(x-v.x, y-v.y);
		}

		bool operator== (const Vec2& other) const
		{
			return x==other.x && y==other.y;
		}

		bool operator<(const Vec2& other) const;

		float x, y;

		static const Vec2 UNIT_X;
		static const Vec2 UNIT_Y;
		static const Vec2 ZERO;


	};
}