#pragma once
#ifndef _Mat33_header_included_
#define _Mat33_header_included_

#include <CommonLib/Math/Vec3.h>

namespace fastbird
{
	class Mat33
	{
	public:
		inline Mat33()	{}

		inline Mat33(float m11, float m12, float m13,
			float m21, float m22, float m23,
			float m31, float m32, float m33)
		{
			m[0][0] = m11;			m[0][1] = m12;			m[0][2] = m13;
			m[1][0] = m21;			m[1][1] = m22;			m[1][2] = m23;
			m[2][0] = m31;			m[2][1] = m32;			m[2][2] = m33;
		}

		inline Mat33(const Vec3& x, const Vec3& y, const Vec3& z)
		{
			SetColumn(0, x);
			SetColumn(1, y);
			SetColumn(2, z);
		}

		inline float* operator[](size_t iRow) const
		{
			return (float*)m[iRow];
		}

		void MakeIdentity()
		{
			m[0][0] = 1.f;			m[0][1] = 0.f;			m[0][2] = 0.f;
			m[1][0] = 0.f;			m[1][1] = 1.f;			m[1][2] = 0.f;
			m[2][0] = 0.f;			m[2][1] = 0.f;			m[2][2] = 1.f;
		}

		void operator/=(float k)
		{
			float invK = 1.0f / k;
			m[0][0] *= invK;	m[0][1] *= invK;	m[0][2] *= invK;
			m[1][0] *= invK;	m[1][1] *= invK;	m[1][2] *= invK;
			m[2][0] *= invK;	m[2][1] *= invK;	m[2][2] *= invK;
		}

		Mat33 operator* (const Mat33& rMat) const;
		Vec3 operator* (const Vec3& rVec) const;
		Mat33 operator/ (float k) const
		{
			float invK = 1.0f / k;
			return Mat33(	m[0][0] * invK,	m[0][1] * invK,	m[0][2] * invK,
							m[1][0] * invK,	m[1][1] * invK,	m[1][2] * invK,
							m[2][0] * invK,	m[2][1] * invK,	m[2][2] * invK);
		}

		Mat33 operator-() const
		{
			Mat33 ret;
			for (size_t row=0; row<3; row++)
			{
				for (size_t col=0; col<3; col++)
				{
					ret[row][col] = -m[row][col];
				}
			}
			return ret;
		}
		
		void FromAxisAngle(const Vec3& axis, float radian);

		Mat33 Inverse() const;
		Mat33 Transpose() const;

		Mat33 ScaleAxis(const Vec3& scale) const;

		Vec3 Column(int index) const
		{
			Vec3 ret(m[0][index], m[1][index], m[2][index]);
			return ret;
		}

		void SetColumn(int index, const Vec3& v)
		{
			m[0][index] = v.x;
			m[1][index] = v.y;
			m[2][index] = v.z;
		}

		bool operator==(const Mat33& other) const;
		static const Mat33 IDENTITY;
		static const Mat33 ZERO;

	public:
		float m[3][3];

	};
}
#endif //_Mat33_header_included_