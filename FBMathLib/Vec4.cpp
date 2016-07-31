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
#include "Vec4.h"
#include "Math.h"
namespace fb
{
const Vec4 Vec4::UNIT_X(1.f, 0, 0, 0);
const Vec4 Vec4::UNIT_Y(0, 1.f, 0, 0);
const Vec4 Vec4::UNIT_Z(0, 0, 1.f, 0);
const Vec4 Vec4::UNIT_W(0, 0, 0, 1.f);
const Vec4 Vec4::ZERO(0, 0, 0, 0);

//-------------------------------------------------------------------------
Vec4::Vec4() {}
Vec4::Vec4(Real _x, Real _y, Real _z, Real _w)
	: x(_x), y(_y), z(_z), w(_w)
{
}
Vec4::Vec4(const Vec3& xyz, Real _w)
	: x(xyz.x), y(xyz.y), z(xyz.z), w(_w)
{
}

#if defined(FB_DOUBLE_PRECISION)
Vec4::Vec4(const Vec4f& other):
	x(other.x), y(other.y), z(other.z), w(other.w){

}
#endif

Vec4::Vec4(const Vec3& xyz)
	:x(xyz.x), y(xyz.y), z(xyz.z), w(1.f)
{}

Vec4::Vec4(const char* s)
{
	char* next;
	x = 0;
	y = 0;
	z = 0;
	w = 1.0f;
	if (s!=0)
	{
		x = (Real)strtod(s, &next);
		if (next != 0 && next[0] != 0)
		{
			StepToDigit_(&next);
			y = (Real)strtod(next, &next);
			if (next!=0 && next[0] != 0)
			{
				StepToDigit_(&next);
				z = (Real)strtod(next, &next);
				if (next!=0 && next[0] != 0)
				{
					StepToDigit_(&next);
					if (next!=0 && next[0] != 0)
						w = (Real)strtod(next, 0);
				}

			}
		}
	}
}

Vec4::Vec4(const Vec4Tuple& t)
	: x(std::get<0>(t))
	, y(std::get<1>(t))
	, z(std::get<2>(t))
	, w(std::get<3>(t))
{
}

//-------------------------------------------------------------------------
Vec4 Vec4::operator+ (const Vec4& r) const
{
	return Vec4(x + r.x, y + r.y, z + r.z, w + r.w);
}

Vec4 Vec4::operator+(Real v)
{
	return Vec4(v + x, v + y, v + z, v + w);
}

Vec4 Vec4::operator* (Real scalar) const
{
	return Vec4(x * scalar, y * scalar, z * scalar, w * scalar);
}

Vec4& Vec4::operator*= (Real scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	w *= scalar;
	return *this;
}

Vec4& Vec4::operator*= (const Vec4& r)
{
	x *= r.x;
	y *= r.y;
	z *= r.z;
	w *= r.w;
	return *this;
}

Vec4 Vec4::operator* (const Vec4& r) const
{
	return Vec4(x*r.x, y*r.y, z*r.z, w*r.w);
}

Vec4& Vec4::operator/=(Real scalar)
{
	x /= scalar;
	y /= scalar;
	z /= scalar;
	w /= scalar;
	return *this;
}

bool Vec4::operator== (const Vec4& other) const
{
	return (IsEqual(x, other.x) && IsEqual(y, other.y) && IsEqual(z, other.z) && IsEqual(w, other.w));
}

bool Vec4::operator!= (const Vec4& other) const{
	return !operator==(other);
}

Real Vec4::operator[] (const size_t i) const {
	assert(i < 4);

	return *(&x + i);
}

Real& Vec4::operator[] (const size_t i) {
	assert(i < 4);

	return *(&x + i);
}

Vec4::operator Vec4Tuple() const{
	return std::make_tuple(x, y, z, w);
}

//-------------------------------------------------------------------------
Real Vec4::Dot(const Vec4& other) const
{
	return x * other.x + y * other.y + z * other.z + w * other.w;
}

void Vec4::SetXYZ(const Vec3& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
}

Vec3 Vec4::GetXYZ() const{
	return Vec3(x, y, z);
}

Vec3 Vec4::ToVec3() const
{
	return Vec3(x, y, z);
}

std::string Vec4::ToString() const {
	return FormatString("%.2f, %.2f, %.2f, %.2f", x, y, z, w);
}

void write(std::ostream& stream, const Vec4& data) {
	stream.write((char*)&data.x, sizeof(data.x));
	stream.write((char*)&data.y, sizeof(data.y));
	stream.write((char*)&data.z, sizeof(data.z));
	stream.write((char*)&data.w, sizeof(data.w));
}

void read(std::istream& stream, Vec4& data) {
	stream.read((char*)&data.x, sizeof(data.x));
	stream.read((char*)&data.y, sizeof(data.y));
	stream.read((char*)&data.z, sizeof(data.z));
	stream.read((char*)&data.w, sizeof(data.w));
}

#if defined(FB_DOUBLE_PRECISION)
const Vec4f Vec4f::ZERO(0, 0, 0, 0);
Vec4f::Vec4f(){

}
Vec4f::Vec4f(Real x_, Real y_, Real z_, Real w_)
	: x((float)x_)
	, y((float)y_)
	, z((float)z_)
	, w((float)w_)
{

}

Vec4f::Vec4f(const Vec4& other){
	x = (float)other.x;
	y = (float)other.y;
	z = (float)other.z;
	w = (float)other.w;
}

Vec4f::Vec4f(const Vec3& other, Real w){
	x = (float)other.x;
	y = (float)other.y;
	z = (float)other.z;
	w = (float)w;
}

Vec4f& Vec4f::operator = (const Vec4& other){
	x = (float)other.x;
	y = (float)other.y;
	z = (float)other.z;
	w = (float)other.w;
	return *this;
}

Vec4f& Vec4f::operator/=(float s){
	x /= s;
	y /= s;
	z /= s;
	w /= s;
	return *this;
}
Vec4f& Vec4f::operator*=(float s) {
	x *= s;
	y *= s;
	z *= s;
	w *= s;
	return *this;
}
#endif
}