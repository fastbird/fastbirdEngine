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
#include "MathDefines.h"
#include "Vec2.h"
namespace fb
{
	class Mat33;
	class Vec3I;
	class Vec3;
	typedef std::vector<Vec3> Vec3s;
	class Vec3
	{
	public:

		Real x, y, z;

		static const Vec3 UNIT_X;
		static const Vec3 UNIT_Y;
		static const Vec3 UNIT_Z;
		static const Vec3 ZERO;
		static const Vec3 ONE;
		static const Vec3 MAX;
		static const Vec3 MIN;

		//-------------------------------------------------------------------
		Vec3();
		Vec3(Real _x, Real _y, Real _z);
		explicit Vec3(const Vec2& v2, Real _z);
		explicit Vec3(Real s);		
		explicit Vec3(const Vec3I& v);
		explicit Vec3(const char* s);
		Vec3(const Vec3Tuple& t);

		//-------------------------------------------------------------------
		Vec3 operator+ (const Vec3& r) const;
		Vec3 operator+ (Real f) const;
		Vec3& operator+= (const Vec3& r);
		Vec3 operator- () const;
		Vec3 operator- (const Vec3& r) const;
		Vec3 operator- (Real f) const;
		Vec3& operator-= (const Vec3& r);
		Vec3& operator-= (const Real f);
		Vec3 operator* (Real scalar) const;
		Vec3 operator* (const Vec3& v) const;
		Vec3 operator* (const Mat33& r) const;
		Vec3& operator*= (const Vec3& r);
		Vec3& operator*= (Real f);
		Vec3 operator/ (Real scalar) const;
		Vec3 operator/ (const Vec3& v) const;
		Vec3& operator/= (const Vec3& r);
		Vec3& operator/= (Real s);
		bool operator == (const Vec3& r) const;
		bool operator != (const Vec3& r) const;
		bool operator< (const Vec3& other) const;
		bool operator >= (const Vec3& other) const;
		bool operator <= (const Vec3& other) const;
		Real operator[] (const size_t i) const;
		Real& operator[] (const size_t i);
		operator Vec3Tuple() const;
		Vec3 xyz();
		Vec3 yxy();
		Vec3 zzx();
		Vec2 xy();

		//-------------------------------------------------------------------
		Real Dot(const Vec3& vec) const;
		Real DistanceTo(const Vec3& other) const;
		Real DistanceToSQ(const Vec3&other) const;		
		void KeepGreater(const Vec3& other);
		void KeepLesser(const Vec3& other);		
		Vec3 Cross(const Vec3& rVector) const;
		int MaxAxis() const;
		void SafeNormalize();
		Real Normalize();
		Vec3 NormalizeCopy() const;
		Real Length() const;
		Real LengthSQ() const;
		Real AngleBetween(const Vec3& v) const;

		Real GetMax() const;

		void write(std::ostream& stream) const;
		void read(std::istream& stream);
	};

	Vec3 operator* (Real l, const Vec3& r);
	bool IsEqual(const Vec3& l, const Vec3& r, Real ep = EPSILON);
	bool IsEqual(const Vec2& l, const Vec2& r, Real ep = EPSILON);
	Vec3 Sign(const Vec3& v);
	Vec3 Floor(const Vec3& v);
	Vec3 Step(const Vec3& edge, const Vec3& v);

#if defined(FB_DOUBLE_PRECISION)
	class Vec3f{
	public:
		float x, y, z;

		Vec3f();
		Vec3f(Real x_, Real y_, Real z_);
		Vec3f& operator=(const Vec3& other);
	};
#else
	typedef Vec3 Vec3f;
#endif
}

// serialization
std::istream& operator>>(std::istream& stream, fb::Vec3& v);
std::ostream& operator<<(std::ostream& stream, const fb::Vec3& v);
