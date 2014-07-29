#pragma once
#include <CommonLib/Math/Vec3.h>

namespace fastbird
{
	void CreateSphereMesh(float radius, int nRings, int nSegments,
		std::vector<Vec3>& pos, std::vector<UINT16>& index, std::vector<Vec3>* normal, std::vector<Vec2>* uv);
}