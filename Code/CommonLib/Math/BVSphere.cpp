#include <CommonLib/StdAfx.h>
#include <CommonLib/Math/BVSphere.h>
#include <CommonLib/Math/BVaabb.h>
#include "Transformation.h"

using namespace fastbird;

BoundingVolume* BoundingVolume::Create(BVType type /*= BV_SPHERE*/)
{
	if (type == BV_SPHERE)
		return FB_NEW(BVSphere);
	else if (type == BV_AABB)
		return FB_NEW(BVaabb);

	else
	{
		assert(0);
		return 0;
	}
}

//----------------------------------------------------------------------------
BVSphere::BVSphere()
{
}

//----------------------------------------------------------------------------
BVSphere::BVSphere(const Vec3& center, float radius)
	: mCenter(center), mRadius(radius)
{
}

//----------------------------------------------------------------------------
void BVSphere::ComputeFromData(const Vec3* pVertices, size_t numVertices)
{
	assert(pVertices && numVertices>0);
	mCenter = pVertices[0];
	for (size_t i=1; i<numVertices; i++)
	{
		mCenter+=pVertices[i];
	}
	mCenter /= (float)numVertices;
	mRadius = 0;
	for (size_t i=0; i<numVertices; i++)
	{
		Vec3 diff = pVertices[i] - mCenter;
		float radiusSQ = diff.LengthSQ();
		if (radiusSQ > mRadius)
		{
			mRadius = radiusSQ;
		}
	}
	mRadius = sqrt(mRadius);
}

//----------------------------------------------------------------------------
void BVSphere::StartComputeFromData()
{
	mVertices.clear();
}

//----------------------------------------------------------------------------
void BVSphere::AddComputeData(const Vec3* pVertices, size_t numVertices)
{
	mVertices.insert(mVertices.end(), pVertices, pVertices + numVertices);
}

//----------------------------------------------------------------------------
void BVSphere::AddComputeData(const Vec3& vert)
{
	mVertices.push_back(vert);
}


//----------------------------------------------------------------------------
void BVSphere::EndComputeFromData()
{
	if (mVertices.empty())
		return;
	ComputeFromData(&mVertices[0], mVertices.size());
	mVertices.resize(0);
}

//----------------------------------------------------------------------------
void BVSphere::TransformBy(const Transformation& transform,
			BoundingVolume* result)
{
	assert(result);
	BVSphere* pNewSphere = (BVSphere*)result;
	Vec3 newCenter = transform.ApplyForward(mCenter);
	float newRadius = transform.GetNorm() * mRadius;
	pNewSphere->SetCenter(newCenter);
	pNewSphere->SetRadius(newRadius);
}

//----------------------------------------------------------------------------
int BVSphere::WhichSide(const Plane3& plane) const
{
	if (mAlwaysPass)
		return 1;
	float fDistance = plane.DistanceTo(mCenter);

    if (fDistance <= -mRadius)
    {
        return -1;
    }

    if (fDistance >= mRadius)
    {
        return +1;
    }

    return 0;
}

//----------------------------------------------------------------------------
bool BVSphere::TestIntersection(const Ray3& ray) const
{
	assert(0 && "Not Implemented!");
	return false;
}

//----------------------------------------------------------------------------
bool BVSphere::TestIntersection(BoundingVolume* pBV) const
{
	float distSQ =  pBV->GetCenter().DistanceToSQ(mCenter);
	float radiusSQ = mRadius + pBV->GetRadius();
	return distSQ < radiusSQ*radiusSQ;
}

//----------------------------------------------------------------------------
void BVSphere::Merge(const BoundingVolume* pBV)
{
	Vec3 dir = mCenter - pBV->GetCenter();
	float distance = dir.Normalize();
	Vec3 myFar = mCenter + dir * mRadius;
	Vec3 otherFar = pBV->GetCenter() + dir * pBV->GetRadius();
	float mydot = myFar.Dot(dir);
	float otherdot = otherFar.Dot(dir);
	Vec3 mostFar0 = mydot > otherdot ? myFar : otherFar;

	dir = pBV->GetCenter() - mCenter;
	dir.Normalize();
	myFar = mCenter + dir * mRadius;
	otherFar = pBV->GetCenter() + dir * pBV->GetRadius();
	mydot = myFar.Dot(dir);
	otherdot = otherFar.Dot(dir);
	Vec3 mostFar1 = mydot > otherdot ? myFar : otherFar;

	mCenter = (mostFar0 + mostFar1) * .5f;
	mRadius = (mostFar0 - mostFar1).Length() * .5f;
}

void BVSphere::Merge(const Vec3& pos)
{
	assert(0 && "don't use");
	Vec3 dir = mCenter - pos;
	float distance = dir.Normalize();
	if (distance > mRadius)
	{
		mRadius = distance;
	}
}

BoundingVolume& BVSphere::operator= (const BoundingVolume& other)
{
	mCenter = other.GetCenter();
	mRadius = other.GetRadius();
	return *this;
}

Vec3 BVSphere::GetSurfaceFrom(const Vec3& src, Vec3& normal)
{
	Vec3 dir = mCenter - src;
	float dist = dir.Normalize();
	normal = -dir;
	dist -= mRadius;
	dir *= dist;
	return dir;
}

void BVSphere::Invalidate()
{
	assert(0 && "No meaning for sphere.");
}

bool BVSphere::Contain(const Vec3& pos) const
{
	return (mCenter - pos).Length() < mRadius;
}