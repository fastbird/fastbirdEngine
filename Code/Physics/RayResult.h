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
		RayResultAll()
		{

		}
		~RayResultAll()
		{
			for (auto& data : mRayResults)
			{
				delete data;
			}
		}

		void AddResult(RigidBody* rigidBody, const Vec3& hitPoint, const Vec3& hitNormal, int index)
		{
			mRayResults.push_back(new RayResultClosest(rigidBody, hitPoint, hitNormal, index));
		}

		std::vector<RayResultClosest*> mRayResults;
	};
}