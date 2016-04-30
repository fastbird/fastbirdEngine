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
#ifndef _fastbird_BoundingVolume_header_included_
#define _fastbird_BoundingVolume_header_included_
#include <memory>
namespace fb
{
	class Transformation;
	class Plane;
	class Ray;
	class Vec3;
	class BoundingVolume;
	class Intersection;
	typedef std::shared_ptr<BoundingVolume> BoundingVolumePtr;
	typedef std::weak_ptr<BoundingVolume> BoundingVolumeWeakPtr;
	class BoundingVolume
	{
	protected:
		bool mAlwaysPass;

	public:
		enum BVType
		{
			BV_SPHERE,
			BV_AABB,
			BV_COUNT,
		};
		// Create default bounding volume which is a sphere
		static BoundingVolumePtr Create(BVType type = BV_SPHERE);

		BoundingVolume() : mAlwaysPass(false) {}
		virtual ~BoundingVolume(){}		
		virtual BoundingVolume& operator=(const BoundingVolume& other) = 0;

	public:
		void SetAlwaysPass(bool p);
		bool GetAlwaysPass() const;

		virtual BVType GetBVType() const = 0;
		virtual void SetCenter (const Vec3& center) = 0;
		virtual void SetRadius (Real fRadius) = 0;
		virtual const Vec3& GetCenter () const = 0;
		virtual Real GetRadius () const = 0;
		virtual void ComputeFromData(const Vec3* pVertices, size_t numVertices) = 0;
		virtual void StartComputeFromData() = 0;
		virtual void AddComputeData(const Vec3* pVertices, size_t numVertices) = 0;
		virtual void AddComputeData(const Vec3& vert) = 0;
		virtual void EndComputeFromData() = 0;
		virtual void TransformBy(const Transformation& transform,BoundingVolumePtr result) = 0;
		virtual int WhichSide(const Plane& plane) const;
		virtual int WhichSide(const Vec3& min, const Vec3& max) const;
		/// intersected?, distant
		virtual std::vector<Intersection> Intersect(const Ray& ray) const = 0;
		virtual std::vector<Intersection> Intersect(BoundingVolume* pBV) const = 0;

		/// deprecated
		virtual bool TestIntersection(const Ray& ray) const = 0;
		virtual bool TestIntersection(BoundingVolume* pBV) const = 0;
		virtual Vec3 GetRandomPosInVolume(const Vec3* nearLocal = 0) const = 0;
		virtual bool Contain(const Vec3& pos) const = 0;
		virtual void Merge(const BoundingVolume* pBV) = 0;
		virtual void Merge(const Vec3& pos) = 0;
		virtual fb::Vec3 GetSurfaceFrom(const Vec3& src, Vec3& normal) = 0;
		virtual void Invalidate() = 0;		
	};
}
#endif //_fastbird_BoundingVolume_header_included_