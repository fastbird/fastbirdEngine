#pragma once

#include <algorithm>
#include <math.h>
namespace fastbird
{
	bool IsEqual(float a, float b, float epsilon = 0.00001f);
}
#include <CommonLib/Math/Vec3.h>
#include <CommonLib/Math/Vec4.h>
#include <CommonLib/Math/Mat33.h>
#include <CommonLib/Math/Mat44.h>

#include <CommonLib/Math/Vec3I.h>
#include <CommonLib/Math/Vec2.h>
#include <CommonLib/Math/Vec2I.h>
#include <CommonLib/Math/Quat.h>
#include <CommonLib/Math/Plane3.h>
#include <CommonLib/Math/AABB.h>
#include <CommonLib/Math/Random.h>
#include <CommonLib/Math/Ray3.h>
#include <CommonLib/Math/Transformation.h>

namespace fastbird
{
	inline bool IsNaN(float f)
	{
		return f != f;
	}

	template<typename T>
	inline bool IsInf(T value)
	{
		return std::numeric_limits<T>::has_infinity &&
			value == std::numeric_limits<T>::infinity();
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

	inline int Round(float v)
	{
		return (int)(v + 0.5f);
	}

	inline Vec2I Round(const Vec2& v)
	{
		return Vec2I(Round(v.x), Round(v.y));
	}

	inline Vec3I Round(const Vec3& v)
	{
		return Vec3I(Round(v.x), Round(v.y), Round(v.z));
	}

	inline Vec3 FixPrecisionScaleVector(const Vec3& v)
	{
		Vec3 fixedScale = v;
		Vec3I rounded = Round(v);
		if (rounded.x == rounded.y && rounded.x == rounded.z)
		{
			// uniform
			if (abs((float)rounded.x - v.x) < 0.00001f)
			{
				fixedScale.x = fixedScale.y = fixedScale.z = (float)rounded.x;
			}
		}
		return fixedScale;
	}

	inline float log2(float v)
	{
		return (float)(log(v) / log(2));
	}

	inline float sinc(float x) {               /* Supporting sinc function */
		if (fabs(x) < 1.0e-4) return 1.0;
		else return(sin(x) / x);
	}
	
	bool IsLittleEndian(); // intel

	void Halfp2Singles(void *target, void *source, unsigned num);
	void Halfp2Doubles(void *target, void *source, unsigned num);
	void Doubles2Halfp(void *target, void *source, unsigned num);
	void Singles2Halfp(void *target, void *source, unsigned num);

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

	inline Vec2I GetNextPowerOfTwo(const Vec2I& value)
	{
		return Vec2I(GetNextPowerOfTwo(value.x), GetNextPowerOfTwo(value.y));
	}

	inline unsigned GetNextMultipleOfFour(unsigned value)
	{
		return (value + 3)&~0x03;
	}

	inline Vec2I GetNextMultipleOfFour(const Vec2I value)
	{
		return Vec2I(GetNextMultipleOfFour(value.x), GetNextMultipleOfFour(value.y));
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
		lp = std::min(lp, 1.0f);
		lp = std::max(0.f, lp);
		return a * (1.0f-lp) + b * lp;
	}

	inline Quat Slerp(Quat qa, Quat qb, float t) 
	{
		float cosHalfTheta = qa.w * qb.w + qa.x * qb.x + qa.y * qb.y + qa.z * qb.z;
		// qa and qb is same.
		if (abs(cosHalfTheta) >= 1.0)
		{
			return qa;
		}
		Quat ret;
		float halfTheta = acos(cosHalfTheta);
		float sinHalfTheta = sqrt(1.0f - cosHalfTheta*cosHalfTheta);
		// 180 degree case
		// not defined.
		if ((float)fabs(sinHalfTheta) < 0.001f)
		{ 
			ret.w = (qa.w * 0.5f + qb.w * 0.5f);
			ret.x = (qa.x * 0.5f + qb.x * 0.5f);
			ret.y = (qa.y * 0.5f + qb.y * 0.5f);
			ret.z = (qa.z * 0.5f + qb.z * 0.5f);
			return ret;
		}

		float ratioA = sin((1.0f - t) * halfTheta) / sinHalfTheta;
		float ratioB = sin(t * halfTheta) / sinHalfTheta;
		ret.w = (qa.w * ratioA + qb.w * ratioB);
		ret.x = (qa.x * ratioA + qb.x * ratioB);
		ret.y = (qa.y * ratioA + qb.y * ratioB);
		ret.z = (qa.z * ratioA + qb.z * ratioB);
		return ret;
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

	inline float Step(float edge, float s)
	{
		return edge > s ? 0.0f : 1.0f;
	}

	//------------------------------------------------------------------------
	inline Mat44 MakeViewMatrix(const Vec3& pos, const Vec3& x, const Vec3& y, const Vec3& z)
	{
		// transposed
		Mat33 transposedRot(
			x.x, x.y, x.z,
			y.x, y.y, y.z,
			z.x, z.y, z.z
			);
		Vec3 t = -(transposedRot * pos);
		Mat44 viewMat(transposedRot, t);
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

	inline Vec2 Abs(const Vec2& v)
	{
		return Vec2(abs(v.x), abs(v.y));
	}

	inline Vec3 Abs(const Vec3& v)
	{
		return Vec3(abs(v.x), abs(v.y), abs(v.z));
	}

	// 0 <= r < infinite
	// 0 <= theta <= PI
	// 0 <= phi < TWO_PI
	inline Vec3 SphericalToCartesian(float r, float theta, float phi)
	{
		float st = sin(theta);
		float cp = cos(phi);
		float sp = sin(phi);
		float ct = cos(theta);

		return Vec3(r*st*cp, r*st*sp, r*ct);
	}

	// for r == 1.0
	inline Vec3 SphericalToCartesian(float theta, float phi)
	{
		float st = sin(theta);
		float cp = cos(phi);
		float sp = sin(phi);
		float ct = cos(theta);

		return Vec3(st*cp, st*sp, ct);
	}

	inline Vec3 CartesianToSpherical(const Vec3& c)
	{
		float r = c.Length();
		float theta = acos(c.z / r);
		float phi = atan2(c.y, c.x);
		phi += phi < 0 ? TWO_PI : 0;
		return Vec3(r, theta, phi);
	}

	inline float SmoothStep(float min, float max, float v)
	{
		float size = max - min;
		float pos = v - min;
		return pos / size;
	}

	inline float Squared(float x)
	{
		return x*x;
	}

	// outPos and TimeToTarget have to be specified both if you need.
	void CalcInterceptPosition(const Vec3& firePos, float ammoSpeed, const Vec3& toTargetDir, float distance, const Vec3& targetVel,
		Vec3& outVelocity, Vec3* outPos = 0, float* timeToTarget = 0);

	void ExpandRect(RECT& r, int size);
}