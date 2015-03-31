#include <Engine/StdAfx.h>
#include <Engine/FBCollisionShape.h>
#include <Engine/IMeshObject.h>
#include <CommonLib/Math/AABB.h>
#include <CommonLib/Math/BVaabb.h>

namespace fastbird
{

	FBCollisionShape::FBCollisionShape(FBColShape::Enum e, const Transformation& t, IMeshObject* colMesh)
	{
		mColShape = e;
		mColMesh = colMesh;
		mTransformation = t;

		switch (e)
		{
		case FBColShape::SPHERE:
		{
								 mBV = BoundingVolume::Create(BoundingVolume::BV_SPHERE);
								 auto scale = t.GetScale();
								 if (scale.x != scale.z || scale.x != scale.y)
								 {
									 Log("Collision Sphere should be uniform scaled!");
									 assert(0);
								 }
								 mBV->SetRadius(1 * t.GetScale().x);
								 mBV->SetCenter(Vec3::ZERO);
		}
			break;

		case FBColShape::CUBE:
		{
							   mBV = BoundingVolume::Create(BoundingVolume::BV_AABB);
							   AABB aabb;
							   aabb.SetMax(Vec3(1, 1, 1) * t.GetScale());
							   aabb.SetMin(Vec3(-1, -1, -1) * t.GetScale());
							   BVaabb* bvaabb = (BVaabb*)mBV.get();
							   bvaabb->SetAABB(aabb);
		}
			break;

		default:
			break;
		}
	}

	FBCollisionShape::~FBCollisionShape()
	{

	}

	FBCollisionShape::IResult FBCollisionShape::intersects(const Ray3& ray, const Transformation& objT) const
	{
		if (!mBV)
			return IResult(false, FLT_MAX);
		Ray3 localRay = objT.ApplyInverse(ray);
		Ray3::IResult ret = localRay.intersects(mBV);
		return ret;
	}

	bool FBCollisionShape::TestCollision(fastbird::BoundingVolume* pBV, const Transformation& objT) const
	{
		if (!mBV)
			return false;

		auto newCenter = objT.ApplyInverse(pBV->GetCenter());
		float newRad = pBV->GetRadius();
		SmartPtr<BoundingVolume> localBV = BoundingVolume::Create();
		localBV->SetCenter(newCenter);
		localBV->SetRadius(newRad);		
		return mBV->TestIntersection(localBV);
	}

	Vec3 FBCollisionShape::GetRandomPosInVolume(const Vec3* nearWorld, const Transformation& objT) const
	{
		if (nearWorld)
		{
			Vec3 nearLocal = objT.ApplyInverse(*nearWorld);
			return mBV->GetRandomPosInVolume(&nearLocal);
		}
		return mBV->GetRandomPosInVolume();
		
	}

}