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
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/level.hpp>
namespace fb
{
	class Vec3I
	{
	public:
		int x, y, z;

		static const Vec3I UNIT_X;
		static const Vec3I UNIT_Y;
		static const Vec3I UNIT_Z;
		static const Vec3I ZERO;
		static const Vec3I MAX;

		//-------------------------------------------------------------------
		Vec3I();
		Vec3I(int _x, int _y, int _z);
		explicit Vec3I(const Vec3& v);
		Vec3I(const Vec3ITuple& t);

		//-------------------------------------------------------------------
		Vec3I operator+ (int s) const;
		Vec3I operator+ (const Vec3I& v) const;
		Vec3I operator- (int s) const;
		Vec3I operator- (const Vec3I& v) const;
		Vec3I operator* (int s) const;
		Vec3 operator*(Real s) const;
		Vec3I operator* (const Vec3I& v) const;
		Vec3I operator/ (int s) const;		
		Vec3I operator/ (const Vec3I& v) const;		
		bool operator== (const Vec3I& v) const;
		bool operator!=(const Vec3I& v) const;
		bool operator<(const Vec3I& other) const;
		operator Vec3ITuple() const;

		//-------------------------------------------------------------------
		Real length() const;
		Real lengthSQ() const;
		Real distance(const Vec3I& to) const;
		Real distanceSQ(const Vec3I& to) const;
	};
}

BOOST_CLASS_IMPLEMENTATION(fb::Vec3I, boost::serialization::primitive_type);