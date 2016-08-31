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
#include "Vec3.h"

namespace fb
{
#if defined(FB_DOUBLE_PRECISION)
	class Vec4f;
#endif
	class Vec4
	{
	public:
		typedef std::vector<Vec4> Array;
		Real x, y, z, w;

		static const Vec4 UNIT_X;
		static const Vec4 UNIT_Y;
		static const Vec4 UNIT_Z;
		static const Vec4 UNIT_W;
		static const Vec4 ZERO;

		//-------------------------------------------------------------------------
		Vec4();		
		Vec4(Real _x, Real _y, Real _z, Real _w);
		Vec4(const Vec3& xyz, Real _w);
#if defined(FB_DOUBLE_PRECISION)
		Vec4(const Vec4f& other);
#endif
		explicit Vec4(const Vec3& xyz);
		explicit Vec4(const char* s);
		Vec4(const Vec4Tuple& t);

		//-------------------------------------------------------------------------
		Vec4 operator+(const Vec4& r) const;
		Vec4 operator+(Real v);
		Vec4 operator* (Real scalar) const;
		Vec4& operator*= (Real scalar);
		Vec4& operator*= (const Vec4& r);
		Vec4 operator* (const Vec4& r) const;
		Vec4& operator/=(Real scalar);
		bool operator== (const Vec4& other) const;
		bool operator!= (const Vec4& other) const;
		Real operator[] (const size_t i) const;
		Real& operator[] (const size_t i);
		operator Vec4Tuple() const;
		
		//-------------------------------------------------------------------------
		Real Dot(const Vec4& other) const;			
		void SetXYZ(const Vec3& v);
		Vec3 GetXYZ() const;
		Vec3 ToVec3() const;
		std::string ToString() const;
	};

#if defined(FB_DOUBLE_PRECISION)
	class Vec4f{
	public:
		float x, y, z, w;

		static const Vec4f ZERO;
		Vec4f();
		Vec4f(Real x_, Real y_, Real z_, Real w_);
		Vec4f(const Vec4& other);
		Vec4f(const Vec3& other, Real w);
		Vec4f& operator=(const Vec4& other);
		Vec4f& operator/=(float s);
		Vec4f& operator*=(float s);
	};
#else
	typedef Vec4 Vec4f;
#endif
}

BOOST_CLASS_IMPLEMENTATION(fb::Vec4, boost::serialization::primitive_type);