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
#include "Mat44.h"
#include "Mat33.h"
#include "Vec3.h"
#include "Vec4.h"
#include "Math.h"

namespace fb
{
	const Mat44 Mat44::IDENTITY(1, 0, 0, 0,
								0, 1, 0, 0,
								0, 0, 1, 0,
								0, 0, 0, 1);

	//-------------------------------------------------------------------------
	Mat44::Mat44() {
	}

	Mat44::Mat44(Real m11, Real m12, Real m13, Real m14,
		Real m21, Real m22, Real m23, Real m24,
		Real m31, Real m32, Real m33, Real m34,
		Real m41, Real m42, Real m43, Real m44)
	{
		m[0][0] = m11; m[0][1] = m12; m[0][2] = m13; m[0][3] = m14;
		m[1][0] = m21; m[1][1] = m22; m[1][2] = m23; m[1][3] = m24;
		m[2][0] = m31; m[2][1] = m32; m[2][2] = m33; m[2][3] = m34;
		m[3][0] = m41; m[3][1] = m42; m[3][2] = m43; m[3][3] = m44;
	}

	Mat44::Mat44(const Mat33& mat33, const Vec3& translation)
	{
		m[0][0] = mat33[0][0]; m[0][1] = mat33[0][1]; m[0][2] = mat33[0][2]; m[0][3] = translation.x;
		m[1][0] = mat33[1][0]; m[1][1] = mat33[1][1]; m[1][2] = mat33[1][2]; m[1][3] = translation.y;
		m[2][0] = mat33[2][0]; m[2][1] = mat33[2][1]; m[2][2] = mat33[2][2]; m[2][3] = translation.z;
		m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1.f;
	}

	//-------------------------------------------------------------------------
	Mat44 Mat44::FromTranslation(const Vec3& translate) 
	{
		Mat44 mat(
			1.f,	0,		0,		translate.x,
			0,		1.f,	0,		translate.y,
			0,		0,		1.f,	translate.z,
			0,		0,		0,		1.f
			);
		return mat;
	}

	//-------------------------------------------------------------------------
	Mat44 Mat44::FromRotationY(Real rotYRadian)
	{
		auto c = cos(rotYRadian);
		auto s = sin(rotYRadian);
		Mat44 m(
			c,			0.f,			s,			0.f,
			0.f,		1.f,			0.f,		0.f,
			-s,			0.f,			c,			0.f,
			0.f,		0.f,			0.f,		1.f
			);
		return m;
	}

	Mat44 Mat44::FromRotationX(Real rotXRadian) {
		auto c = cos(rotXRadian);
		auto s = sin(rotXRadian);
		return Mat44 (
			1.f,			0.f,		0.f,		0.f,
			0.f,			c,			-s,			0.f,
			0.f,			s,			c,			0.f,
			0.f,			0.f,		0.f,		1.f
			);
	}

	Mat44 Mat44::FromRotationZ(Real rotXRadian) {
		auto c = cos(rotXRadian);
		auto s = sin(rotXRadian);
		return Mat44(
			c, -s, 0.0, 0.0,
			s, c, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0,
			0.0, 0.0, 0.0, 1.0
			);
	}

	Mat44 Mat44::FromAxisAngle(const Vec3& axis, Real radian) {
		Real fCos = cos(radian);
		Real fSin = sin(radian);
		Real fOneMinusCos = 1.0f - fCos;
		Real fX2 = axis.x*axis.x;
		Real fY2 = axis.y*axis.y;
		Real fZ2 = axis.z*axis.z;
		Real fXYM = axis.x*axis.y*fOneMinusCos;
		Real fXZM = axis.x*axis.z*fOneMinusCos;
		Real fYZM = axis.y*axis.z*fOneMinusCos;
		Real fXSin = axis.x*fSin;
		Real fYSin = axis.y*fSin;
		Real fZSin = axis.z*fSin;

		return Mat44(
			// Row 1
			fX2*fOneMinusCos + fCos,
			fXYM - fZSin,
			fXZM + fYSin,
			0.0,
			// Row 2
			fXYM + fZSin,
			fY2*fOneMinusCos + fCos,
			fYZM - fXSin,
			0.0,
			// Row 3
			fXZM - fYSin,
			fYZM + fXSin,
			fZ2*fOneMinusCos + fCos,
			0.0,
			// Row 4
			0.0, 0.0, 0.0, 1.0);
	}

