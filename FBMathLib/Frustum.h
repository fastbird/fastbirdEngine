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