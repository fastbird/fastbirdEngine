#pragma once
#include "Vec3.h"
#include "Quat.h"
#include "Plane.h"
namespace fb{
	class Mat44;
	class BoundingVolume;
	class Frustum{
	public:
		enum FRUSTUM_PLANE
		{
			FRUSTUM_PLANE_NEAR = 0,
			FRUSTUM_PLANE_FAR = 1,
			FRUSTUM_PLANE_LEFT = 2,
			FRUSTUM_PLANE_RIGHT = 3,
			FRUSTUM_PLANE_TOP = 4,
			FRUSTUM_PLANE_BOTTOM = 5,
			NumPlanes= 6,
		};

		Vec3 mOrigin;
		Vec3 mCenter;		
		Quat mOrientation;
		Plane mPlanes[6];		

		float mRightSlope; // x/y
		float mLeftSlope;
		float mTopSlope; // z/y
		float mBottomSlope;
		float mNear, mFar;
		bool mOrthogonal;


		Frustum();
		Frustum(const Plane& left, const Plane& right, const Plane& bottom, const Plane& top,
			const Plane& near, const Plane& far);
		void SetData(float near, float far, float fov, float aspectRatio, bool orthogonal);
		void UpdatePlaneWithViewProjMat(const Mat44& viewProj);		
		bool IsCulled(BoundingVolume* pBV) const;
		bool Contains(const Vec3& point) const;
		Frustum TransformBy(const Mat44& mat);		
		const Plane& GetPlane(FRUSTUM_PLANE p) const;
		const Plane& GetNear() const;
		const Plane& GetFar() const;
		const Plane& GetLeft() const;
		const Plane& GetRight() const;
		const Plane& GetTop() const;
		const Plane& GetBottom() const;
		/// near (left, bottom), (left, top), (right, bottom), (right, top)
		/// far (left, bottom), (left, top), (right, bottom), (right, top)
		std::vector<Vec3> ToPoints() const;
		void SetOrthogonal(bool ortho);

	protected:
		void _Validate();
	};
}