#pragma once
#include "Vec3.h"
#include "Quat.h"
namespace fb{
	class Frustum{
	public:
		Vec3 mOrigin;
		Quat mOrientation;

		float mRightSlope; // x/y
		float mLeftSlope;
		float mTopSlope; // z/y
		float mBottomSlope;
		float mNear, mFar;


		Frustum();
		void SetData(float near, float far, float fov, float aspectRatio);
	};
}