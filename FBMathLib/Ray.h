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

#pragma once
#include "FBCommonHeaders/Types.h"
#include "Vec3.h"
#include "Vec3I.h"
#include "Plane.h"

namespace fb
{
	FB_DECLARE_SMART_PTR(BoundingVolume);
	class AABB;
	class Frustum;
	class Ray
	{
		Vec3 mOrigin;
		Vec3 mDir;
		Vec3 mDirInv;
		Vec3I mSigns;
		Real mDirLen = 0.f;

	public:
		Ray();
		Ray(const Vec3& origin, const Vec3& dir);

		static Ray FromSegment(const Vec3& pa, const Vec3& pb);

		// IntersectionResult
		typedef std::pair<bool, Real> IResult;


		IResult Intersects(const BoundingVolume* pBoundingVolume) const;
		IResult Intersects(const AABB& aabb, Vec3& normal) const;
		IResult Intersects(const Plane& p) const;

		const Vec3& GetDir() const { return mDir; }
		const Vec3& GetDirection() const { return mDir; }
		/// Is the direction is unit vector?
		bool IsUnitVector() const;
		Real GetDirLen() const;
		const Vec3& GetDirInv() const;
		const Vec3& GetOrigin() const { return mOrigin; }
		const Vec3I& GetSigns() const { return mSigns; }
		void SetOrigin(const Vec3& v) { mOrigin = v; }
		void SetDirection(const Vec3& dir);
		Vec3 GetPoint(Real dist) const { return mOrigin + mDir * dist; }
		void AddOffset(const Vec3& v) { mOrigin  += v;}

		// segments
		const Vec3& GetPointA() const;
		const Vec3 GetPointB() const;
		static Vec3 NearestPointOnSegment(const Vec3& p0, const Vec3& p1, const Vec3& p);
		static Real DistanceToSegment(const Vec3& p0, const Vec3& p1, const Vec3& p);
		/// Clip a line segment to a frustum, returning the end points of the 
		/// portion of the segment that is within the frustum.
		/// \return The two points at which the segment intersects the frustum, or 
		/// {-Vec3(FLT_MAX), -Vec3(FLT_MAX)} if the segment does not intersect and the frustum does not fully 
		/// contain it.If the segment is coincident with a plane of the frustum, the
		/// returned segment is the portion of the original segment on that plane, 
		/// clipped to the other frustum planes.
		static std::pair<Vec3, Vec3> ClipToFrustum(const Vec3& pa, const Vec3& pb, const Frustum& frustum);
		static std::pair<Vec3, Vec3> ClipToFrustum(const Vec3& pa, const Vec3& pb, const Frustum& frustum, int maxRecursionCount);		
		Vec3 GetPointAt(Real t) const;
		Real SelfDot() const;
		bool operator == (const Ray& other) const;
		std::string ToString() const;
		Vec3 NearestPointTo(const Vec3& p) const;
		Real DistanceTo(const Vec3& p) const;
		bool IsPointBehindOrigin(const Vec3& point);
		Vec3 NearestIntersectionPoint(const std::vector<Vec3>& intersections);		
	};
}