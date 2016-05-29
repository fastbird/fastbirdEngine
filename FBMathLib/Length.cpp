#include "stdafx.h"
#include "Length.h"
#include "Vec3.h"
namespace fb {
	Real Length(Real x) {
		return std::abs(x);
	}
	Real Length(const Vec3& x) {
		return x.Length();
	}
}