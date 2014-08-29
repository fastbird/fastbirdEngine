#pragma once
#include <CommonLib/Math/AABB.h>
namespace fastbird
{
	// ro : ray origin
	// rdi : 1.0 / ray direction
	bool RayAABB(const Vec3& ro, const Vec3& rdi, 
		const Vec3I& raySign,
		const AABB& aabb, 
		float& min, Vec3& normal,
		float pseudo_min, float pseudo_max);

	float GetTimeToExitNDCSquare(const Vec2& pos, const Vec2& dir);

		
}