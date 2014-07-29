#pragma once

#include <CommonLib/Math/Mat33.h>
#include <CommonLib/Math/Vec3.h>
#include <CommonLib/Math/Vec4.h>

namespace fastbird
{
	class Mat44
	{
	public:
		Mat44() {}

		Mat44(float m11, float m12, float m13, float m14,
			float m21, float m22, float m23, float m24,
			float m31, float m32, float m33, float m34,
			float m41, float m42, float m43, float m44)
		{
			m[0][0]=m11; m[0][1]=m12; m[0][2]=m13; m[0][3]=m14; 
			m[1][0]=m21; m[1][1]=m22; m[1][2]=m23; m[1][3]=m24; 
			m[2][0]=m31; m[2][1]=m32; m[2][2]=m33; m[2][3]=m34; 
			m[3][0]=m41; m[3][1]=m42; m[3][2]=m43; m[3][3]=m44;
		}

		Mat44(const Mat33& mat33, const Vec3& vec3)
		{
			m[0][0]=mat33[0][0]; m[0][1]=mat33[0][1]; m[0][2]=mat33[0][2]; m[0][3]=vec3.x; 
			m[1][0]=mat33[1][0]; m[1][1]=mat33[1][1]; m[1][2]=mat33[1][2]; m[1][3]=vec3.y; 
			m[2][0]=mat33[2][0]; m[2][1]=mat33[2][1]; m[2][2]=mat33[2][2]; m[2][3]=vec3.z; 
			m[3][0]=0; m[3][1]=0; m[3][2]=0; m[3][3]=1.f;
		}

		inline float* operator[](size_t iRow) const
		{
			return (float*)m[iRow];
		}

		inline void MakeIdentity()
		{
			m[0][0] = 1.f; m[0][1] = 0.f; m[0][2] = 0.f; m[0][3] = 0.f;
			m[1][0] = 0.f; m[1][1] = 1.f; m[1][2] = 0.f; m[1][3] = 0.f;
			m[2][0] = 0.f; m[2][1] = 0.f; m[2][2] = 1.f; m[2][3] = 0.f;
			m[3][0] = 0.f; m[3][1] = 0.f; m[3][2] = 0.f; m[3][3] = 1.f;
		}
		inline void MakeTranslation(const Vec3& pos)
		{
			m[0][0] = 1.f; m[0][1] = 0.f; m[0][2] = 0.f; m[0][3] = pos.x;
			m[1][0] = 0.f; m[1][1] = 1.f; m[1][2] = 0.f; m[1][3] = pos.y;
			m[2][0] = 0.f; m[2][1] = 0.f; m[2][2] = 1.f; m[2][3] = pos.z;
			m[3][0] = 0.f; m[3][1] = 0.f; m[3][2] = 0.f; m[3][3] = 1.f;
		}

		Mat44 Inverse() const;
		Mat44 InverseAffine() const;

		inline bool IsAffine(void) const
        {
            return m[3][0] == 0 && m[3][1] == 0 && m[3][2] == 0 && m[3][3] == 1;
        }

		inline Mat44 transpose(void) const
        {
            return Mat44(m[0][0], m[1][0], m[2][0], m[3][0],
                           m[0][1], m[1][1], m[2][1], m[3][1],
                           m[0][2], m[1][2], m[2][2], m[3][2],
                           m[0][3], m[1][3], m[2][3], m[3][3]);
        }

		Vec4 operator*(const Vec4& columnVector) const;
		Vec3 operator*(const Vec3& columnVector) const;

		inline const Vec4& Row(size_t iRow) const
		{
			return *((Vec4*)m[iRow]);
		}

		Mat44 operator* (const Mat44& other) const;

		Mat44& operator*=(const Mat44& other);

		bool operator==(const Mat44& other) const
		{
			return *((Vec4*)m[0]) == *((Vec4*)other.m[0]) &&
				*((Vec4*)m[1]) == *((Vec4*)other.m[1]) &&
				*((Vec4*)m[2]) == *((Vec4*)other.m[2]) &&
				*((Vec4*)m[3]) == *((Vec4*)other.m[3]);
		}

		void SetTranslation(const Vec3& pos)
		{
			m[0][3] = pos.x;
			m[1][3] = pos.y;
			m[2][3] = pos.z;
		}

		Vec3 GetTranslation() const
		{
			Vec3 pos(m[0][3], m[1][3], m[2][3]);
			return pos;
		}

		Mat33 To33() const
		{
			Mat33 ret(
				m[0][0], m[0][1], m[0][2],
				m[1][0], m[1][1], m[1][2],
				m[2][0], m[2][1], m[2][2]
				);
			return ret;
		}


	public:
		float m[4][4];

		static const Mat44 IDENTITY;
	};
}