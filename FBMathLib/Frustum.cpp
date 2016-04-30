#include "stdafx.h"
#include "Frustum.h"
#include "Math.h"
#include "BoundingVolume.h"
using namespace fb;

Frustum::Frustum()
	: mOrigin(0, 0, 0)
{

}

Frustum::Frustum(const Plane& left, const Plane& right, const Plane& bottom, const Plane& top,
	const Plane& near, const Plane& far)
{
	mPlanes[FRUSTUM_PLANE_LEFT] = left;
	mPlanes[FRUSTUM_PLANE_RIGHT] = right;
	mPlanes[FRUSTUM_PLANE_BOTTOM] = bottom;
	mPlanes[FRUSTUM_PLANE_TOP] = top;
	mPlanes[FRUSTUM_PLANE_NEAR] = near;
	mPlanes[FRUSTUM_PLANE_FAR] = far;
	mNear = near.mConstant;
	mFar = far.mConstant;

	auto angle = top.mNormal.AngleBetween(near.mNormal);
	auto topHalfFov = PI - HALF_PI - angle;
	mTopSlope = tan(topHalfFov);
	mBottomSlope = -mTopSlope;

	angle = right.mNormal.AngleBetween(near.mNormal);
	auto rightHalfFov = PI - HALF_PI - angle;
	mRightSlope = tan(rightHalfFov);
	mLeftSlope = -mRightSlope;	
}

void Frustum::SetData(float near, float far, float fov, float aspectRatio){
	mNear = near;
	mFar = far;
	mTopSlope = tan(fov*.5f);
	mBottomSlope = -mTopSlope;
	mRightSlope = mTopSlope * aspectRatio;
	mLeftSlope = -mRightSlope;	
}

void Frustum::UpdatePlaneWithViewProjMat(const Mat44& viewProjMat) {
	mPlanes[FRUSTUM_PLANE_LEFT].mNormal.x = viewProjMat[3][0] + viewProjMat[0][0];
	mPlanes[FRUSTUM_PLANE_LEFT].mNormal.y = viewProjMat[3][1] + viewProjMat[0][1];
	mPlanes[FRUSTUM_PLANE_LEFT].mNormal.z = viewProjMat[3][2] + viewProjMat[0][2];
	mPlanes[FRUSTUM_PLANE_LEFT].mConstant = -(viewProjMat[3][3] + viewProjMat[0][3]);

	mPlanes[FRUSTUM_PLANE_RIGHT].mNormal.x = viewProjMat[3][0] - viewProjMat[0][0];
	mPlanes[FRUSTUM_PLANE_RIGHT].mNormal.y = viewProjMat[3][1] - viewProjMat[0][1];
	mPlanes[FRUSTUM_PLANE_RIGHT].mNormal.z = viewProjMat[3][2] - viewProjMat[0][2];
	mPlanes[FRUSTUM_PLANE_RIGHT].mConstant = -(viewProjMat[3][3] - viewProjMat[0][3]);

	mPlanes[FRUSTUM_PLANE_TOP].mNormal.x = viewProjMat[3][0] - viewProjMat[1][0];
	mPlanes[FRUSTUM_PLANE_TOP].mNormal.y = viewProjMat[3][1] - viewProjMat[1][1];
	mPlanes[FRUSTUM_PLANE_TOP].mNormal.z = viewProjMat[3][2] - viewProjMat[1][2];
	mPlanes[FRUSTUM_PLANE_TOP].mConstant = -(viewProjMat[3][3] - viewProjMat[1][3]);

	mPlanes[FRUSTUM_PLANE_BOTTOM].mNormal.x = viewProjMat[3][0] + viewProjMat[1][0];
	mPlanes[FRUSTUM_PLANE_BOTTOM].mNormal.y = viewProjMat[3][1] + viewProjMat[1][1];
	mPlanes[FRUSTUM_PLANE_BOTTOM].mNormal.z = viewProjMat[3][2] + viewProjMat[1][2];
	mPlanes[FRUSTUM_PLANE_BOTTOM].mConstant = -(viewProjMat[3][3] + viewProjMat[1][3]);

	mPlanes[FRUSTUM_PLANE_NEAR].mNormal.x = viewProjMat[3][0] + viewProjMat[2][0];
	mPlanes[FRUSTUM_PLANE_NEAR].mNormal.y = viewProjMat[3][1] + viewProjMat[2][1];
	mPlanes[FRUSTUM_PLANE_NEAR].mNormal.z = viewProjMat[3][2] + viewProjMat[2][2];
	mPlanes[FRUSTUM_PLANE_NEAR].mConstant = -(viewProjMat[3][3] + viewProjMat[2][3]);

	mPlanes[FRUSTUM_PLANE_FAR].mNormal.x = viewProjMat[3][0] - viewProjMat[2][0];
	mPlanes[FRUSTUM_PLANE_FAR].mNormal.y = viewProjMat[3][1] - viewProjMat[2][1];
	mPlanes[FRUSTUM_PLANE_FAR].mNormal.z = viewProjMat[3][2] - viewProjMat[2][2];
	mPlanes[FRUSTUM_PLANE_FAR].mConstant = -(viewProjMat[3][3] - viewProjMat[2][3]);

	// Renormalise any normals which were not unit length
	for (int i = 0; i<6; i++)
	{
		Real length = mPlanes[i].mNormal.Normalize();
		mPlanes[i].mConstant /= length;
	}
}

