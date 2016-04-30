#pragma once
#include "FBMathLib/Vec3.h"

namespace fb {
	FB_DECLARE_SMART_PTR(Intersection);
	class Intersection {
	protected:
		Vec3 mIntersectionPoint{};
		Vec3 mNormal{};
		Real mIntersectionLength=0.f;		
		bool mIsTangent=false;		
		void* mUserData=0;		

	public:
		/// Unknown intersection point.
		/// Use this when collision occured but the exact point is unknown.
		static const Intersection DummyIntersection;
		// usually use this
		typedef std::vector<Intersection> Array;
			

		Intersection();
		Intersection(const Vec3& intersectionPoint, bool isTangent);
		Intersection(const Vec3& intersectionPoint, const Vec3& normal);
		Intersection(const Vec3& intersectionPoint, Real intersectionLength, bool isTangent);
		/// pointer is not managed by this class.
		void SetUserData(void* data);
		void* GetUserData() const;

		Vec3 GetIntersectionPoint() const;
		void SetIntersectionPoint(const Vec3& intersectionPoint);

		bool IsTangent() const;
		void SetTangent(bool tangent);

		Real GetIntersectionLength() const;
		/// returning the merged list of intersections, sorted by increasing distance 
		/// from the reference point.
		static Array sort(const Vec3& refPoint, const Array& listA, const Array& listB);
		bool operator == (const Intersection& other) const;
		std::string ToString() const;
	};
}