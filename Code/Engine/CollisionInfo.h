#pragma once
#include <CommonLib/FBColShape.h>

namespace fastbird
{
	class IMeshObject;
	struct CollisionInfo
	{
		FBColShape::Enum mColShapeType;
		Transformation mTransform;
		SmartPtr<IMeshObject> mCollisionMesh;
	};

	typedef std::vector< CollisionInfo > COLLISION_INFOS;
}