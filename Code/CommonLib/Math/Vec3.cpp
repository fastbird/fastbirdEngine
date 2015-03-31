#include <CommonLib/StdAfx.h>
#include <CommonLib/Math/Vec3.h>
#include <CommonLib/Math/Vec3I.h>
#include <CommonLib/Math/Mat33.h>
#include <CommonLib/StringUtils.h>

namespace fastbird
{
	const Vec3 Vec3::UNIT_X(1.f, 0, 0);
	const Vec3 Vec3::UNIT_Y(0, 1.f, 0);
	const Vec3 Vec3::UNIT_Z(0, 0, 1.f);
	const Vec3 Vec3::ZERO(0, 0, 0);
	const Vec3 Vec3::MAX(FLT_MAX, FLT_MAX, FLT_MAX);
	const Vec3 Vec3::MIN(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	Vec3::Vec3(const char* s)
	{
		char* next;
		x = 0;
		y = 0;
		z = 0;
		if (s!=0)
		{
			x = (float)strtod(s, &next);
			if (next!=0)
			{
				StepToDigit(&next);
				y = (float)strtod(next, &next);
				if (next!=0)
				{
					StepToDigit(&next);
					z = (float)strtod(next, 0);
				}
			}
		}			
	}

	Vec3::Vec3(const Vec3I& v)
	{
		x = (float)v.x;
		y = (float)v.y;
		z = (float)v.z;
	}

	Vec3 Vec3::operator* (const Mat33& r) const
	{
		return Vec3(	x * r[0][0] + y * r[1][0] + z * r[2][0],
						x * r[0][1] + y * r[1][1] + z * r[2][1],
						x * r[0][2] + y * r[1][2] + z * r[2][2]);
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

	float Vec3::AngleBetween(const Vec3& v) const
	{
		float lenProduct = Length() * v.Length();

		// Prevent dividing zero
		if(lenProduct < 1e-6f)
			lenProduct = 1e-6f;

		float f = Dot(v) / lenProduct;
			
		Clamp(f, -1.0f, 1.0f);
		return ACos(f);

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
}
