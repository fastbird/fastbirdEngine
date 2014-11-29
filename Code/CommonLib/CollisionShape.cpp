#include <CommonLib/StdAfx.h>
#include <CommonLib/CollisionShape.h>
#include <CommonLib/Math/AABB.h>
#include <CommonLib/Math/BVaabb.h>

namespace fastbird
{

	CollisionShape::CollisionShape(ColShape::Enum e, const Transformation& t)
	{
		mTransformation = t;
		switch (e)
		{
		case ColShape::SPHERE:
		{
								 mBV = BoundingVolume::Create(BoundingVolume::BV_SPHERE);
								 mBV->SetRadius(1);
								 mBV->SetCenter(Vec3::ZERO);
		}
			break;

		case ColShape::CUBE:
		{
							   mBV = BoundingVolume::Create(BoundingVolume::BV_AABB);
							   AABB aabb;
							   aabb.SetMax(Vec3(1, 1, 1));
							   aabb.SetMin(Vec3(-1, -1, -1));
							   BVaabb* bvaabb = (BVaabb*)mBV.get();
							   bvaabb->SetAABB(aabb);
		}
			break;

		default:
			assert(0);
			break;
		}
	}

	CollisionShape::~CollisionShape()
	{

	}

	CollisionShape::IResult CollisionShape::intersects(const Ray3& ray, const Transformation& objT) const
	{
		Ray3 localRay = (objT * mTransformation).ApplyInverse(ray);
		Ray3::IResult ret = localRay.intersects(mBV);
		return ret;
	}

	bool CollisionShape::TestCollision(fastbird::BoundingVolume* pBV, const Transformation& objT) const
	{
		assert(pBV);
		auto shapeTransform = objT * mTransformation;
		auto newCenter = (shapeTransform).ApplyInverse(pBV->GetCenter());
		float newRad = pBV->GetRadius() / shapeTransform.GetNorm();
		auto localBV = BoundingVolume::Create();
		localBV->SetCenter(newCenter);
		localBV->SetRadius(newRad);		
		return localBV->TestIntersection(mBV);
	}

}