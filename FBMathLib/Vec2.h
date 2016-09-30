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
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/level.hpp>
#include "Vec2I.h"
#include "FBCommonHeaders\String.h"
namespace fb
{
	class Vec2
	{
	public:
		Real x, y;

		static const Vec2 UNIT_X;
		static const Vec2 UNIT_Y;
		static const Vec2 ZERO;
		static const Vec2 ONE;		

		//-------------------------------------------------------------------
		Vec2();
		Vec2(Real _x, Real _y);
		explicit Vec2(const Vec2I& v);		
		Vec2(const Vec2Tuple& t);

		//-------------------------------------------------------------------
		Vec2 operator+ (Real s) const;
		Vec2& operator+= (Real s);
		Vec2 operator+ (const Vec2& v) const;
		Vec2& operator+= (const Vec2& s);

		Vec2 operator- (Real s) const;
		Vec2 operator- () const;
		Vec2 operator- (const Vec2& v) const;
		Vec2& operator-= (Real s);

		Vec2 operator* (Real s) const;
		Vec2 operator* (const Vec2& v) const;
		Vec2& operator*= (Real s);
		Vec2& operator*= (const Vec2& v);

		Vec2 operator/ (Real s) const;
		Vec2 operator/ (const Vec2& v) const;
		Vec2& operator/= (const Vec2& other);
		Vec2& operator/= (Real s);				
		
		bool operator== (const Vec2& other) const;
		bool operator!= (const Vec2& other) const;
		Real operator[] (const size_t i) const;
		Real& operator[] (const size_t i);
		bool operator<(const Vec2& other) const;
		operator Vec2Tuple() const;

		//-------------------------------------------------------------------
		Real Normalize();
		Vec2 NormalizeCopy() const;
		void SafeNormalize();
		Real Length() const;
		Real LengthSQ() const;
		Real DistanceTo(const Vec2& other) const;
		Real DistanceToSQ(const Vec2& other) const;
		Real Cross(const Vec2& right);
		
		int MaxAxis() const;		
		size_t ComputeHash() const;
	};

	Vec2 operator / (const Vec2I& left, const Vec2& right);
	Vec2 operator * (const Vec2I& left, const Vec2& right);
	Vec2 operator * (const Vec2& left, const Vec2I& right);
	Vec2 operator*(const Vec2I& left, Real right);

#if defined(FB_DOUBLE_PRECISION)
	class Vec2f{
	public:
		float x, y;

		Vec2f();
		Vec2f(Real x_, Real y_);
		Vec2f& operator=(const Vec2& other);
	};
#else
	typedef Vec2 Vec2f;
#endif
}

BOOST_CLASS_IMPLEMENTATION(fb::Vec2, boost::serialization::primitive_type);