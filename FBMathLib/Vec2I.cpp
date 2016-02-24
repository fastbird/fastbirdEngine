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

#include "stdafx.h"
#include "Vec2I.h"
#include "Math.h"

namespace fb
{
	const Vec2I Vec2I::ZERO(0, 0);

	//-------------------------------------------------------------------
	Vec2I::Vec2I()
	{
	}

	Vec2I::Vec2I(int _x, int _y)
		: x(_x), y(_y)
	{
	}

	Vec2I::Vec2I(const Vec2& v){
		x = Round(v.x);
		y = Round(v.y);
	}	

	Vec2I::Vec2I(const Vec2ITuple& t)
		: x(std::get<0>(t))
		, y(std::get<1>(t))
	{
	}

	//-------------------------------------------------------------------
	Vec2I Vec2I::operator+ (int s) const
	{
		return Vec2I(x + s, y + s);
	}

	Vec2I Vec2I::operator+ (const Vec2I& v) const
	{
		return Vec2I(x + v.x, y + v.y);
	}

	Vec2I& Vec2I::operator+=(const Vec2I& v)
	{
		x += v.x;
		y += v.y;
		return *this;
	}

	Vec2I Vec2I::operator- (int s) const
	{
		return Vec2I(x - s, y - s);
	}

	Vec2I Vec2I::operator- (const Vec2I& v) const
	{
		return Vec2I(x - v.x, y - v.y);
	}

	Vec2I& Vec2I::operator-=(const Vec2I& v)
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}

	Vec2I Vec2I::operator* (int s) const
	{
		return Vec2I(x*s, y*s);
	}

	Vec2I Vec2I::operator* (const Vec2I& v) const
	{
		return Vec2I(x*v.x, y*v.y);
	}

	Vec2I Vec2I::operator/ (int s) const
	{
		return Vec2I(x / s, y / s);
	}	

	Vec2I Vec2I::operator/ (const Vec2I& v) const
	{
		return Vec2I(x / v.x, y / v.y);
	}

	bool Vec2I::operator== (const Vec2I& v) const
	{
		return x == v.x && y == v.y;
	}

	bool Vec2I::operator!=(const Vec2I& v) const
	{
		return !operator==(v);
	}

	bool Vec2I::operator<(const Vec2I& v) const
	{
		if (x < v.x)
		{
			return true;
		}
		else if (x == v.x)
		{
			if (y<v.y)
			{
				return true;
			}
		}

		return false;
	}

	Vec2I::operator Vec2ITuple() const{
		return std::make_tuple(x, y);
	}

	int Vec2I::Cross(const Vec2I& right){
		return x * right.y - y * right.x;
	}
	int Vec2I::Dot(const Vec2I& right){
		return x * right.x + y * right.y;
	}
	
	Real Vec2I::DistanceTo(const Vec2I& d) const{
		return (*this - d).Length();
	}

	Real Vec2I::Length() const{
		return (Real)std::sqrt(x*x + y*y);
	}

	Vec2I Vec2I::Scale(Real f) const {
		return Vec2I(Round(x*f), Round(y*f));
	}

	Vec2I operator*(const Vec2I& left, unsigned right) {
		return Vec2I(left.x * right, left.y * right);
	}
}