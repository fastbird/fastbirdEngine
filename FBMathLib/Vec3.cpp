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
#include "Vec3.h"
#include "Vec3I.h"
#include "Mat33.h"
#include "Math.h"
#include "Vec3d.h"
#include "FBStringLib/MurmurHash.h"

namespace fb
{
	const Vec3 Vec3::UNIT_X(1.f, 0, 0);
	const Vec3 Vec3::UNIT_Y(0, 1.f, 0);
	const Vec3 Vec3::UNIT_Z(0, 0, 1.f);
	const Vec3 Vec3::ZERO(0, 0, 0);
	const Vec3 Vec3::ONE(1.f, 1.f, 1.f);
	const Vec3 Vec3::MAX(FLT_MAX, FLT_MAX, FLT_MAX);
	const Vec3 Vec3::MIN(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	//-------------------------------------------------------------------
	Vec3::Vec3() 
		: x(FB_INVALID_REAL)
	{
	}

	Vec3::Vec3(Real _x, Real _y, Real _z)
		: x(_x), y(_y), z(_z)
	{
	}

	Vec3::Vec3(const Vec2& v2, Real _z)
	{
		x = v2.x;
		y = v2.y;
		z = _z;
	}

	Vec3::Vec3(Real s)
		: x(s), y(s), z(s)
	{
	}

	Vec3::Vec3(const Vec3I& v)
	{
		x = (Real)v.x;
		y = (Real)v.y;
		z = (Real)v.z;
	}

	Vec3::Vec3(const char* s)
	{
		char* next;
		x = 0;
		y = 0;
		z = 0;
		if (s != 0)
		{
			x = (Real)strtod(s, &next);
			if (next != 0)
			{
				StepToDigit_(&next);
				y = (Real)strtod(next, &next);
				if (next != 0)
				{
					StepToDigit_(&next);
					z = (Real)strtod(next, 0);
				}
			}
		}
	}

	Vec3::Vec3(const Vec3Tuple& t)
		: x(std::get<0>(t))
		, y(std::get<1>(t))
		, z(std::get<2>(t))
	{
	}

	Vec3::Vec3(const Vec3d& v)
		: x((Real)v.x)
		, y((Real)v.y)
		, z((Real)v.z)
	{
	}


	//-------------------------------------------------------------------

	bool Vec3::IsInitialized() const
	{
		return x != FB_INVALID_REAL;
	}
	Vec3 Vec3::operator+ (const Vec3& r) const
	{
		return Vec3(x + r.x, y + r.y, z + r.z);
	}

	Vec3 Vec3::operator+ (Real f) const
	{
		return Vec3(x + f, y + f, z + f);
	}
	Vec3& Vec3::operator+= (const Vec3& r)
	{
		x += r.x;
		y += r.y;
		z += r.z;
		return *this;
	}

	Vec3& Vec3::operator+= (Real r) {
		x += r;
		y += r;
		z += r;
		return *this;
	}

	Vec3 Vec3::operator- () const
	{
		return Vec3(-x, -y, -z);
	}

	Vec3 Vec3::operator- (const Vec3& r) const
	{
		return Vec3(x - r.x, y - r.y, z - r.z);
	}

	Vec3 Vec3::operator- (Real f) const{
		return Vec3(x - f, y - f, z - f);
	}

	Vec3& Vec3::operator-= (const Vec3& r)
	{
		x -= r.x;
		y -= r.y;
		z -= r.z;
		return *this;
	}

	Vec3& Vec3::operator-= (const Real f)
	{
		x -= f;
		y -= f;
		z -= f;
		return *this;
	}

	Vec3 Vec3::operator* (Real scalar) const
	{
		return Vec3(x * scalar, y * scalar, z * scalar);
	}

	Vec3 Vec3::operator* (const Vec3& v) const
	{
		return Vec3(x * v.x, y* v.y, z*v.z);
	}

	Vec3 Vec3::operator* (const Mat33& r) const
	{
		return Vec3(x * r[0][0] + y * r[1][0] + z * r[2][0],
			x * r[0][1] + y * r[1][1] + z * r[2][1],
			x * r[0][2] + y * r[1][2] + z * r[2][2]);
	}

	Vec3& Vec3::operator*= (const Vec3& r)
	{
		x *= r.x;
		y *= r.y;
		z *= r.z;
		return *this;
	}

	Vec3& Vec3::operator*= (Real f)
	{
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}

	Vec3 Vec3::operator/ (Real scalar) const
	{
		auto invScalar = 1.f / scalar;
		return Vec3(x * invScalar, y *invScalar, z *invScalar);
	}

	Vec3 Vec3::operator/ (const Vec3& v) const
	{
		return Vec3(x / v.x, y / v.y, z / v.z);
	}

	Vec3& Vec3::operator/= (const Vec3& r)
	{
		x /= r.x;
		y /= r.y;
		z /= r.z;
		return *this;
	}

	Vec3& Vec3::operator/= (Real s)
	{
		x /= s;
		y /= s;
		z /= s;
		return *this;
	}

	bool Vec3::operator == (const Vec3& r) const
	{
		return (abs(x - r.x) < EPSILON &&
			abs(y - r.y) < EPSILON &&
			abs(z - r.z) < EPSILON);
	}
	bool Vec3::operator != (const Vec3& r) const
	{
		return !operator==(r);
	}

	bool Vec3::operator< (const Vec3& other) const
	{
		if (x < other.x)
		{
			return true;
		}
		else if (IsEqual(x, other.x, EPSILON))
		{
			if (y<other.y)
			{
				return true;
			}
			else if (IsEqual(y, other.y, EPSILON))
			{
				return z < other.z;
			}
		}

		return false;
	}

	bool Vec3::operator >= (const Vec3& other) const
	{
		return x >= other.x && y >= other.y && z >= other.z;
	}

	bool Vec3::operator <= (const Vec3& other) const
	{
		return x <= other.x && y <= other.y && z <= other.z;
	}

	Real Vec3::operator[] (const size_t i) const
	{
		assert(i < 3);

		return *(&x + i);
	}

	Real& Vec3::operator[] (const size_t i)
	{
		assert(i < 3);

		return *(&x + i);
	}

	Vec3::operator Vec3Tuple() const{
		return std::make_tuple(x, y, z);
	}

	Vec3 Vec3::xyz()
	{
		return *this;
	}

	Vec3 Vec3::yxy()
	{
		return Vec3(y, x, y);
	}

	Vec3 Vec3::zzx()
	{
		return Vec3(z, z, x);
	}

	Vec2 Vec3::xy(){
		return Vec2(x, y);
	}
	
	//-------------------------------------------------------------------
	Real Vec3::Dot(const Vec3& vec) const
	{
		return x * vec.x + y * vec.y + z * vec.z;
	}

	Real Vec3::DistanceTo(const Vec3& other) const
	{
		return (*this - other).Length();
	}

	Real Vec3::DistanceToSQ(const Vec3&other) const
	{
		return (*this - other).LengthSQ();
	}

	void Vec3::KeepGreater(const Vec3& other)
	{
		if (other.x > x) x = other.x;
		if (other.y > y) y = other.y;
		if (other.z > z) z = other.z;
	}

	void Vec3::KeepLesser(const Vec3& other)
	{
		if (other.x < x) x = other.x;
		if (other.y < y) y = other.y;
		if (other.z < z) z = other.z;
	}
	
	Vec3 Vec3::Cross(const Vec3& rVector) const
	{
		return Vec3(
			y * rVector.z - z * rVector.y,
			z * rVector.x - x * rVector.z,
			x * rVector.y - y * rVector.x);
	}

	int Vec3::MaxAxis() const
	{
		return x < y ? (y < z ? 2 : 1) : (x <z ? 2 : 0);
	}

	void Vec3::SafeNormalize()
	{
		Vec3 absVec(Abs(*this));

		int maxIndex = absVec.MaxAxis();
		if (absVec[maxIndex]>0)
		{
			*this /= absVec[maxIndex];
			*this /= Length();
			return;
		}
		*this = Vec3(1, 0, 0);

	}

	Real Vec3::Normalize()
	{
		Real length = sqrt(x*x + y*y + z*z);
		if (length > 0.0f)
		{
			Real invLength = 1.f / length;
			x *= invLength;
			y *= invLength;
			z *= invLength;
		}

		return length;
	}

	Vec3 Vec3::NormalizeCopy() const
	{
		Vec3 result = *this;
		result.Normalize();
		return result;
	}

	Real Vec3::Length() const
	{
		return sqrt(x*x + y*y + z*z);
	}

	Real Vec3::LengthSQ() const
	{
		return x*x + y*y + z*z;
	}

	Real Vec3::AngleBetween(const Vec3& v) const
	{
		Real lenProduct = Length() * v.Length();

		// Prevent dividing zero
		if (lenProduct < 1e-6f)
			lenProduct = 1e-6f;

		Real f = Dot(v) / lenProduct;

		Clamp(f, (Real)-1.0, (Real)1.0);
		return ACos(f);

	}	

	Real Vec3::GetMax() const{
		return std::max(std::max(x, y), z);
	}

	std::string Vec3::ToString() const {
		return FormatString("%f %f %f", x, y, z);
	}

	Vec2 Vec3::GetXY() const {
		return Vec2(x, y);
	}

	Vec3 Vec3::GetInv() const {
		return Vec3(1.f / x, 1.f / y, 1.f / z);
	}

	size_t Vec3::ComputeHash() const {
		return murmur3_32((const char*)this, sizeof(Vec3));
	}

	bool Vec3::IsSpecialFloat() const {
		return isnan(x) || isnan(y) || isnan(z) || isinf(x) || isinf(y) || isinf(z);
	}

	//--------------------------------------------------------------------------
	Vec3 operator* (Real l, const Vec3& r)
	{
		return Vec3(r.x*l, r.y*l, r.z*l);
	}

	bool IsEqual(const Vec3& l, const Vec3& r, Real ep/* = EPSILON*/)
	{
		Vec3 t = l - r;
		if (abs(t.x) > ep || abs(t.y) > ep || abs(t.z) > ep)
			return false;

		return true;
	}

	bool IsEqual(const Vec2& l, const Vec2& r, Real ep/* = EPSILON*/)
	{
		Vec2 t = l - r;
		if (abs(t.x) > ep || abs(t.y) > ep)
			return false;

		return true;
	}

	Vec3 Sign(const Vec3& v)
	{
		return Vec3(v.x < 0.0f ? -1 : (v.x == 0.0f ? 0.0f : 1.0f),
			v.y < 0.0f ? -1 : (v.y == 0.0f ? 0.0f : 1.0f),
			v.z < 0.0f ? -1 : (v.z == 0.0f ? 0.0f : 1.0f));
	}

	Vec3 Floor(const Vec3& v)
	{
		return Vec3(floor(v.x), floor(v.y), floor(v.z));
	}

	Vec3 Step(const Vec3& edge, const Vec3& v)
	{
		return Vec3(edge.x > v.x ? 0.0f : 1.0f,
			edge.y > v.y ? 0.0f : 1.0f,
			edge.z > v.z ? 0.0f : 1.0f);
	}

#if defined(FB_DOUBLE_PRECISION)
	Vec3f::Vec3f(){

	}

	Vec3f::Vec3f(Real x_, Real y_, Real z_)
		: x((float)x_)
		, y((float)y_)
		, z((float)z_)
	{

	}

	Vec3f& Vec3f::operator = (const Vec3& other){
		x = (float)other.x;
		y = (float)other.y;
		z = (float)other.z;
		return *this;
	}
#endif
}