#pragma once
#include <CommonLib/Math/Vec3.h>
namespace fastbird
{
	class Vec3I
	{
	public:
		int x, y, z;

		Vec3I()
		{
		}

		Vec3I(int _x, int _y, int _z)
			: x(_x), y(_y), z(_z)
		{
		}

		Vec3I(const Vec3& v)
		{
			x = (int)v.x;
			y = (int)v.y;
			z = (int)v.z;
		}

		//-------------------------------------------------------------------
		Vec3I operator* (int s) const
		{
			return Vec3I(x*s, y*s, z*s);
		}

		Vec3 operator*(float s) const
		{
			return Vec3(x*s, y*s, z*s);
		}

		Vec3I operator/ (int s) const
		{
			return Vec3I(x/s, y/s, z*s);
		}

		Vec3I operator+ (int s) const
		{
			return Vec3I(x+s, y+s, z+s);
		}

		Vec3I operator- (int s) const
		{
			return Vec3I(x-s, y-s, y-s);
		}

		//-------------------------------------------------------------------
		Vec3I operator* (const Vec3I& v) const
		{
			return Vec3I(x*v.x, y*v.y, z*v.z);
		}

		Vec3I operator/ (const Vec3I& v) const
		{
			return Vec3I(x/v.x, y/v.y, z/v.z);
		}

		Vec3I operator+ (const Vec3I& v) const
		{
			return Vec3I(x+v.x, y+v.y, z+v.z);
		}

		Vec3I operator- (const Vec3I& v) const
		{
			return Vec3I(x-v.x, y-v.y, y-v.y);
		}

		//-------------------------------------------------------------------
		bool operator== (const Vec3I& v) const
		{
			return x == v.x && y == v.y && z==v.z;
		}

		bool operator!=(const Vec3I& v) const
		{
			return !operator==(v);
		}

		static const Vec3I UNIT_X;
		static const Vec3I UNIT_Y;
		static const Vec3I UNIT_Z;
		static const Vec3I ZERO;
	};
}