#pragma once
#ifndef _fastbird_BoundingVolume_header_included_
#define _fastbird_BoundingVolume_header_included_

#include <CommonLib/SmartPtr.h>

namespace fastbird
{
	class Transformation;
	class Plane3;
	class Ray3;
	class Vec3;
	class BoundingVolume : public ReferenceCounter
	{
	public:
		enum BVType
		{
			BV_SPHERE,
			BV_AABB,
			BV_COUNT,
		};

		// Create default bounding volume which is a sphere
		static BoundingVolume* Create(BVType type = BV_SPHERE);

		BoundingVolume() : mAlwaysPass(false) {}
		virtual ~BoundingVolume(){}

		virtual int GetBVType() const = 0;
		virtual void SetCenter (const Vec3& center) = 0;
		virtual void SetRadius (float fRadius) = 0;
		virtual const Vec3& GetCenter () const = 0;
		virtual float GetRadius () const = 0;

		virtual void ComputeFromData(const Vec3* pVertices, size_t numVertices) = 0;
		virtual void StartComputeFromData() = 0;
		virtual void AddComputeData(const Vec3* pVertices, size_t numVertices) = 0;
		virtual void EndComputeFromData() = 0;
		virtual void TransformBy(const Transformation& transform,
			BoundingVolume* result) = 0;
		virtual int WhichSide(const Plane3& plane) const { assert(0); return -1;}
		virtual int WhichSide(const Vec3& min, const Vec3& max) const { assert(0); return -1;}
		virtual bool TestIntersection(const Ray3& ray) const = 0;
		virtual bool TestIntersection(BoundingVolume* pBV) const = 0;

		virtual void Merge(const BoundingVolume* pBV) = 0;
		virtual void Merge(const Vec3& pos) = 0;
		virtual BoundingVolume& operator= (const BoundingVolume& other) = 0;

		virtual fastbird::Vec3 GetSurfaceTo(const Vec3& target, Vec3& normal) = 0;
		virtual void Invalidate() = 0;

		void SetAlwaysPass(bool p) { mAlwaysPass = p; }

	protected:
		bool mAlwaysPass;


	};
}
#endif //_fastbird_BoundingVolume_header_included_