	Mat44 Mat44::FromViewLookAt(const Vec3& eye, const Vec3& center, const Vec3& up)
	{
		return MakeViewMatrix(eye, center, up);
	}

	//-------------------------------------------------------------------------
	Vec4 Mat44::operator*(const Vec4& columnVector) const
	{
		return Vec4(
			Row(0).Dot(columnVector),
			Row(1).Dot(columnVector),
			Row(2).Dot(columnVector),
			Row(3).Dot(columnVector));
	}

	// Initial w = 1
	// Transforms the given vector and project to the w=1	
	Vec3 Mat44::operator*(const Vec3& columnVector) const
	{
		Vec3 result;
		Real invW = 1.f / (m[3][0] * columnVector.x + m[3][1] * columnVector.y + m[3][2] * columnVector.z + m[3][3]);
		result.x = (m[0][0] * columnVector.x + m[0][1] * columnVector.y + m[0][2] * columnVector.z + m[0][3]) * invW;
		result.y = (m[1][0] * columnVector.x + m[1][1] * columnVector.y + m[1][2] * columnVector.z + m[1][3]) * invW;
		result.z = (m[2][0] * columnVector.x + m[2][1] * columnVector.y + m[2][2] * columnVector.z + m[2][3]) * invW;

		return result;
	}

	Mat44 Mat44::operator* (const Mat44& other) const
	{
		Mat44 r;
		r.m[0][0] = m[0][0] * other.m[0][0] + m[0][1] * other.m[1][0] + m[0][2] * other.m[2][0] + m[0][3] * other.m[3][0];
		r.m[0][1] = m[0][0] * other.m[0][1] + m[0][1] * other.m[1][1] + m[0][2] * other.m[2][1] + m[0][3] * other.m[3][1];
		r.m[0][2] = m[0][0] * other.m[0][2] + m[0][1] * other.m[1][2] + m[0][2] * other.m[2][2] + m[0][3] * other.m[3][2];
		r.m[0][3] = m[0][0] * other.m[0][3] + m[0][1] * other.m[1][3] + m[0][2] * other.m[2][3] + m[0][3] * other.m[3][3];

		r.m[1][0] = m[1][0] * other.m[0][0] + m[1][1] * other.m[1][0] + m[1][2] * other.m[2][0] + m[1][3] * other.m[3][0];
		r.m[1][1] = m[1][0] * other.m[0][1] + m[1][1] * other.m[1][1] + m[1][2] * other.m[2][1] + m[1][3] * other.m[3][1];
		r.m[1][2] = m[1][0] * other.m[0][2] + m[1][1] * other.m[1][2] + m[1][2] * other.m[2][2] + m[1][3] * other.m[3][2];
		r.m[1][3] = m[1][0] * other.m[0][3] + m[1][1] * other.m[1][3] + m[1][2] * other.m[2][3] + m[1][3] * other.m[3][3];

		r.m[2][0] = m[2][0] * other.m[0][0] + m[2][1] * other.m[1][0] + m[2][2] * other.m[2][0] + m[2][3] * other.m[3][0];
		r.m[2][1] = m[2][0] * other.m[0][1] + m[2][1] * other.m[1][1] + m[2][2] * other.m[2][1] + m[2][3] * other.m[3][1];
		r.m[2][2] = m[2][0] * other.m[0][2] + m[2][1] * other.m[1][2] + m[2][2] * other.m[2][2] + m[2][3] * other.m[3][2];
		r.m[2][3] = m[2][0] * other.m[0][3] + m[2][1] * other.m[1][3] + m[2][2] * other.m[2][3] + m[2][3] * other.m[3][3];

		r.m[3][0] = m[3][0] * other.m[0][0] + m[3][1] * other.m[1][0] + m[3][2] * other.m[2][0] + m[3][3] * other.m[3][0];
		r.m[3][1] = m[3][0] * other.m[0][1] + m[3][1] * other.m[1][1] + m[3][2] * other.m[2][1] + m[3][3] * other.m[3][1];
		r.m[3][2] = m[3][0] * other.m[0][2] + m[3][1] * other.m[1][2] + m[3][2] * other.m[2][2] + m[3][3] * other.m[3][2];
		r.m[3][3] = m[3][0] * other.m[0][3] + m[3][1] * other.m[1][3] + m[3][2] * other.m[2][3] + m[3][3] * other.m[3][3];
		return r;
	}

