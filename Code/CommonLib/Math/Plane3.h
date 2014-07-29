#pragma once
#ifndef _fastbird_Plane3_header_included_
#define _fastbird_Plane3_header_included_

#include <CommonLib/Math/Vec3.h>

namespace fastbird
{
	class Plane3
	{
	public:
		//Dot(N,X) = c
		Plane3()
		{
		}

		Plane3(const Plane3& other)
			: mNormal(other.mNormal)
			, mConstant(other.mConstant)
		{
		}

		Plane3(const Vec3& normal, float c)
			: mNormal(normal)
			, mConstant(c)
		{
		}

		Plane3(const Vec3& normal, const Vec3& p)
			: mNormal(normal)
		{
			mConstant = mNormal.Dot(p);

		}

		Plane3(const Vec3& p0, const Vec3& p1, const Vec3& p2)
		{
			Vec3 edge1 = p1 - p0;
			Vec3 edge2 = p2 - p0;
			mNormal = edge1.Cross(edge2);
			mNormal.Normalize();
			mConstant = mNormal.Dot(p0);
		}

		Plane3 operator=(const Plane3& other)
		{
			mNormal = other.mNormal;
			mConstant = other.mConstant;
			return *this;
		}

		int WhichSide(const Vec3& p) const
		{
			float distance = DistanceTo(p);
			if (distance<0.0f)
				return -1;
			if (distance>0.0)
				return 1;
			return 0;
		}

		float DistanceTo(const Vec3& p) const
		{
			return mNormal.Dot(p) - mConstant;
		}

		Vec3 mNormal;
		float mConstant;
	};
}

#endif //_fastbird_Plane3_header_included_