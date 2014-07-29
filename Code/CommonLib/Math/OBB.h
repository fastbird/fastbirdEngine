#pragma once
#include <CommonLib/Math/AABB.h>
#include <CommonLib/Math/Transformation.h>
namespace fastbird
{

class OBB
{
public:
	Transformation mTransform;
	AABB mAABB;
};

}