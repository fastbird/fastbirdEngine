#include <CommonLib/StdAfx.h>
#include <CommonLib/Math/Ray3.h>
#include <CommonLib/Math/BoundingVolume.h>
#include <CommonLib/Math/AABB.h>
#include <CommonLib/Collision/GeomCollisions.h>

namespace fastbird
{

Ray3::Ray3()
{
}

Ray3::Ray3(const Vec3& origin, const Vec3& dir)
	: mOrigin(origin)
{
	SetDir(dir);
}

Ray3::IResult Ray3::intersects(BoundingVolume* pBoundingVolume) const
{
	assert(pBoundingVolume);
    // to relative space
	Vec3 origin = mOrigin - pBoundingVolume->GetCenter();
	float radius = pBoundingVolume->GetRadius();

    // Check origin inside first
	if (origin.LengthSQ() <= radius*radius)
    {
		return Ray3::IResult(true, 0.f);
    }

    // t = (-b +/- sqrt(b*b + 4ac)) / 2a
	float a = mDir.Dot(mDir);
	float b = 2 * origin.Dot(mDir);
    float c = origin.Dot(origin) - radius*radius;

    // determinant
    float d = (b*b) - (4 * a * c);
    if (d < 0)
    {
        // No intersection
        return IResult(false, 0.f);
    }
    else
    {
        float t = ( -b - sqrt(d) ) / (2 * a);

        if (t < 0)
            t = ( -b + sqrt(d) ) / (2 * a);
        return IResult(true, t);
    }
}

Ray3::IResult Ray3::intersects(const Plane3& p) const
{
	float denom = p.mNormal.Dot(mDir);
	if (abs(denom) < std::numeric_limits<float>::epsilon())
	{
		return IResult(false, 0.f);
	}
	else
	{
		float nom = p.mNormal.Dot(mOrigin) + p.mConstant;
		float t = -(nom/denom);
		return IResult(t>=0, t);
	}
}

Ray3::IResult Ray3::intersects(const AABB& aabb, Vec3& normal) const
{
	float min = 1.0f;
	float pseudo_min = 0.0f;
	float pseudo_max = 1000.0f;
	bool collide = RayAABB(mOrigin, mDirInv, mSigns, aabb, min, normal, pseudo_min, pseudo_max);
	return IResult(collide, min);
}


void Ray3::SetDir(const Vec3& dir)
{ 
	mDir = dir; 
	mDirInv.x = mDir.x == 0.0f ? LARGE_FLOAT : 1.0f / mDir.x;
	mDirInv.y = mDir.y == 0.0f ? LARGE_FLOAT : 1.0f / mDir.y;
	mDirInv.z = mDir.z == 0.0f ? LARGE_FLOAT : 1.0f / mDir.z;
	mSigns = Vec3I(mDir.x < 0.0 ? 1 : 0, 
		mDir.y < 0.0 ? 1 : 0, 
		mDir.z < 0.0 ? 1 : 0);
}

}