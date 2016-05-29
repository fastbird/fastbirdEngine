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
#include "Vec3I.h"

namespace fb
{
	const Vec3I Vec3I::UNIT_X = Vec3I(1, 0, 0);
	const Vec3I Vec3I::UNIT_Y = Vec3I(0, 1, 0);
	const Vec3I Vec3I::UNIT_Z = Vec3I(0, 0, 1);
	const Vec3I Vec3I::ZERO = Vec3I(0, 0, 0);
	const Vec3I Vec3I::MAX = Vec3I(INT_MAX, INT_MAX, INT_MAX);

	//-------------------------------------------------------------------
	Vec3I::Vec3I()
	{
	}

	Vec3I::Vec3I(int _x, int _y, int _z)
		: x(_x), y(_y), z(_z)
	{
	}

	Vec3I::Vec3I(const Vec3& v)
	{
		x = (int)v.x;
		y = (int)v.y;
		z = (int)v.z;
	}

	Vec3I::Vec3I(const Vec3ITuple& t)
		: x(std::get<0>(t))
		, y(std::get<1>(t))
		, z(std::get<2>(t))
	{
	}

	//-------------------------------------------------------------------
	Vec3I Vec3I::operator+ (int s) const
	{
		return Vec3I(x + s, y + s, z + s);
	}

	Vec3I Vec3I::operator+ (const Vec3I& v) const
	{
		return Vec3I(x + v.x, y + v.y, z + v.z);
	}

	Vec3I Vec3I::operator- (int s) const
	{
		return Vec3I(x - s, y - s, y - s);
	}

	Vec3I Vec3I::operator- (const Vec3I& v) const
	{
		return Vec3I(x - v.x, y - v.y, z - v.z);
	}

	Vec3I Vec3I::operator* (int s) const
	{
		return Vec3I(x*s, y*s, z*s);
	}

	Vec3 Vec3I::operator*(Real s) const
	{
		return Vec3(x*s, y*s, z*s);
	}

	Vec3I Vec3I::operator* (const Vec3I& v) const
	{
		return Vec3I(x*v.x, y*v.y, z*v.z);
	}

	Vec3I Vec3I::operator/ (int s) const
	{
		return Vec3I(x / s, y / s, z / s);
	}	

	Vec3I Vec3I::operator/ (const Vec3I& v) const
	{
		return Vec3I(x / v.x, y / v.y, z / v.z);
	}

	bool Vec3I::operator== (const Vec3I& v) const
	{
		return x == v.x && y == v.y && z == v.z;
	}

	bool Vec3I::operator!=(const Vec3I& v) const
	{
		return !operator==(v);
	}

	bool Vec3I::operator<(const Vec3I& other) const
	{
		return memcmp(this, &other, sizeof(Vec3I)) < 0;
	}

	Vec3I::operator Vec3ITuple() const{
		return std::make_tuple(x, y, z);
	}

	//-------------------------------------------------------------------
	Real Vec3I::length() const
	{
		return (Real)sqrt(x*x + y*y + z*z);
	}

	Real Vec3I::lengthSQ() const
	{
		return (Real)(x*x + y*y + z*z);
	}

	Real Vec3I::distance(const Vec3I& to) const{
		Vec3I dif = to - *this;
		return dif.length();
	}

	Real Vec3I::distanceSQ(const Vec3I& to) const{
		Vec3I dif = to - *this;
		return dif.lengthSQ();
	}

	void write(std::ostream& stream, const Vec3I& data) {
		stream.write((char*)&data.x, sizeof(data.x));
		stream.write((char*)&data.y, sizeof(data.y));
		stream.write((char*)&data.z, sizeof(data.z));
	}

	void read(std::istream& stream, Vec3I& data){
		stream.read((char*)&data.x, sizeof(data.x));
		stream.read((char*)&data.y, sizeof(data.y));
		stream.read((char*)&data.z, sizeof(data.z));
	}
	/*void write(std::ostream& stream, fb::Vec3I& v){
		stream.write((char*)&v.x, sizeof(v.x));
		stream.write((char*)&v.y, sizeof(v.y));
		stream.write((char*)&v.z, sizeof(v.z));
	}

	void read(std::istream& stream, fb::Vec3I& v){
		stream.read((char*)&v.x, sizeof(v.x));
		stream.read((char*)&v.y, sizeof(v.y));
		stream.read((char*)&v.z, sizeof(v.z));
	}*/
}

std::istream& operator>>(std::istream& stream, fb::Vec3I& v)
{
	stream >> v.x >> v.y >> v.z;
	return stream;
}

std::ostream& operator<<(std::ostream& stream, const fb::Vec3I& v)
{
	stream << v.x << v.y << v.z;
	return stream;
}