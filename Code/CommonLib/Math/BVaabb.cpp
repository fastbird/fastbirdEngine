#include <CommonLib/StdAfx.h>
#include <CommonLib/Math/BVaabb.h>
#include <CommonLib/Math/Transformation.h>

namespace fastbird
{
BVaabb::BVaabb()
	: mCenter(0, 0, 0)
{
}

void BVaabb::Merge(const BoundingVolume* pBV)
{
	assert(pBV);
	const Vec3& center = pBV->GetCenter();
	float radius = pBV->GetRadius();
	mAABB.Merge(center + Vec3(radius, 0, 0));
	mAABB.Merge(center + Vec3(-radius, 0, 0));

	mAABB.Merge(center + Vec3(0, radius, 0));
	mAABB.Merge(center + Vec3(0, -radius, 0));

	mAABB.Merge(center + Vec3(0, 0, radius));
	mAABB.Merge(center + Vec3(0, 0, -radius));	

}

void BVaabb::Merge(const Vec3& pos)
{
	mAABB.Merge(pos);
}

void BVaabb::SetCenter (const Vec3& center)
{
	if (mAABB.IsValid())
	{
		Vec3 diff = center - mCenter;
		mAABB.Translate(diff);
		
	}
	else
	{
		mAABB.Merge(center);
	}
	mCenter = center;
	
}

void BVaabb::SetRadius (float fRadius)
{
	Vec3 center = mAABB.GetCenter();
	mAABB.Merge(center + Vec3(fRadius, 0, 0));
	mAABB.Merge(center + Vec3(-fRadius, 0, 0));

	mAABB.Merge(center + Vec3(0, fRadius, 0));
	mAABB.Merge(center + Vec3(0, -fRadius, 0));

	mAABB.Merge(center + Vec3(0, 0, fRadius));
	mAABB.Merge(center + Vec3(0, 0, -fRadius));	
	
}

const Vec3& BVaabb::GetCenter () const
{
	return mCenter;
}

float BVaabb::GetRadius () const
{
	Vec3 axis =  mAABB.GetMax() - mCenter;
	return std::max(axis.x, std::max(axis.y, axis.z));
}

void BVaabb::ComputeFromData(const Vec3* pVertices, size_t numVertices)
{
	assert(pVertices && numVertices>0);
	mAABB.Invalidate();
	for (size_t i=0; i<numVertices; i++)
	{
		mAABB.Merge(pVertices[i]);
	}
	mCenter = mAABB.GetCenter();
}

void BVaabb::StartComputeFromData()
{
	mAABB.Invalidate();
}

void BVaabb::AddComputeData(const Vec3* pVertices, size_t numVertices)
{
	assert(pVertices && numVertices>0);
	for (size_t i=0; i<numVertices; i++)
	{
		mAABB.Merge(pVertices[i]);
	}
}

void BVaabb::EndComputeFromData()
{
	mCenter = mAABB.GetCenter();
}

void BVaabb::TransformBy(const Transformation& transform,
	BoundingVolume* result)
{
	assert(result);
	BVaabb* pNewBound = (BVaabb*)result;
	AABB newAABB = mAABB;
	newAABB.Translate(transform.GetTranslation());
	pNewBound->SetAABB(newAABB);
}

int BVaabb::WhichSide(const Vec3& min, const Vec3& max) const
{
	if (mAlwaysPass)
		return 1;
	const Vec3& objMin = mAABB.GetMin();
	const Vec3& objMax = mAABB.GetMax();
	if (min.x > objMax.x || max.x < objMin.x ||
		min.y > objMax.y || max.y < objMin.y ||
		min.z > objMax.z || max.z < objMin.z)
		return -1; // outside

	else if (min.x < objMin.x && max.x > objMax.x &&
		min.y < objMin.y && max.y > objMax.y &&
		min.z < objMin.z && max.z > objMax.z)
		return 1;

	return 0;
}

bool BVaabb::TestIntersection(const Ray3& ray) const
{
	Vec3 normal;
	Ray3::IResult ret = ray.intersects(mAABB, normal);
	return ret.first;
}

bool BVaabb::TestIntersection(BoundingVolume* pBV) const
{
	float distSQ = pBV->GetCenter().DistanceToSQ(mCenter);
	float radiusSQ = GetRadius() + pBV->GetRadius();
	return distSQ < radiusSQ*radiusSQ;
}

void BVaabb::SetAABB(const AABB& aabb)
{
	mAABB = aabb;
	mCenter = mAABB.GetCenter();
}

BoundingVolume& BVaabb::operator= (const BoundingVolume& other)
{
	mAABB.Invalidate();
	mAABB.Merge(other.GetCenter());
	float radius = other.GetRadius();
	SetRadius(radius);
	mCenter = mAABB.GetCenter();
	return *this;
}

Vec3 BVaabb::GetSurfaceTo(const Vec3& target, Vec3& normal)
{
	Vec3 dir = mCenter - target;
	dir.Normalize();
	Ray3 ray(target, dir);
	Ray3::IResult ret = ray.intersects(mAABB, normal);
	return -dir * ret.second;
}

void BVaabb::Expand(float e)
{
	mAABB.SetMax(mAABB.GetMax() + e);
	mAABB.SetMin(mAABB.GetMin() - e);
	
}
}
