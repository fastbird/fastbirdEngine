#include "stdafx.h"
#include "Frustum.h"
#include "Math.h"
using namespace fb;

Frustum::Frustum()
	: mOrigin(0, 0, 0)
{

}

void Frustum::SetData(float near, float far, float fov, float aspectRatio){
	mNear = near;
	mFar = far;
	mRightSlope = tan(fov*.5f);
	mLeftSlope = -mRightSlope;
	mTopSlope = mRightSlope / aspectRatio;
	assert(IsEqual(mTopSlope, (float)tan(fov*.5f / aspectRatio), 0.001f));
	mBottomSlope = -mTopSlope;
}