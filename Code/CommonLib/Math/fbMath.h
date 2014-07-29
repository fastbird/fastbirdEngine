#pragma once

#include <algorithm>
#include <math.h>
#include <CommonLib/Math/Mat33.h>
#include <CommonLib/Math/Mat44.h>
#include <CommonLib/Math/Vec3.h>
#include <CommonLib/Math/Vec3I.h>
#include <CommonLib/Math/Vec4.h>
#include <CommonLib/Math/Vec2.h>
#include <CommonLib/Math/Vec2I.h>
#include <CommonLib/Math/Quat.h>
#include <CommonLib/Math/Plane3.h>
#include <CommonLib/Math/AABB.h>
#include <CommonLib/Math/Random.h>
#include <CommonLib/Math/Ray3.h>



namespace fastbird
{
	inline bool IsNaN(float f)
	{
		return f != f;
	}

	inline float Degree(float radian)
	{
		return radian / PI * 180.0f;
	}

	inline float Radian(float degree)
	{
		return degree / 180.0f * PI;
	}

	inline Vec2 Radian(Vec2 degrees)
	{
		return Vec2(Radian(degrees.x), Radian(degrees.y));
	}

	inline bool IsPowerOfTwo(int a)
	{
		return !(a & (a-1) );
	}

	inline unsigned long GetNextPowerOfTwo(unsigned long Value)
	{
		if(Value)
		{
			Value--;
			Value = (Value >> 1) | Value;
			Value = (Value >> 2) | Value;
			Value = (Value >> 4) | Value;
			Value = (Value >> 8) | Value;
			Value = (Value >> 16) | Value;
		}
		return Value + 1;
	}

	template <class T>
	inline void Clamp(T& a, const T min, const T max)
	{
		if (a < min)
			a = min;
		else if (a>max)
			a = max;
	}

	template <class T>
	inline T ClampRet(const T a, const T min, const T max)
	{
		T ret = a;
		Clamp(ret, min, max);
		return ret;
	}

	//-------------------------------------------------------------------------
	template <class T>
	inline T Lerp(const T& a, const T& b, float lp)
	{
		return a * (1.0f-lp) + b * lp;
	}

	//-------------------------------------------------------------------------
	inline float ACos (float fValue)
	{
		if ( -1.0 < fValue )
		{
			if ( fValue < 1.0 )
				return acos(fValue);
			else
				return 0.0f;
		}
		else
		{
			return PI;
		}
	}

	//-------------------------------------------------------------------------
	inline bool IsOverlapped(const RECT& a, const RECT& b)
	{
		if (a.right < b.left || a.left > b.right ||
			a.bottom < b.top || a.top > b.bottom)
			return false;

		return true;
	}

	inline bool IsEqual(float a, float b, float epsilon = 0.00001f)
	{
		return abs(a-b) < epsilon;
	}

	inline float Step(float edge, float s)
	{
		return edge > s ? 0.0f : 1.0f;
	}

	//------------------------------------------------------------------------
	inline Mat44 MakeViewMatrix(const Vec3& pos, const Vec3& x, const Vec3& y, const Vec3& z)
	{
		// transposed
		Mat33 tansposedRot(
			x.x, x.y, x.z,
			y.x, y.y, y.z,
			z.x, z.y, z.z
			);
		Vec3 t = -(tansposedRot * pos);
		Mat44 viewMat(tansposedRot, t);
		return viewMat;
	}

	//------------------------------------------------------------------------
	// d3d left handed
	inline Mat44 MakeProjectionMatrix(float fov, float aspectRatio, float n, float f)
	{
		const float cot = 1.0f/tan(fov/2.f);
		const float DofA = cot/aspectRatio;
		float A = f / (f-n);
		float B = -n*f / (f-n);

		return Mat44(
				DofA,	0,	0,	0,
				0,		cot,	0,	0,
				0,		0,	A,	B,
				0,		0,	1,	0
				) ;
	}

	//------------------------------------------------------------------------
	// d3d left handed
	inline Mat44 MakeOrthogonalMatrix(float l, float t, float r, float b, float n, float f)
	{
		return Mat44(
			2.f/(r-l),	0,			0,	(l+r)/(l-r),
			0,			2.f/(t-b),	0,	(t+b)/(b-t),
			0,			0,			1.f/(f-n),	n/(n-f),
			0,			0,			0,			1.f);
	}

	inline Vec2 operator/(const Vec2& a, const Vec2I b)
	{
		return Vec2(a.x / (float)b.x, a.y / (float)b.y);
	}

	inline Vec3 CleanNegativeZero(const Vec3& a)
	{
		return Vec3(abs(a.x) < EPSILON ? 0.f : a.x,
			abs(a.y) < EPSILON ? 0.f : a.y,
			abs(a.z) < EPSILON ? 0.f : a.z);
	}

	//-----------------------------------------------------------------------
    Vec3 CalculateTangentSpaceVector(
        const Vec3& position1, const Vec3& position2, const Vec3& position3,
        const Vec2& uv1, const Vec2& uv2, const Vec2& uv3);

	Vec3 ProjectTo(const Plane3& plane, const Ray3& ray0, const Ray3& ray1);

	inline int GetMipLevels(float v)
	{
		return (int)((log(v) / LOG2)+0.5f)+1;
	}

	inline float Sign(float s)
	{
		return s < 0.0f ? -1.0f : (s == 0.0f ? 0.0f : 1.0f);
	}

	inline Vec3 Max(const Vec3& a, const Vec3& b)
	{
		return Vec3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
	}

	inline Vec3 Min(const Vec3& a, const Vec3& b)
	{
		return Vec3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
	}
}