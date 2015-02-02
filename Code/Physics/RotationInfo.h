#pragma once

namespace fastbird
{
	struct RotationInfo
	{
		RotationInfo()
		: mUpdateDir(false)
		, mTorqueInversed(false)
		, mForce(1)
		, mDestDir(0, 1, 0)
		, mNeedToAlignRight(false)
		{

		}
		btVector3 mDestDir;
		btScalar mInitAngle;
		btVector3 mInitAxis;
		bool mUpdateDir;
		bool mNeedToAlignRight;
		bool mTorqueInversed;
		float mForce;
	};
}