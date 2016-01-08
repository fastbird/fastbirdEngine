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
	mTopSlope = tan(fov*.5f);
	mBottomSlope = -mTopSlope;
	mRightSlope = mTopSlope * aspectRatio;
	mLeftSlope = -mRightSlope;
	
}