	Mat44& Mat44::operator*=(const Mat44& other)
	{
		*this = operator*(other);
		return *this;
	}

	bool Mat44::operator==(const Mat44& other) const
	{
		return *((Vec4*)m[0]) == *((Vec4*)other.m[0]) &&
			*((Vec4*)m[1]) == *((Vec4*)other.m[1]) &&
			*((Vec4*)m[2]) == *((Vec4*)other.m[2]) &&
			*((Vec4*)m[3]) == *((Vec4*)other.m[3]);
	}

	Real* Mat44::operator[](size_t iRow) const
	{
		return (Real*)m[iRow];
	}

	//-------------------------------------------------------------------------
	void Mat44::MakeIdentity()
	{
		m[0][0] = 1.; m[0][1] = 0.; m[0][2] = 0.; m[0][3] = 0.;
		m[1][0] = 0.; m[1][1] = 1.; m[1][2] = 0.; m[1][3] = 0.;
		m[2][0] = 0.; m[2][1] = 0.; m[2][2] = 1.; m[2][3] = 0.;
		m[3][0] = 0.; m[3][1] = 0.; m[3][2] = 0.; m[3][3] = 1.;
	}
	void Mat44::MakeTranslation(const Vec3& pos)
	{
		m[0][0] = 1.; m[0][1] = 0.; m[0][2] = 0.; m[0][3] = pos.x;
		m[1][0] = 0.; m[1][1] = 1.; m[1][2] = 0.; m[1][3] = pos.y;
		m[2][0] = 0.; m[2][1] = 0.; m[2][2] = 1.; m[2][3] = pos.z;
		m[3][0] = 0.; m[3][1] = 0.; m[3][2] = 0.; m[3][3] = 1.;
	}

