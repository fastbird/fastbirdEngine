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
#include "Vec3.h"
namespace fb
{
	class BVSphere : public BoundingVolume
	{
	public:
		BVSphere();
		BVSphere(const Vec3& center, Real radius);
		BoundingVolume& operator=(const BoundingVolume& other);
		virtual ~BVSphere(){}
		//--------------------------------------------------------------------
		// BoundingVolume Interfaces
		//--------------------------------------------------------------------
		virtual int GetBVType() const {return BV_SPHERE;}
		virtual void SetCenter (const Vec3& center) { mCenter = center; }
		virtual void SetRadius (Real fRadius) { mRadius = fRadius; }
		virtual const Vec3& GetCenter () const { return mCenter;}
		virtual Real GetRadius () const { return mRadius; }

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
		virtual void Merge(const Vec3& pos);		
		virtual fb::Vec3 GetSurfaceFrom(const Vec3& src, Vec3& normal);
		virtual void Invalidate();
		virtual bool Contain(const Vec3& pos) const;
		virtual Vec3 GetRandomPosInVolume(const Vec3* nearLocal=0) const;

	private:
		Vec3 mCenter;
		Real mRadius;

		std::vector<Vec3> mVertices;
	};
}