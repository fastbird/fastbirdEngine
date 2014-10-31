#pragma once
#include <CommonLib/Math/BoundingVolume.h>
#include <CommonLib/Math/Vec3.h>
namespace fastbird
{
	class BVSphere : public BoundingVolume
	{
	public:
		BVSphere();
		BVSphere(const Vec3& center, float radius);
		virtual ~BVSphere(){}
		//--------------------------------------------------------------------
		// BoundingVolume Interfaces
		//--------------------------------------------------------------------
		virtual int GetBVType() const {return BV_SPHERE;}
		virtual void SetCenter (const Vec3& center) { mCenter = center; }
		virtual void SetRadius (float fRadius) { mRadius = fRadius; }
		virtual const Vec3& GetCenter () const { return mCenter;}
		virtual float GetRadius () const { return mRadius; }

		virtual void ComputeFromData(const Vec3* pVertices, size_t numVertices);
		virtual void StartComputeFromData();
		virtual void AddComputeData(const Vec3* pVertices, size_t numVertices);
		virtual void AddComputeData(const Vec3& vert);
		virtual void EndComputeFromData();
		virtual void TransformBy(const Transformation& transform,
			BoundingVolume* result);
		virtual int WhichSide(const Plane3& plane) const;
		virtual bool TestIntersection(const Ray3& ray) const;
		virtual bool TestIntersection(BoundingVolume* pBV) const;
		virtual void Merge(const BoundingVolume* pBV);
		virtual void Merge(const Vec3& pos);
		virtual BoundingVolume& operator= (const BoundingVolume& other);
		virtual fastbird::Vec3 GetSurfaceFrom(const Vec3& src, Vec3& normal);
		virtual void Invalidate();

	private:
		Vec3 mCenter;
		float mRadius;

		std::vector<Vec3> mVertices;
	};
}