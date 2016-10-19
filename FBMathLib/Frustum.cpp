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

#include "stdafx.h"
#include "Frustum.h"
#include "Math.h"
#include "BoundingVolume.h"
using namespace fb;

Frustum::Frustum()
	: mOrigin(0, 0, 0)
	, mOrthogonal(false)
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
	mOrthogonal  = IsEqual(left.mNormal.Dot(near.mNormal), 0.f, 0.0001f);		

	auto angle = top.mNormal.AngleBetween(near.mNormal);
	auto topHalfFov = PI - HALF_PI - angle;
	mTopSlope = tan(topHalfFov);
	mBottomSlope = -mTopSlope;

	angle = right.mNormal.AngleBetween(near.mNormal);
	auto rightHalfFov = PI - HALF_PI - angle;
	mRightSlope = tan(rightHalfFov);
	mLeftSlope = -mRightSlope;	
}

void Frustum::SetData(float near, float far, float fov, float aspectRatio, bool orthogonal){
	mNear = near;
	mFar = far;
	mTopSlope = tan(fov*.5f);
	mBottomSlope = -mTopSlope;
	mRightSlope = mTopSlope * aspectRatio;
	mLeftSlope = -mRightSlope;	
	mOrthogonal = orthogonal;
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
	
	mCenter = mOrigin +
		mPlanes[FRUSTUM_PLANE_NEAR].mNormal * mNear +
		mPlanes[FRUSTUM_PLANE_NEAR].mNormal * ((mFar - mNear)*.5f);

	// debug code
	//_Validate();
}

void Frustum::_Validate() {
	if (mOrthogonal)
		return;

	Mat33 rot;
	mOrientation.ToRotationMatrix(rot);
	auto dir = rot.Column(1);
	auto up = rot.Column(2);
	if (!IsEqual(mPlanes[FRUSTUM_PLANE_NEAR].mNormal, dir, 0.01f)) {
		Logger::Log(FB_ERROR_LOG_ARG, "Near plane normal is wrong");		
	}
	if (!IsEqual(mPlanes[FRUSTUM_PLANE_FAR].mNormal, -dir, 0.01f)) {
		Logger::Log(FB_ERROR_LOG_ARG, "Far plane normal is wrong");		
	}

	Vec3 lefttopn = mOrientation * Vec3(mLeftSlope * mNear, mNear, mTopSlope * mNear);
	Vec3 leftbottomn = mOrientation * Vec3(mLeftSlope * mNear, mNear, mBottomSlope * mNear);
	Vec3 leftbottomf = mOrientation * Vec3(mLeftSlope * mFar, mFar, mBottomSlope * mFar);
	Vec3 leftNormal = (leftbottomf - leftbottomn).Cross(lefttopn - leftbottomn).NormalizeCopy();
	if (!IsEqual(mPlanes[FRUSTUM_PLANE_LEFT].mNormal, leftNormal, 0.01f)) {
		Logger::Log(FB_ERROR_LOG_ARG, "Left plane normal is wrong");		
	}

	Vec3 righttopn = mOrientation * Vec3(mRightSlope * mNear, mNear, mTopSlope * mNear);
	Vec3 rightbottomn = mOrientation * Vec3(mRightSlope * mNear, mNear, mBottomSlope * mNear);
	Vec3 rightbottomf = mOrientation * Vec3(mRightSlope * mFar, mFar, mBottomSlope * mFar);
	Vec3 rightNormal = (righttopn - rightbottomn).Cross(rightbottomf - rightbottomn).NormalizeCopy();
	if (!IsEqual(mPlanes[FRUSTUM_PLANE_RIGHT].mNormal, rightNormal, 0.01f)) {
		Logger::Log(FB_ERROR_LOG_ARG, "Left plane normal is wrong");		
	}

	Vec3 rightn = mOrientation * Vec3(mRightSlope * mNear, mNear, mTopSlope * mNear);
	Vec3 leftn = mOrientation * Vec3(mLeftSlope * mNear, mNear, mTopSlope * mNear);
	Vec3 leftf = mOrientation * Vec3(mLeftSlope * mFar, mFar, mTopSlope * mFar);
	Vec3 topNormal = (leftf - leftn).Cross(rightn - leftn).NormalizeCopy();
	if (!IsEqual(mPlanes[FRUSTUM_PLANE_TOP].mNormal, topNormal, 0.01f)) {
		Logger::Log(FB_ERROR_LOG_ARG, "Top plane normal is wrong");		
	}

	 rightn = mOrientation * Vec3(mRightSlope * mNear, mNear, mBottomSlope * mNear);
	 leftn = mOrientation * Vec3(mLeftSlope * mNear, mNear, mBottomSlope * mNear);
	 leftf = mOrientation * Vec3(mLeftSlope * mFar, mFar, mBottomSlope * mFar);
	Vec3 bottomNormal = (rightn - leftn).Cross(leftf - leftn).NormalizeCopy();
	if (!IsEqual(mPlanes[FRUSTUM_PLANE_BOTTOM].mNormal, bottomNormal, 0.01f)) {
		Logger::Log(FB_ERROR_LOG_ARG, "Bottom plane normal is wrong");		
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

/// near (left, bottom), (left, top), (right, bottom), (right, top)
/// far (left, bottom), (left, top), (right, bottom), (right, top)
std::vector<Vec3> Frustum::ToPoints() const
{
	std::vector<Vec3> ret(8);
	auto nleft = mLeftSlope * mNear;
	auto nright = mRightSlope * mNear;
	auto ntop = mTopSlope * mNear;
	auto nbottom = mBottomSlope * mNear;
	ret[0] = Vec3(nleft, mNear, nbottom);
	ret[1] = Vec3(nleft, mNear, ntop);
	ret[2] = Vec3(nright, mNear, nbottom);
	ret[3] = Vec3(nright, mNear, ntop);
	auto fleft = mLeftSlope * mFar;
	auto fright = mRightSlope * mFar;
	auto ftop = mTopSlope * mFar;
	auto fbottom = mBottomSlope * mFar;
	ret[4] = Vec3(fleft, mFar, fbottom);
	ret[5] = Vec3(fleft, mFar, ftop);
	ret[6] = Vec3(fright, mFar, fbottom);
	ret[7] = Vec3(fright, mFar, ftop);
	for (auto i = 0; i < 8; ++i) {
		ret[i] = mOrientation * ret[i] + mOrigin;
	}
	return ret;
}

void Frustum::SetOrthogonal(bool ortho)
{
	mOrthogonal = ortho;
}