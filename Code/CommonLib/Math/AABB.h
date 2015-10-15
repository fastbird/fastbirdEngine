#pragma once
#include "Vec3.h"

namespace fastbird
{
	class AABB
	{
	public:
		AABB()
		{
			Invalidate();
		}

		bool IsValid() const
		{
			return mMax >= mMin;
		}

		void Invalidate()
		{
			mMin = Vec3::MAX;
			mMax = Vec3::MIN;
		}

		void Merge(const Vec3& point)
		{
			mMin.KeepLesser(point);
			mMax.KeepGreater(point);
		}

		void Merge(const AABB& aabb){
			mMin.x = std::min(mMin.x, aabb.mMin.x);
			mMin.y = std::min(mMin.y, aabb.mMin.y);
			mMin.z = std::min(mMin.z, aabb.mMin.z);
			mMax.x = std::max(mMax.x, aabb.mMax.x);
			mMax.y = std::max(mMax.y, aabb.mMax.y);
			mMax.z = std::max(mMax.z, aabb.mMax.z);
		}

		const Vec3& GetMin() const { return mMin; }
		const Vec3& GetMax() const { return mMax; }

		void SetMin(const Vec3& min) { mMin = min;}
		void SetMax(const Vec3& max) { mMax = max;}

		Vec3 GetCenter() const { return (mMin + mMax) * .5f; }

		void Translate(const Vec3& pos)
		{
			mMin += pos;
			mMax += pos;
		}

		bool Contain(const Vec3&pos) const
		{
			if (pos.x < mMin.x || pos.y < mMin.y || pos.z < mMin.z ||
				pos.x > mMax.x || pos.y > mMax.y || pos.z > mMax.z)
				return false;

			return true;
		}


	private:
		Vec3 mMin;
		Vec3 mMax;
	};
}