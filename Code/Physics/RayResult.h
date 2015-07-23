#pragma once

namespace fastbird
{
	class RigidBody;
	struct RayResultClosest
	{
		RayResultClosest()
		: mRigidBody(0), mHitNormalWorld(0, 1, 0), mHitPointWorld(0, 0, 0), mIndex(-1)
		{

		}

		RayResultClosest(RigidBody* body, const Vec3& hitPoint, const Vec3& normal, int index)
			:mRigidBody(body), mHitPointWorld(hitPoint), mHitNormalWorld(normal), mIndex(index)
		{

		}

		RigidBody* mRigidBody;
		Vec3 mHitPointWorld;
		Vec3 mHitNormalWorld;
		int mIndex; // child index for compound shapes
	};

	struct RayResultWithObj
	{
		RayResultWithObj(RigidBody* body)
		: mTargetBody(body), mRigidBody(0), mHitNormalWorld(0, 1, 0), mHitPointWorld(0, 0, 0), mIndex(-1)
		{

		}

		RigidBody* mTargetBody;
		RigidBody* mRigidBody;
		Vec3 mHitPointWorld;
		Vec3 mHitNormalWorld;
		int mIndex; // child index for compound shapes
	};

	struct RayResultAll
	{
		static const int SIZE = 200;
		RayResultAll()
			:mCurSize(0)
		{

		}
		~RayResultAll()
		{
			for (unsigned i = 0; i < mCurSize; ++i)
			{
				mRayResults[i]->~RayResultClosest();
				free(mRayResults[i]);
			}
		}

		void AddResult(RigidBody* rigidBody, const Vec3& hitPoint, const Vec3& hitNormal, int index)
		{
			if (mCurSize >= SIZE)
				return;
			
			mRayResults[mCurSize++] = (RayResultClosest*)malloc(sizeof(RayResultClosest));

			new (mRayResults[mCurSize-1]) RayResultClosest(rigidBody, hitPoint, hitNormal, index);
		}

		RayResultClosest* mRayResults[SIZE];
		unsigned mCurSize;
	};
}