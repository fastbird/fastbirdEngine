#pragma once

#include <CommonLib/Math/Vec3.h>

namespace fastbird
{
	class Vec4
	{
	public:
		Vec4() {}
		Vec4(float _x, float _y, float _z, float _w)
			: x(_x), y(_y), z(_z), w(_w)
		{			
		}
		Vec4(const Vec3& xyz, float _w)
			: x(xyz.x), y(xyz.y), z(xyz.z), w(_w)
		{
		}
		Vec4(const Vec3& xyz)
			:x(xyz.x), y(xyz.y), z(xyz.z), w(1.f)
		{}

		Vec4(const char* s);

		float Dot( const Vec4& other ) const
		{
			return x * other.x + y * other.y + z * other.z + w * other.w;
		}

		inline Vec4 operator* (float scalar) const
		{
			return Vec4(x * scalar, y * scalar, z * scalar, w * scalar);
		}

		inline Vec4& operator*= (float scalar)
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
			w *= scalar;
			return *this;
		}

		inline Vec4& operator*= (const Vec4& r)
		{
			x *= r.x;
			y *= r.y;
			z *= r.z;
			w *= r.w;
			return *this;
		}

		inline Vec4& operator/=(float scalar)
		{
			x /= scalar;
			y /= scalar;
			z /= scalar;
			w /= scalar;
			return *this;
		}

		inline Vec4 operator* (const Vec4& r) const
		{
			return Vec4(x*r.x, y*r.y, z*r.z, w*r.w);
		}

		inline Vec4 operator+ (const Vec4& r) const
		{
			return Vec4(x + r.x, y + r.y, z + r.z, w + r.w);
		}

		bool operator== (const Vec4& other) const
		{
			return (IsEqual(x, other.x) && IsEqual(y, other.y) && IsEqual(z, other.z) && IsEqual(w, other.w));
		}

		inline void SetXYZ(const Vec3& v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
		}

		inline Vec3 ToVec3() const
		{
			return Vec3(x, y, z);
		}

	public:
		float x, y, z, w;

		static const Vec4 UNIT_X;
		static const Vec4 UNIT_Y;
		static const Vec4 UNIT_Z;
		static const Vec4 ZERO;
	};
}