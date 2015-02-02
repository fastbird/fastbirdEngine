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
								 mBV->SetRadius(1);
								 mBV->SetCenter(Vec3::ZERO);
		}
			break;

		case FBColShape::CUBE:
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
		Ray3 localRay = (objT * mTransformation).ApplyInverse(ray);
		Ray3::IResult ret = localRay.intersects(mBV);
		return ret;
	}

	bool FBCollisionShape::TestCollision(fastbird::BoundingVolume* pBV, const Transformation& objT) const
	{
		if (!mBV)
			return false;

		auto shapeTransform = objT * mTransformation;
		auto newCenter = (shapeTransform).ApplyInverse(pBV->GetCenter());
		float newRad = pBV->GetRadius() / shapeTransform.GetNorm();
		auto localBV = BoundingVolume::Create();
		localBV->SetCenter(newCenter);
		localBV->SetRadius(newRad);		
		return localBV->TestIntersection(mBV);
	}

}