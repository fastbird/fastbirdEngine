#include <CommonLib/StdAfx.h>
#include <CommonLib/Collision/GeomCollisions.h>
#include <CommonLib/Math/Vec3I.h>

namespace fastbird
{
	bool RayAABB(const Vec3& ro, const Vec3& rdi, 
		const Vec3I& raySign, // 1 if negative
		const AABB& aabb,
		float& min, Vec3& normal,
		float pseudo_min, float pseudo_max)
	{
		// 0 : x, 1: y, 2: z
		int collisionFace = 0;
		// impact times.
		float max, ymin, ymax, zmin, zmax;
		Vec3 bounds[] = {aabb.GetMin(), aabb.GetMax()};
		min = (bounds[raySign.x].x - ro.x) * rdi.x;
		max = (bounds[1-raySign.x].x - ro.x) * rdi.x;
		ymin = (bounds[raySign.y].y - ro.y) * rdi.y;
		ymax = (bounds[1-raySign.y].y - ro.y) * rdi.y;

		if ( (min > ymax) || (ymin > max) )
			return false;

		if (ymin > min)
		{
			min = ymin;
			collisionFace = 1;
		}

		if (ymax < max)
			max = ymax;

		zmin = (bounds[raySign.z].z - ro.z) * rdi.z;
		zmax = (bounds[1-raySign.z].z - ro.z) * rdi.z;

		if ( (min > zmax) || (zmin > max) )
			return false;
		if (zmin > min)
		{
			min = zmin;
			collisionFace = 2;
		}

		if (zmax < max)
			max = zmax;

		static float normalElem[] = {-1.0f, 1.0f}; // reversed order.
		normal = Vec3(collisionFace==0 ? normalElem[raySign.x] : 0,
			collisionFace==1 ? normalElem[raySign.y] : 0,
			collisionFace==2 ? normalElem[raySign.z] : 0);


		return ( (min < pseudo_max) && (max > pseudo_min) );
	}
}