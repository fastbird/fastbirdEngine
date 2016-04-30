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
#include "Ray.h"
#include "Math.h"
#include "BoundingVolume.h"
#include "Frustum.h"

namespace fb
{

Ray Ray::FromSegment(const Vec3& pa, const Vec3& pb)
{
	return Ray(pa, pb - pa);
}

Ray::Ray()
{
}

Ray::Ray(const Vec3& origin, const Vec3& dir)
	: mOrigin(origin)
{
	SetDirection(dir);
}

Ray::IResult Ray::Intersects(const BoundingVolume* pBoundingVolume) const
{
	assert(pBoundingVolume);
	// to relative space
	Vec3 toOrigin = mOrigin - pBoundingVolume->GetCenter();
	Real radius = pBoundingVolume->GetRadius();

	// Check origin inside first
	if (toOrigin.LengthSQ() <= radius*radius)
	{
		return Ray::IResult(true, 0.f);
	}
	auto& dir = mDir;
	// t = (-b +/- sqrt(b*b - 4ac)) / 2a
	Real a = dir.Dot(dir);
	Real b = 2 * toOrigin.Dot(dir);
	Real c = toOrigin.Dot(toOrigin) - radius*radius;

	// Discriminant
	Real discriminant = Discriminant(a, b, c);
	if (discriminant < 0)
	{
		// No intersection
		return IResult(false, 0.f);
	}
	else
	{
		Real discriminantRoot = sqrt(discriminant);
		Real t = (-b - discriminantRoot) / (2 * a);
		if (t < 0)
			t = (-b + discriminantRoot) / (2 * a);
		return IResult(true, t);
	}
}

Ray::IResult Ray::Intersects(const Plane& p) const
{
	Real denom = p.mNormal.Dot(mDir);
	if (abs(denom) < std::numeric_limits<Real>::epsilon())
	{
		return IResult(false, 0.f);
	}
	else
	{
		Real nom = p.mNormal.Dot(mOrigin) + p.mConstant;
		Real t = -(nom / denom);
		return IResult(t >= 0, t);
	}
}

Ray::IResult Ray::Intersects(const AABB& aabb, Vec3& normal) const
{
	Real min = 1.0f;
	Real pseudo_min = 0.0f;
	Real pseudo_max = 1000.0f;
	bool collide = RayAABB(mOrigin, mDirInv, mSigns, aabb, min, normal, pseudo_min, pseudo_max);
	return IResult(collide, min);
}

void Ray::SetDirection(const Vec3& dir)
{
	mDir = dir;
	mDirLen = mDir.Length();
	mDirInv.x = mDir.x == 0.0f ? LARGE_Real : 1.0f / mDir.x;
	mDirInv.y = mDir.y == 0.0f ? LARGE_Real : 1.0f / mDir.y;
	mDirInv.z = mDir.z == 0.0f ? LARGE_Real : 1.0f / mDir.z;
	mSigns = Vec3I(
		mDir.x < 0.0 ? 1 : 0,
		mDir.y < 0.0 ? 1 : 0,
		mDir.z < 0.0 ? 1 : 0);
}

float Ray::GetDirLen() const {
	return mDirLen;
}

const Vec3& Ray::GetDirInv() const
{
	return mDirInv;
}

bool Ray::IsUnitVector() const {
	return IsEqual(mDirLen, 1.0f, 0.0001f);
}

const Vec3& Ray::GetPointA() const
{
	return mOrigin;
}

const Vec3 Ray::GetPointB() const
{
	return mOrigin + mDir;
}

// static
Vec3 Ray::NearestPointOnSegment(const Vec3& p0, const Vec3& p1, const Vec3& p)
{
	Vec3 v = p1 - p0;
	Vec3 w = p - p0;
	auto c1 = w.Dot(v);
	auto c2 = v.Dot(v);
	if (c1 <= 0)
		return p0;
	if (c2 <= c1)
		return p1;

	return p0 + v * (c1 / c2);
}

// static
Real Ray::DistanceToSegment(const Vec3& p0, const Vec3& p1, const Vec3& p)
{
	auto pb = NearestPointOnSegment(p0, p1, p);

	return p.DistanceTo(pb);
}
/// Clip a line segment to a frustum, returning the end points of the 
/// portion of the segment that is within the frustum.
/// \return The two points at which the segment intersects the frustum, or 
/// {-Vec3(FLT_MAX), -Vec3(FLT_MAX)} if the segment does not intersect and the frustum does not fully 
/// contain it.If the segment is coincident with a plane of the frustum, the
/// returned segment is the portion of the original segment on that plane, 
/// clipped to the other frustum planes.
/// static function
std::pair<Vec3, Vec3> Ray::ClipToFrustum(const Vec3& pa, const Vec3& pb, const Frustum& frustum)
{
	return ClipToFrustum(pa, pb, frustum, 1);
}
/// static function
std::pair<Vec3, Vec3> Ray::ClipToFrustum(const Vec3& pa, const Vec3& pb, const Frustum& frustum, int maxRecursionCount)
{
	// First do a trivial accept test.
	if (frustum.Contains(pa) && frustum.Contains(pb))
		return{ pa, pb };

	std::pair<Vec3, Vec3> segment{ pa, pb };

	for (int i = 0; i < 6; ++i) {
		auto& p = frustum.GetPlane((Frustum::FRUSTUM_PLANE)i);
		if (p.OnSameSide(segment.first, segment.second) < 0)
			return{ Vec3(FB_INVALID_REAL), Vec3(FB_INVALID_REAL) };
		// Clip the segment to the plane if they intersect.
		auto ipts = p.Clip(segment.first, segment.second);
		if (ipts.first.x != -FLT_MAX && ipts.first.x != FLT_MAX)
		{
			segment = ipts;
		}
	}

	// If one of the initial points was in the frustum then the segment must have been clipped.
	if (frustum.Contains(pa) || frustum.Contains(pb))
		return segment;

	// The segment was clipped by an infinite frustum plane but may still lie outside the frustum.
	// So recurse using the clipped segment.
	if (maxRecursionCount > 0)
		return ClipToFrustum(segment.first, segment.second, frustum, --maxRecursionCount);
	else
		return segment;
}

Vec3 Ray::GetPointAt(Real t) const
{
	return mOrigin + mDir * t;
}

Real Ray::SelfDot() const
{
	return mOrigin.Dot(mDir);
}

bool Ray::operator == (const Ray& other) const
{
	return mOrigin == other.mOrigin && mDir == other.mDir;
}

std::string Ray::ToString() const
{
	return FormatString("Origin: %s, Direction: %s", mOrigin.ToString().c_str(), mDir.ToString().c_str());
}

Vec3 Ray::NearestPointTo(const Vec3& p) const
{
	Vec3 w = p - mOrigin;

	Real c1 = w.Dot(mDir);
	Real c2 = mDir.Dot(mDir);

	return mOrigin + mDir * (c1 / c2);
}

Real Ray::DistanceTo(const Vec3& p) const
{
	return p.DistanceTo(NearestPointTo(p));
}

bool Ray::IsPointBehindOrigin(const Vec3& point)
{
	auto dot = (point - mOrigin).Dot(mDir);
	return dot < 0.0;
}

Vec3 Ray::NearestIntersectionPoint(const std::vector<Vec3>& intersections)
{
	Vec3 intersectionPoint(FB_INVALID_REAL);

	// Find the nearest intersection that's in front of the ray origin.
	Real nearestDistance = FLT_MAX;
	for (auto& point : intersections)
	{
		// Ignore any intersections behind the line origin.
		if (!IsPointBehindOrigin(point))
		{
			Real d = point.DistanceTo(mOrigin);
			if (d < nearestDistance)
			{
				intersectionPoint = point;
				nearestDistance = d;
			}
		}
	}

	return intersectionPoint;
}

}