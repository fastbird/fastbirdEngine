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
#include "BoundingVolume.h"
#include "AABB.h"
namespace fb
{
	class BVaabb: public BoundingVolume
	{
	public:
		BVaabb();
		BVaabb& operator=(const BVaabb& other);
		BoundingVolume& operator=(const BoundingVolume& other);
		
		virtual BVType GetBVType() const { return BV_AABB; }
		virtual void SetCenter (const Vec3& center);
		virtual void SetRadius (Real fRadius);
		virtual const Vec3& GetCenter () const;
		virtual Real GetRadius () const;

		virtual void ComputeFromData(const Vec3* pVertices, size_t numVertices);
		virtual void StartComputeFromData();
		virtual void AddComputeData(const Vec3* pVertices, size_t numVertices);
		virtual void AddComputeData(const Vec3& vert);
		virtual void EndComputeFromData();
		virtual void TransformBy(const Transformation& transform,
			BoundingVolumePtr result);
		virtual int WhichSide(const Plane3& plane) const;
		virtual bool TestIntersection(const Ray3& ray) const;
		virtual bool TestIntersection(BoundingVolume* pBV) const;

		virtual void Merge(const BoundingVolume* pBV);
		virtual void Merge(const Vec3& worldPos);

		virtual fb::Vec3 GetSurfaceFrom(const Vec3& source, Vec3& normal);
		virtual void Invalidate(){ mAABB.Invalidate(); }

		void SetAABB(const AABB& aabb);
		const AABB& GetAABB() const { return mAABB; }

		void Expand(Real e);
		virtual bool Contain(const Vec3& pos) const;
		virtual Vec3 GetRandomPosInVolume(const Vec3* nearLocal = 0) const;
	

	private:
		AABB mAABB;
		Vec3 mCenter;
		Real mRadius;
	};
}