	Mat44 Mat44::Inverse() const
	{
		Real m00 = m[0][0], m01 = m[0][1], m02 = m[0][2], m03 = m[0][3];
        Real m10 = m[1][0], m11 = m[1][1], m12 = m[1][2], m13 = m[1][3];
        Real m20 = m[2][0], m21 = m[2][1], m22 = m[2][2], m23 = m[2][3];
        Real m30 = m[3][0], m31 = m[3][1], m32 = m[3][2], m33 = m[3][3];

        Real v0 = m20 * m31 - m21 * m30;
        Real v1 = m20 * m32 - m22 * m30;
        Real v2 = m20 * m33 - m23 * m30;
        Real v3 = m21 * m32 - m22 * m31;
        Real v4 = m21 * m33 - m23 * m31;
        Real v5 = m22 * m33 - m23 * m32;

        Real t00 = + (v5 * m11 - v4 * m12 + v3 * m13);
        Real t10 = - (v5 * m10 - v2 * m12 + v1 * m13);
        Real t20 = + (v4 * m10 - v2 * m11 + v0 * m13);
        Real t30 = - (v3 * m10 - v1 * m11 + v0 * m12);

        Real invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

        Real d00 = t00 * invDet;
        Real d10 = t10 * invDet;
        Real d20 = t20 * invDet;
        Real d30 = t30 * invDet;

        Real d01 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
        Real d11 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
        Real d21 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
        Real d31 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

        v0 = m10 * m31 - m11 * m30;
        v1 = m10 * m32 - m12 * m30;
        v2 = m10 * m33 - m13 * m30;
        v3 = m11 * m32 - m12 * m31;
        v4 = m11 * m33 - m13 * m31;
        v5 = m12 * m33 - m13 * m32;

        Real d02 = + (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
        Real d12 = - (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
        Real d22 = + (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
        Real d32 = - (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

        v0 = m21 * m10 - m20 * m11;
        v1 = m22 * m10 - m20 * m12;
        v2 = m23 * m10 - m20 * m13;
        v3 = m22 * m11 - m21 * m12;
        v4 = m23 * m11 - m21 * m13;
        v5 = m23 * m12 - m22 * m13;

        Real d03 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
        Real d13 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
        Real d23 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
        Real d33 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

        return Mat44(
            d00, d01, d02, d03,
            d10, d11, d12, d13,
            d20, d21, d22, d23,
            d30, d31, d32, d33);

	}

	Mat44 Mat44::InverseAffine() const
	{
		assert(IsAffine());

        Real m10 = m[1][0], m11 = m[1][1], m12 = m[1][2];
        Real m20 = m[2][0], m21 = m[2][1], m22 = m[2][2];

        Real t00 = m22 * m11 - m21 * m12;
        Real t10 = m20 * m12 - m22 * m10;
        Real t20 = m21 * m10 - m20 * m11;

        Real m00 = m[0][0], m01 = m[0][1], m02 = m[0][2];

        Real invDet = 1 / (m00 * t00 + m01 * t10 + m02 * t20);

        t00 *= invDet; t10 *= invDet; t20 *= invDet;

        m00 *= invDet; m01 *= invDet; m02 *= invDet;

        Real r00 = t00;
        Real r01 = m02 * m21 - m01 * m22;
        Real r02 = m01 * m12 - m02 * m11;

        Real r10 = t10;
        Real r11 = m00 * m22 - m02 * m20;
        Real r12 = m02 * m10 - m00 * m12;

        Real r20 = t20;
        Real r21 = m01 * m20 - m00 * m21;
        Real r22 = m00 * m11 - m01 * m10;

        Real m03 = m[0][3], m13 = m[1][3], m23 = m[2][3];

        Real r03 = - (r00 * m03 + r01 * m13 + r02 * m23);
        Real r13 = - (r10 * m03 + r11 * m13 + r12 * m23);
        Real r23 = - (r20 * m03 + r21 * m13 + r22 * m23);

        return Mat44(
            r00, r01, r02, r03,
            r10, r11, r12, r13,
            r20, r21, r22, r23,
              0,   0,   0,   1);
	}

	bool Mat44::IsAffine(void) const
	{
		return m[3][0] == 0 && m[3][1] == 0 && m[3][2] == 0 && m[3][3] == 1;
	}

	Mat44 Mat44::Transpose(void) const
	{
		return Mat44(m[0][0], m[1][0], m[2][0], m[3][0],
			m[0][1], m[1][1], m[2][1], m[3][1],
			m[0][2], m[1][2], m[2][2], m[3][2],
			m[0][3], m[1][3], m[2][3], m[3][3]);
	}

	const Vec4& Mat44::Row(size_t iRow) const
	{
		return *((Vec4*)m[iRow]);
	}

	void Mat44::SetTranslation(const Vec3& pos)
	{
		m[0][3] = pos.x;
		m[1][3] = pos.y;
		m[2][3] = pos.z;
	}

	Vec3 Mat44::GetTranslation() const
	{
		Vec3 pos(m[0][3], m[1][3], m[2][3]);
		return pos;
	}

	Mat33 Mat44::To33() const
	{
		Mat33 ret(
			m[0][0], m[0][1], m[0][2],
			m[1][0], m[1][1], m[1][2],
			m[2][0], m[2][1], m[2][2]
			);
		return ret;
	}

	Real Mat44::GetRotationZ() const
	{
		Real yRadians = asin(m[0][2]);
		Real cosY = cos(yRadians);
		if (cosY==0.f)
			return 0.f;

		Real zRadians;
		// No Gimball lock.
		if (abs(cosY) > 0.005f)
		{
			zRadians = atan2(-m[0][1] / cosY, m[0][0] / cosY);
		}
		// Gimball lock has occurred. Rotation around X axis becomes rotation around Z axis.
		else
		{
			zRadians = atan2(m[1][0], m[1][1]);
		}

		if (IsNaN(zRadians))
			return 0.f;

		return zRadians;
	}

	Real Mat44::GetRotationX() const
	{
		Real yRadians = asin(m[0][2]);
		Real cosY = cos(yRadians);
		if (cosY==0.f)
			return 0.f;

		Real xRadians;
		// No Gimball lock.
		if (abs(cosY) > 0.005f)
		{
			xRadians = atan2(-m[1][2] / cosY, m[2][2] / cosY);
		}
		// Gimball lock has occurred. Rotation around X axis becomes rotation around Z axis.
		else
		{
			xRadians = 0;
		}

		if (IsNaN(xRadians))
			return 0.f;

		return xRadians;
	}

	Real Mat44::GetRotationY() const
	{
		auto yRadians = asin(m[0][2]);
		if (IsNaN(yRadians))
			return 0.f;

		return yRadians;
	}


#if defined(FB_DOUBLE_PRECISION)
	Mat44f::Mat44f(){

	}

	Mat44f::Mat44f(const Mat44& other){
		float* d = (float*)this;
		Real* s = (Real*)&other;
		for (int i = 0; i < 16; ++i){
			*d = (float)*s;
			++d;
			++s;
		}
	}
	Mat44f& Mat44f::operator = (const Mat44& other){
		float* d = (float*)this;
		Real* s = (Real*)&other;
		for (int i = 0; i < 16; ++i){
			*d = (float)*s;
			++d;
			++s;
		}
		return *this;
	}

	void Mat44f::MakeIdentity()
	{
		m[0][0] = 1.f; m[0][1] = 0.f; m[0][2] = 0.f; m[0][3] = 0.f;
		m[1][0] = 0.f; m[1][1] = 1.f; m[1][2] = 0.f; m[1][3] = 0.f;
		m[2][0] = 0.f; m[2][1] = 0.f; m[2][2] = 1.f; m[2][3] = 0.f;
		m[3][0] = 0.f; m[3][1] = 0.f; m[3][2] = 0.f; m[3][3] = 1.f;
	}
#endif

	Vec3 Mat44::ComputeAveragePoint3(const Real* coordinates, int numElem, int stride)
	{
		if (stride < 3)
		{
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return Vec3::ZERO;
		}

		int count = 0;
		Real x = 0;
		Real y = 0;
		Real z = 0;		
		for (int i = 0; i <= numElem - stride; i += stride)
		{
			count++;
			x += coordinates[i];
			y += coordinates[i + 1];
			z += coordinates[i + 2];
		}

		if (count == 0)
			return Vec3::ZERO;

		return Vec3(x / (Real)count, y / (Real)count, z / (Real)count);
	}

	/**
	* Computes a symmetric covariance Matrix from the x, y, z coordinates.
	* layout:
	* C(x, x)  C(x, y)  C(x, z)
	* C(x, y)  C(y, y)  C(y, z)
	* C(x, z)  C(y, z)  C(z, z)
	*
	*/
	Mat44 Mat44::FromCovarianceOfVertices(const Real* coordinates, int numElem, int stride)
	{
		Mat44 mat = Mat44::IDENTITY;

		if (stride < 3)
		{
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return mat;
		}

		Vec3 mean = ComputeAveragePoint3(coordinates, numElem, stride);

		int count = 0;
		Real c11 = 0;
		Real c22 = 0;
		Real c33 = 0;
		Real c12 = 0;
		Real c13 = 0;
		Real c23 = 0;		
		for (int i = 0; i <= numElem - stride; i += stride)
		{
			Real x = coordinates[i];
			Real y = coordinates[i + 1];
			Real z = coordinates[i + 2];
			count++;
			c11 += (x - mean.x) * (x - mean.x);
			c22 += (y - mean.y) * (y - mean.y);
			c33 += (z - mean.z) * (z - mean.z);
			c12 += (x - mean.x) * (y - mean.y); // c12 = c21
			c13 += (x - mean.x) * (z - mean.z); // c13 = c31
			c23 += (y - mean.y) * (z - mean.z); // c23 = c32
		}

		if (count == 0)
			return mat;

		return Mat44(
			c11 / (Real)count, c12 / (Real)count, c13 / (Real)count, 0,
			c12 / (Real)count, c22 / (Real)count, c23 / (Real)count, 0,
			c13 / (Real)count, c23 / (Real)count, c33 / (Real)count, 0,
			0, 0, 0, 0);
	}


	bool Mat44::IsSymmetric() const {
		return m[0][1] == m[1][0] && m[0][2] == m[2][0] && m[1][2] == m[2][1];
	}
	/// Computes the eigensystem of the specified symmetric Matrix's upper 3x3 matrix. 		
	void Mat44::ComputeEigensystemFromSymmetricMatrix3(const Mat44& matrix, Real outEigenvalues[3],
		Vec3 outEigenvectors[3])
	{
		if (!matrix.IsSymmetric())
		{
			Logger::Log(FB_ERROR_LOG_ARG, "Matrix is not symmetric.");
			return;
		}

		// Take from "Mathematics for 3D Game Programming and Computer Graphics, Second Edition" by Eric Lengyel,
		// Listing 14.6 (pages 441-444).
		const Real EPSILON = 1.0e-10;
		const int MAX_SWEEPS = 32;

		// Since the Matrix is symmetric, m12=m21, m13=m31, and m23=m32. Therefore we can ignore the values m21, m31,
		// and m32.
		Real m11 = matrix.m[0][0];
		Real m12 = matrix.m[0][1];
		Real m13 = matrix.m[0][2];
		Real m22 = matrix.m[1][1];
		Real m23 = matrix.m[1][2];
		Real m33 = matrix.m[2][2];

		Real r[3][3];
		r[0][0] = r[1][1] = r[2][2] = 1;
		for (int a = 0; a < MAX_SWEEPS; a++)
		{
			// Exit if off-diagonal entries small enough
			if ((std::abs(m12) < EPSILON) && (std::abs(m13) < EPSILON) && (std::abs(m23) < EPSILON))
				break;

			// Annihilate (1,2) entry
			if (m12 != 0.f)
			{
				Real u = (m22 - m11) * 0.5f / m12;
				Real u2 = u * u;
				Real u2p1 = u2 + 1.f;
				Real t = (u2p1 != u2) ?
					((u < 0.f) ? -1.f : 1.f) * (sqrt(u2p1) - std::abs(u))
					: 0.5f / u;
				Real c = 1.f / sqrt(t * t + 1.f);
				Real s = c * t;

				m11 -= t * m12;
				m22 += t * m12;
				m12 = 0.f;

				Real temp = c * m13 - s * m23;
				m23 = s * m13 + c * m23;
				m13 = temp;

				for (int i = 0; i < 3; i++)
				{
					temp = c * r[i][0] - s * r[i][1];
					r[i][1] = s * r[i][0] + c * r[i][1];
					r[i][0] = temp;
				}
			}

			// Annihilate (1,3) entry
			if (m13 != 0.f)
			{
				Real u = (m33 - m11) * 0.5f / m13;
				Real u2 = u * u;
				Real u2p1 = u2 + 1.f;
				Real t = (u2p1 != u2) ?
					((u < 0.f) ? -1.f : 1.f) * (sqrt(u2p1) - std::abs(u))
					: 0.5f / u;
				Real c = 1.f / sqrt(t * t + 1.f);
				Real s = c * t;

				m11 -= t * m13;
				m33 += t * m13;
				m13 = 0.f;

				Real temp = c * m12 - s * m23;
				m23 = s * m12 + c * m23;
				m12 = temp;

				for (int i = 0; i < 3; i++)
				{
					temp = c * r[i][0] - s * r[i][2];
					r[i][2] = s * r[i][0] + c * r[i][2];
					r[i][0] = temp;
				}
			}

			// Annihilate (2,3) entry
			if (m23 != 0.f)
			{
				Real u = (m33 - m22) * 0.5f / m23;
				Real u2 = u * u;
				Real u2p1 = u2 + 1.f;
				Real t = (u2p1 != u2) ?
					((u < 0.f) ? -1.f : 1.f) * (sqrt(u2p1) - std::abs(u))
					: 0.5f / u;
				Real c = 1.f / sqrt(t * t + 1.f);
				Real s = c * t;

				m22 -= t * m23;
				m33 += t * m23;
				m23 = 0.f;

				Real temp = c * m12 - s * m13;
				m13 = s * m12 + c * m13;
				m12 = temp;

				for (int i = 0; i < 3; i++)
				{
					temp = c * r[i][1] - s * r[i][2];
					r[i][2] = s * r[i][1] + c * r[i][2];
					r[i][1] = temp;
				}
			}
		}

		outEigenvalues[0] = m11;
		outEigenvalues[1] = m22;
		outEigenvalues[2] = m33;

		outEigenvectors[0] = Vec3(r[0][0], r[1][0], r[2][0]);
		outEigenvectors[1] = Vec3(r[0][1], r[1][1], r[2][1]);
		outEigenvectors[2] = Vec3(r[0][2], r[1][2], r[2][2]);
	}
}

