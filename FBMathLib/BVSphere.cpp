/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#include "stdafx.h"
#include "BVSphere.h"
#include "Math.h"

using namespace fb;

//----------------------------------------------------------------------------
BVSphere::BVSphere()
{
}

//----------------------------------------------------------------------------
BVSphere::BVSphere(const Vec3& center, Real radius)
	: mCenter(center), mRadius(radius)
{
}

BoundingVolume& BVSphere::operator = (const BoundingVolume& other){
	mAlwaysPass = other.GetAlwaysPass();
	mCenter = other.GetCenter();
	mRadius = other.GetRadius();
	return *this;
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
	mCenter /= (Real)numVertices;
	mRadius = 0;
	for (size_t i=0; i<numVertices; i++)
	{
		Vec3 diff = pVertices[i] - mCenter;
		Real radiusSQ = diff.LengthSQ();
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
			BoundingVolumePtr result)
{
	assert(result);
	BVSphere* pNewSphere = (BVSphere*)result.get();
	Vec3 newCenter = transform.ApplyForward(mCenter);
	Real newRadius = transform.GetNorm() * mRadius;
	pNewSphere->SetCenter(newCenter);
	pNewSphere->SetRadius(newRadius);
}

//----------------------------------------------------------------------------
int BVSphere::WhichSide(const Plane3& plane) const
{
	if (mAlwaysPass)
		return 1;
	Real fDistance = plane.DistanceTo(mCenter);

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
	Real distSQ =  pBV->GetCenter().DistanceToSQ(mCenter);
	Real radiusSQ = mRadius + pBV->GetRadius();
	return distSQ < radiusSQ*radiusSQ;
}

//----------------------------------------------------------------------------
void BVSphere::Merge(const BoundingVolume* pBV)
{
	Vec3 dir = mCenter - pBV->GetCenter();
	Real distance = dir.Normalize();
	Vec3 myFar = mCenter + dir * mRadius;
	Vec3 otherFar = pBV->GetCenter() + dir * pBV->GetRadius();
	Real mydot = myFar.Dot(dir);
	Real otherdot = otherFar.Dot(dir);
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
	Real distance = dir.Normalize();
	if (distance > mRadius)
	{
		mRadius = distance;
	}
}

Vec3 BVSphere::GetSurfaceFrom(const Vec3& src, Vec3& normal)
{
	Vec3 dir = mCenter - src;
	Real dist = dir.Normalize();
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

Vec3 BVSphere::GetRandomPosInVolume(const Vec3* nearLocal) const
{
	Vec3 pos = Vec3(Random(0.f, mRadius), Random(0.f, mRadius), Random(0.f, mRadius));
	if (nearLocal)
	{
		pos = Lerp(pos, *nearLocal, Random(0.5f, 1.0f));
	}
	pos.Normalize();
	return mCenter + pos*mRadius;
	
}