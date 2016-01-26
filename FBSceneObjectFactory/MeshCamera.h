#pragma once
#include <string>
#include "FBCommonHeaders/VectorMap.h"
#include "FBMathLib/Transformation.h"
namespace fb{
	struct MeshCamera{
		std::string mName;
		Transformation mLocation;		
		/// radian
		float mFov;
		float mAspectRatio;
		float mNear;
		float mFar;
		MeshCamera()
		{
		}

		MeshCamera(std::string name, const Transformation& transform)
			: mName(name), mLocation(transform)
		{

		}
	};

	typedef VectorMap<std::string, MeshCamera> MeshCameras;
}