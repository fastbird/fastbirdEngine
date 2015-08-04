#include <CommonLib/StdAfx.h>
#include <CommonLib/Math/Vec3I.h>

namespace fastbird
{
	const Vec3I Vec3I::UNIT_X = Vec3I(1, 0, 0);
	const Vec3I Vec3I::UNIT_Y = Vec3I(0, 1, 0);
	const Vec3I Vec3I::UNIT_Z = Vec3I(0, 0, 1);
	const Vec3I Vec3I::ZERO = Vec3I(0, 0, 0);
	const Vec3I Vec3I::MAX = Vec3I(INT_MAX, INT_MAX, INT_MAX);
}