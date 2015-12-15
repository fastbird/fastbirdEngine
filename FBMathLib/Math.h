/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#pragma once

#include "Vec3.h"
#include "Vec4.h"
#include "Mat33.h"
#include "Mat44.h"

#include "Vec3I.h"
#include "Vec2.h"
#include "Vec2I.h"
#include "Quat.h"
#include "Plane3.h"
#include "AABB.h"
#include "Random.h"
#include "Ray3.h"
#include "Transformation.h"
#include "MathDefines.h"
#include "Rect.h"
#include "Mat44.h"
#include "Color.h"

namespace fb
{
	class Color;
	bool IsEqual(Real a, Real b, Real epsilon = 0.00001f);
	bool IsNaN(Real f);	

	template<typename T>
	inline bool IsInf(T value)
	{
		return std::numeric_limits<T>::has_infinity &&
			(value == std::numeric_limits<T>::infinity() || value == -std::numeric_limits<T>::infinity());
	}

	Real Degree(Real radian);
	Real Radian(Real degree);
	Vec2 Radian(const Vec2& degrees);
	bool IsPowerOfTwo(int a);
	int Round(Real v);
	Vec2I Round(const Vec2& v);
	Vec3I Round(const Vec3& v);
	Vec3 FixPrecisionScaleVector(const Vec3& v);
	Real log2(Real v);
	Real sinc(Real x);	
	bool IsLittleEndian(); // intel
	void Halfp2Singles(void *target, void *source, unsigned num);
	void Halfp2Doubles(void *target, void *source, unsigned num);
	void Doubles2Halfp(void *target, void *source, unsigned num);
	void Singles2Halfp(void *target, void *source, unsigned num);

	//-----------------------------------------------------------------------
	unsigned long GetNextPowerOfTwo(unsigned long Value);
	Vec2I GetNextPowerOfTwo(const Vec2I& value);
	unsigned GetNextMultipleOfFour(unsigned value);
	Vec2I GetNextMultipleOfFour(const Vec2I value);

	template <class T>
	void Clamp(T& a, const T min, const T max)
	{
		if (a < min)
			a = min;
		else if (a>max)
			a = max;
	}

	template <class T>
	T ClampRet(const T a, const T min, const T max)
	{
		T ret = a;
		Clamp(ret, min, max);
		return ret;
	}

	//-------------------------------------------------------------------------
#undef min
#undef max
	template <class T>
	T Lerp(const T& a, const T& b, Real lp)
	{
		lp = std::min(lp, (Real)1.0);
		lp = std::max((Real)0.0, lp);
		return a * (1.0f-lp) + b * lp;
	}

	Color Lerp(const Color& a, const Color& b, Real lp);

	Quat Slerp(Quat qa, Quat qb, Real t);
	Real ACos(Real fValue);
	bool IsOverlapped(const Rect& a, const Rect& b);
	Real Step(Real edge, Real s);	
	Mat44 MakeViewMatrix(const Vec3& pos, const Vec3& x, const Vec3& y, const Vec3& z);	
	/// d3d left handed
	Mat44 MakeProjectionMatrix(Real fov, Real aspectRatio, Real n, Real f);	
	/// d3d left handed
	Mat44 MakeOrthogonalMatrix(Real l, Real t, Real r, Real b, Real n, Real f);
	Vec2 operator/(const Vec2& a, const Vec2I b);
	Vec3 CleanNegativeZero(const Vec3& a);	
    Vec3 CalculateTangentSpaceVector(
        const Vec3& position1, const Vec3& position2, const Vec3& position3,
        const Vec2& uv1, const Vec2& uv2, const Vec2& uv3);

	Vec3 ProjectTo(const Plane3& plane, const Ray3& ray0, const Ray3& ray1);

	int GetMipLevels(Real v);
	Real Sign(Real s);
	Vec3 Max(const Vec3& a, const Vec3& b);
	Vec3 Min(const Vec3& a, const Vec3& b);
	Vec2 Abs(const Vec2& v);
	Vec3 Abs(const Vec3& v);
	/**
	0 <= r < infinite
	0 <= theta <= PI
	0 <= phi < TWO_PI
	*/
	Vec3 SphericalToCartesian(Real r, Real theta, Real phi);
	/// for r == 1.0
	Vec3 SphericalToCartesian(Real theta, Real phi);
	Vec3 CartesianToSpherical(const Vec3& c);
	Real SmoothStep(Real min, Real max, Real v);
	Real Squared(Real x);
	// outPos and TimeToTarget have to be specified both if you need.
	void CalcInterceptPosition(const Vec3& firePos, Real ammoSpeed, const Vec3& toTargetDir, Real distance, const Vec3& targetVel,
		Vec3& outVelocity, Vec3* outPos = 0, Real* timeToTarget = 0);
	
	enum SegmentIntersectResult{
		SIR_COLLINEAR,
		SIR_NONINTERSECTING,
		SIR_INTERSECT,
	};
	SegmentIntersectResult SegmentIntersect(const Vec2& p, const Vec2& pend,
						const Vec2& q, const Vec2& qend, 
						Vec2& outIntersect);

	void ExpandRect(Rect& r, int size);

	Vec2I GetMinComp(const Vec2I& a, const Vec2I& b);
	Vec2I GetMaxComp(const Vec2I& a, const Vec2I& b);

	bool RayAABB(const Vec3& ro, const Vec3& rdi,
		const Vec3I& raySign,
		const AABB& aabb,
		Real& min, Vec3& normal,
		Real pseudo_min, Real pseudo_max);

	Real GetTimeToExitNDCSquare(const Vec2& pos, const Vec2& dir);


	// Intended redundancy
	// Same functions in StringLib
	void StepToDigit_(TCHAR** ppch);

	void GenerateHammersley(int numSamples, std::vector<Vec2>& outResult);
	int CropSize8(int size);
	Real ShortestYaw(Real from, Real to);
}