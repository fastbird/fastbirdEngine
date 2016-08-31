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
#include "Mat33.h"
#include "FBStringLib/MurmurHash.h"

using namespace fb;

const Mat33 Mat33::IDENTITY(1, 0, 0, 0, 1, 0, 0, 0, 1);
const Mat33 Mat33::ZERO(0, 0, 0, 0, 0, 0, 0, 0, 0);

//----------------------------------------------------------------------------
Mat33::Mat33()	{
}

Mat33::Mat33(Real m11, Real m12, Real m13,
	Real m21, Real m22, Real m23,
	Real m31, Real m32, Real m33) {
	m[0][0] = m11;			m[0][1] = m12;			m[0][2] = m13;
	m[1][0] = m21;			m[1][1] = m22;			m[1][2] = m23;
	m[2][0] = m31;			m[2][1] = m32;			m[2][2] = m33;
}

Mat33::Mat33(const Vec3& x, const Vec3& y, const Vec3& z) {
	SetColumn(0, x);
	SetColumn(1, y);
	SetColumn(2, z);
}

//----------------------------------------------------------------------------
Mat33 Mat33::operator-() const
{
	Mat33 ret;
	for (size_t row = 0; row<3; row++)
	{
		for (size_t col = 0; col<3; col++)
		{
			ret[row][col] = -m[row][col];
		}
	}
	return ret;
}

Mat33 Mat33::operator* (const Mat33& rMat) const
{
	Mat33 result;
	for (size_t iRow = 0; iRow < 3; iRow++)
	{
		for (size_t iCol = 0; iCol < 3; iCol++)
		{
			result.m[iRow][iCol] =
				m[iRow][0] * rMat.m[0][iCol] +
				m[iRow][1] * rMat.m[1][iCol] +
				m[iRow][2] * rMat.m[2][iCol];
		}
	}
	return result;
}

Vec3 Mat33::operator* (const Vec3& rVec) const
{
	Vec3 result;
	for (size_t iRow = 0; iRow < 3; iRow++)
	{
		result[iRow] =
			m[iRow][0] * rVec[0] +
			m[iRow][1] * rVec[1] +
			m[iRow][2] * rVec[2];
	}
	return result;
}

void Mat33::operator/=(Real k)
{
	Real invK = 1.0f / k;
	m[0][0] *= invK;	m[0][1] *= invK;	m[0][2] *= invK;
	m[1][0] *= invK;	m[1][1] *= invK;	m[1][2] *= invK;
	m[2][0] *= invK;	m[2][1] *= invK;	m[2][2] *= invK;
}

Mat33 Mat33::operator/ (Real k) const
{
	Real invK = 1.0f / k;
	return Mat33(m[0][0] * invK, m[0][1] * invK, m[0][2] * invK,
		m[1][0] * invK, m[1][1] * invK, m[1][2] * invK,
		m[2][0] * invK, m[2][1] * invK, m[2][2] * invK);
}

Real* Mat33::operator[](size_t iRow) const
{
	return (Real*)m[iRow];
}

bool Mat33::operator==(const Mat33& other) const
{
	return memcmp(m, other.m, sizeof(36)) == 0;
}

//----------------------------------------------------------------------------
void Mat33::MakeIdentity()
{
	m[0][0] = 1.f;			m[0][1] = 0.f;			m[0][2] = 0.f;
	m[1][0] = 0.f;			m[1][1] = 1.f;			m[1][2] = 0.f;
	m[2][0] = 0.f;			m[2][1] = 0.f;			m[2][2] = 1.f;
}

void Mat33::FromAxisAngle(const Vec3& axis, Real radian)
{
	Real fCos = cos(radian);
  Real fSin = sin(radian);
  Real fOneMinusCos = 1.0f-fCos;
  Real fX2 = axis.x*axis.x;
  Real fY2 = axis.y*axis.y;
  Real fZ2 = axis.z*axis.z;
  Real fXYM = axis.x*axis.y*fOneMinusCos;
  Real fXZM = axis.x*axis.z*fOneMinusCos;
  Real fYZM = axis.y*axis.z*fOneMinusCos;
  Real fXSin = axis.x*fSin;
  Real fYSin = axis.y*fSin;
  Real fZSin = axis.z*fSin;

  m[0][0] = fX2*fOneMinusCos+fCos;
  m[0][1] = fXYM-fZSin;
  m[0][2] = fXZM+fYSin;
  m[1][0] = fXYM+fZSin;
  m[1][1] = fY2*fOneMinusCos+fCos;
  m[1][2] = fYZM-fXSin;
  m[2][0] = fXZM-fYSin;
  m[2][1] = fYZM+fXSin;
  m[2][2] = fZ2*fOneMinusCos+fCos;
}

Mat33 Mat33::FromAxisAngleStatic(const Vec3& axis, Real radian)
{
	Mat33 m;
	m.FromAxisAngle(axis, radian);
	return m;
}

Mat33 Mat33::Inverse() const
{
    Mat33 inversed;

    inversed.m[0][0] = m[1][1]*m[2][2] - m[1][2]*m[2][1];
    inversed.m[0][1] = m[0][2]*m[2][1] - m[0][1]*m[2][2];
    inversed.m[0][2] = m[0][1]*m[1][2] - m[0][2]*m[1][1];
    inversed.m[1][0] = m[1][2]*m[2][0] - m[1][0]*m[2][2];
    inversed.m[1][1] = m[0][0]*m[2][2] - m[0][2]*m[2][0];
    inversed.m[1][2] = m[0][2]*m[1][0] - m[0][0]*m[1][2];
    inversed.m[2][0] = m[1][0]*m[2][1] - m[1][1]*m[2][0];
    inversed.m[2][1] = m[0][1]*m[2][0] - m[0][0]*m[2][1];
    inversed.m[2][2] = m[0][0]*m[1][1] - m[0][1]*m[1][0];

    Real fDet = m[0][0]*inversed[0][0] + m[0][1]*inversed[1][0]+
        m[0][2]*inversed[2][0];

    if (abs(fDet) <= ZERO_TOLERANCE)
    {
        return ZERO;
    }

    inversed /= fDet;
    return inversed;
}

Mat33 Mat33::Transpose() const
{
	return Mat33(	m[0][0], m[1][0], m[2][0],
					m[0][1], m[1][1], m[2][1],
					m[0][2], m[1][2], m[2][2] );
}

Mat33 Mat33::ScaleAxis(const Vec3& scale) const
{
	return Mat33(	m[0][0] * scale.x, m[0][1] * scale.y, m[0][2] * scale.z,
					m[1][0] * scale.x, m[1][1] * scale.y, m[1][2] * scale.z,
					m[2][0] * scale.x, m[2][1] * scale.y, m[2][2] * scale.z	);
}

Vec3 Mat33::Column(int index) const
{
	Vec3 ret(m[0][index], m[1][index], m[2][index]);
	return ret;
}

void Mat33::SetColumn(int index, const Vec3& v)
{
	m[0][index] = v.x;
	m[1][index] = v.y;
	m[2][index] = v.z;
}

bool Mat33::IsSymmetric() const {
	return m[0][1] == m[1][0] && m[0][2] == m[2][0] && m[1][2] == m[2][1];
}

size_t Mat33::ComputeHash() const {
	return (size_t)murmur3_32((const char*)this, sizeof(Mat33));
}