bool Frustum::IsCulled(BoundingVolume* pBV) const {
	for (int i = 0; i<6; i++)
	{
		if (pBV->WhichSide(mPlanes[i])<0)
			return true;
	}

	return false;
}

bool Frustum::Contains(const Vec3& point) const {
	for (int i = 0; i < 6; ++i) {
		if (mPlanes[i].Dot(Vec4(point)) <= 0)
			return false;
	}	

	return true;
}

Frustum Frustum::TransformBy(const Mat44& mat)
{
	Plane left(mat * mPlanes[FRUSTUM_PLANE_LEFT].GetVec4());
	Plane right(mat * mPlanes[FRUSTUM_PLANE_RIGHT].GetVec4());
	Plane bottom(mat * mPlanes[FRUSTUM_PLANE_BOTTOM].GetVec4());
	Plane top(mat * mPlanes[FRUSTUM_PLANE_TOP].GetVec4());
	Plane near(mat * mPlanes[FRUSTUM_PLANE_NEAR].GetVec4());
	Plane far(mat * mPlanes[FRUSTUM_PLANE_FAR].GetVec4());
	return Frustum(left, right, bottom, top, near, far);
}

Frustum& Frustum::operator= (const Frustum& other) {
	mOrigin = other.mOrigin;
	mOrientation = other.mOrientation;
	for (int i = 0; i < 6; ++i) {
		mPlanes[i] = other.mPlanes[i];
	}
	
	mRightSlope = other.mRightSlope; // x/y
	mLeftSlope = other.mLeftSlope;
	mTopSlope = other.mTopSlope; // z/y
	mBottomSlope = other.mBottomSlope;
	mNear = other.mNear;
	mFar = other.mFar;
	return *this;
}

const Plane& Frustum::GetPlane(FRUSTUM_PLANE p) const {
	if (p <FRUSTUM_PLANE_NEAR || p > FRUSTUM_PLANE_BOTTOM) {
		Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
		return mPlanes[0];
	}

	return mPlanes[p];
}

const Plane& Frustum::GetNear() const {
	return mPlanes[FRUSTUM_PLANE_NEAR];
}
const Plane& Frustum::GetFar() const {
	return mPlanes[FRUSTUM_PLANE_FAR];
}
const Plane& Frustum::GetLeft() const {
	return mPlanes[FRUSTUM_PLANE_LEFT];
}
const Plane& Frustum::GetRight() const {
	return mPlanes[FRUSTUM_PLANE_RIGHT];
}
const Plane& Frustum::GetTop() const {
	return mPlanes[FRUSTUM_PLANE_TOP];
}
const Plane& Frustum::GetBottom() const {
	return mPlanes[FRUSTUM_PLANE_BOTTOM];
}
