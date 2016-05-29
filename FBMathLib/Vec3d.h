#pragma once
#include "FBCommonHeaders/Types.h"
namespace fb {
	class Vec3;
	/// double vector
	class Vec3d {
	public:
		double x, y, z;

		Vec3d();
		Vec3d(double x, double y, double z);
		Vec3d(const Vec3& other);
		Vec3d NormalizeCopy() const;
		double Normalize();
		Vec3d operator-(const Vec3d& other) const;
	};
}
