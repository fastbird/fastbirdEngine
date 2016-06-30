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
#include "Quat.h"
#include "Vec3.h"
#include "Math.h"
#include "MurmurHash.h"

namespace fb
{
const Real Quat::ms_fEpsilon = 1e-03f;
const Quat Quat::ZERO(0, 0, 0, 0);
const Quat Quat::IDENTITY(1, 0, 0, 0);

//-------------------------------------------------------------------------
Quat::Quat()
	: w(1), x(0), y(0), z(0)
{
}

Quat::Quat(Real _w, Real _x, Real _y, Real _z)
	: w(_w), x(_x), y(_y), z(_z)
{
}

Quat::Quat(const Mat33& rot)
{
	FromRotationMatrix(rot);
}

Quat::Quat(Real radian, const Vec3& axis)
{
	FromAngleAxis(radian, axis);
}

Quat::Quat(const Vec3& xAxis, const Vec3& yAxis, const Vec3& zAxis)
{
	FromAxes(xAxis, yAxis, zAxis);
}

Quat::Quat(const Vec3& euler) {
	Real sx = sin(euler.x*.5f);
	Real sy = sin(euler.y*.5f);
	Real sz = sin(euler.z*.5f);

	Real cx = cos(euler.x*.5f);
	Real cy = cos(euler.y*.5f);
	Real cz = cos(euler.z*.5f);

	// xyz order
	this->w = cx * cy * cz + sx * sy * sz;
	this->x = sx * cy * cz - cx * sy * sz;
	this->y = cx * sy * cz + sx * cy * sz;
	this->z = cx * cy * sz - sx * sy * cz;
}

Quat::Quat(const QuatTuple& t)
	: w(std::get<0>(t.value))
	, x(std::get<1>(t.value))
	, y(std::get<2>(t.value))
	, z(std::get<3>(t.value))
{
}

//-------------------------------------------------------------------------
Quat Quat::operator-(void) const{
	return Quat(-w, -x, -y, -z);
}

Vec3 Quat::operator* (const Vec3& v) const
{
	// from nVidia SDK
	Vec3 uv, uuv;
	Vec3 qvec(x, y, z);
	uv = qvec.Cross(v);
	uuv = qvec.Cross(uv);
	uv *= (2.0f * w);
	uuv *= 2.0f;

	return v + uv + uuv;
}

Quat Quat::operator* (Real scalar) const
{
	return Quat(scalar*w, scalar*x, scalar*y, scalar*z);
}

Quat Quat::operator* (const Quat& q) const
{
	return Quat
		(
		w * q.w - x * q.x - y * q.y - z * q.z,
		w * q.x + x * q.w + y * q.z - z * q.y,
		w * q.y + y * q.w + z * q.x - x * q.z,
		w * q.z + z * q.w + x * q.y - y * q.x
		);
}

Quat& Quat::operator= (const Quat& other)
{
	w = other.w;
	x = other.x;
	y = other.y;
	z = other.z;
	return *this;
}

bool Quat::operator== (const Quat& rhs) const
{
	return w == rhs.w && x == rhs.x &&
		y == rhs.y && z == rhs.z;
}

bool Quat::operator!= (const Quat& rhs) const
{
	return !operator==(rhs);
}

Real Quat::operator [] (const size_t i) const
{
	assert(i < 4);
	return *(&w + i);
}

Real& Quat::operator[] (const size_t i)
{
	assert(i<4);
	return *(&w + i);
}

Quat::operator QuatTuple() const{
	return QuatTuple(w, x, y, z);
}

//-------------------------------------------------------------------
void Quat::Swap(Quat& other)
{
	std::swap(w, other.w);
	std::swap(x, other.x);
	std::swap(y, other.y);
	std::swap(z, other.z);
}

void Quat::FromRotationMatrix(const Mat33& rot)
{
	// Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
	// article "Quaternion Calculus and Fast Animation".

	Real fTrace = rot[0][0] + rot[1][1] + rot[2][2];
	Real fRoot;

	if (fTrace > 0.0)
	{
		// |w| > 1/2, may as well choose w > 1/2
		fRoot = sqrt(fTrace + 1.0f);  // 2w
		w = 0.5f*fRoot;
		fRoot = 0.5f / fRoot;  // 1/(4w)
		x = (rot[2][1] - rot[1][2])*fRoot;
		y = (rot[0][2] - rot[2][0])*fRoot;
		z = (rot[1][0] - rot[0][1])*fRoot;
	}
	else
	{
		// |w| <= 1/2
		static size_t s_iNext[3] = { 1, 2, 0 };
		size_t i = 0;
		if (rot[1][1] > rot[0][0])
			i = 1;
		if (rot[2][2] > rot[i][i])
			i = 2;
		size_t j = s_iNext[i];
		size_t k = s_iNext[j];

		fRoot = sqrt(rot[i][i] - rot[j][j] - rot[k][k] + 1.0f);
		Real* apkQuat[3] = { &x, &y, &z };
		*apkQuat[i] = 0.5f*fRoot;
		fRoot = 0.5f / fRoot;
		w = (rot[k][j] - rot[j][k])*fRoot;
		*apkQuat[j] = (rot[j][i] + rot[i][j])*fRoot;
		*apkQuat[k] = (rot[k][i] + rot[i][k])*fRoot;
	}
}

void Quat::ToRotationMatrix(Mat33& rot) const
{
	Real fTx = x + x;
	Real fTy = y + y;
	Real fTz = z + z;
	Real fTwx = fTx*w;
	Real fTwy = fTy*w;
	Real fTwz = fTz*w;
	Real fTxx = fTx*x;
	Real fTxy = fTy*x;
	Real fTxz = fTz*x;
	Real fTyy = fTy*y;
	Real fTyz = fTz*y;
	Real fTzz = fTz*z;

	rot[0][0] = 1.0f - (fTyy + fTzz);
	rot[0][1] = fTxy - fTwz;
	rot[0][2] = fTxz + fTwy;
	rot[1][0] = fTxy + fTwz;
	rot[1][1] = 1.0f - (fTxx + fTzz);
	rot[1][2] = fTyz - fTwx;
	rot[2][0] = fTxz - fTwy;
	rot[2][1] = fTyz + fTwx;
	rot[2][2] = 1.0f - (fTxx + fTyy);
}

void Quat::FromAngleAxis (Real radian,
    const Vec3& axis)
{
    Real fHalfAngle = radian * .5f;
    Real fSin = sin(fHalfAngle);
    w = cos(fHalfAngle);
    x = fSin*axis.x;
    y = fSin*axis.y;
    z = fSin*axis.z;
}

Quat Quat::CreateFromAngleAxis(const Real radian, const Vec3& axis) {
	Quat q;
	q.FromAngleAxis(radian, axis);
	return q;
}

void Quat::ToAngleAxis(Real& radian, Vec3& axis)
{
	//   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)
    Real fSqrLength = x*x+y*y+z*z;
    if ( fSqrLength > 0.0 )
    {
        radian = 2.0f * acos(w);
        Real fInvLength = 1.f / sqrt(fSqrLength);
        axis.x = x*fInvLength;
        axis.y = y*fInvLength;
        axis.z = z*fInvLength;
    }
    else
    {
        radian = Radian(0.0);
        axis.x = 1.0;
        axis.y = 0.0;
        axis.z = 0.0;
    }
}

void Quat::FromAxes(const Vec3& xAxis, const Vec3& yAxis, const Vec3& zAxis)
{
	Mat33 rot;

    rot[0][0] = xAxis.x;
    rot[1][0] = xAxis.y;
    rot[2][0] = xAxis.z;

	rot[0][1] = yAxis.x;
    rot[1][1] = yAxis.y;
    rot[2][1] = yAxis.z;

	rot[0][2] = zAxis.x;
    rot[1][2] = zAxis.y;
    rot[2][2] = zAxis.z;

    FromRotationMatrix(rot);
}

void Quat::ToAxes(Vec3& xAxis, Vec3& yAxis, Vec3& zAxis)
{
	Mat33 kRot;

    ToRotationMatrix(kRot);

    xAxis.x = kRot[0][0];
    xAxis.y = kRot[1][0];
    xAxis.z = kRot[2][0];

	yAxis.x = kRot[0][1];
    yAxis.y = kRot[1][1];
    yAxis.z = kRot[2][1];

	zAxis.x = kRot[0][2];
    zAxis.y = kRot[1][2];
    zAxis.z = kRot[2][2];
}

void Quat::FromDirection(const Vec3& dir){
	Real angle = Vec3::UNIT_Y.AngleBetween(dir);
	if (IsEqual(angle, 0.f, 0.001f)){
		*this = IDENTITY;
	}
	else{
		auto axis = Vec3::UNIT_Y.Cross(dir).NormalizeCopy();
		FromAngleAxis(angle, axis);
	}
}

Real Quat::Norm() const
{
	return w*w + x*x + y*y + z*z;
}

Real Quat::Normalise()
{
	Real len = Norm();
	Real factor = 1.f / (Real)sqrt(len);
	*this = *this * factor;
	return len;
}

Quat Quat::Inverse() const
{
	return Quat(w, -x, -y, -z);
}

Real Quat::GetPitch(bool reprojectAxis) const {
	auto radians = atan2((2.f * x * w) - (2.f * y * z),
		1.f - 2.f * x * x - 2.f * z * z);
	return radians;	
}

bool Quat::Equals(const Quat& rhs, const Real toleranceRadian) const
{
	Real fCos = Dot(rhs);
    Real angle = ACos(fCos);

	return (abs(angle) <= toleranceRadian) || IsEqual(angle, PI, toleranceRadian);
}

Real Quat::Dot(const Quat& other) const
{
	return w*other.w+x*other.x+y*other.y+z*other.z; 
}


bool Quat::IsNaN() const
{
		return fb::IsNaN(w) || fb::IsNaN(x) ||
			fb::IsNaN(y) || fb::IsNaN(z);
}

size_t Quat::ComputeHash() const {
	return murmur3_32((const char*)this, sizeof(Quat));
}

void write(std::ostream& stream, const Quat& data){
	stream.write((char*)&data.w, sizeof(Real));
	stream.write((char*)&data.x, sizeof(Real));
	stream.write((char*)&data.y, sizeof(Real));
	stream.write((char*)&data.z, sizeof(Real));
}

void read(std::istream& stream, Quat& data){
	stream.read((char*)&data.w, sizeof(Real));
	stream.read((char*)&data.x, sizeof(Real));
	stream.read((char*)&data.y, sizeof(Real));
	stream.read((char*)&data.z, sizeof(Real));
}

}

//-----------------------------------------------------------------------
std::istream& operator>>(std::istream& stream, fb::Quat& v)
{
	stream >> v.w >> v.x >> v.y >> v.z;
	return stream;
}

std::ostream& operator<<(std::ostream& stream, const fb::Quat& v)
{
	stream << v.w << v.x << v.y << v.z;
	return stream;
}