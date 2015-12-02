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

#include "Vec3.h"
#include "Vec3I.h"
#include "Plane3.h"

namespace fb
{
	FB_DECLARE_SMART_PTR(BoundingVolume);
	class AABB;
	class Ray3
	{
		Vec3 mOrigin;
		Vec3 mDir;
		Vec3 mDirInv;
		Vec3I mSigns;

	public:
		Ray3();
		Ray3(const Vec3& origin, const Vec3& dir);

		// IntersectionResult
		typedef std::pair<bool, Real> IResult;
		IResult Intersects(const BoundingVolume* pBoundingVolume) const;
		IResult Intersects(const AABB& aabb, Vec3& normal) const;
		IResult Intersects(const Plane3& p) const;

		const Vec3& GetDir() const { return mDir; }
		const Vec3& GetOrigin() const { return mOrigin; }
		void SetOrigin(const Vec3& v) { mOrigin = v; }
		void SetDirection(const Vec3& dir);
		Vec3 GetPoint(Real dist) const { return mOrigin + mDir * dist; }
		void AddOffset(const Vec3& v) { mOrigin  += v;}		
	};
}