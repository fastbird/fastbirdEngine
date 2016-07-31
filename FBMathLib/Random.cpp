#include "stdafx.h"
#include "Random.h"
#include "Quat.h"

namespace fb {
	Vec3 RandomDirectionCone(const Vec3& axis, Real theta) {
		Vec3 up;
		if (IsEqual(axis, Vec3::UNIT_X, 0.001f)) {
			up = axis.Cross(Vec3::UNIT_Z);
		}
		else {
			up = axis.Cross(Vec3::UNIT_X);
		}
		Vec3 right = axis.Cross(up);
		up.Normalize();
		right.Normalize();
		auto rightRotAngle = Random(-theta, theta);
		auto upRotAngle = Random(-theta, theta);

		Quat rightRot = Quat::CreateFromAngleAxis(rightRotAngle, right);
		Quat upRot = Quat::CreateFromAngleAxis(upRotAngle, up);
		auto randomdir = rightRot * axis;
		randomdir = upRot * randomdir;
		return randomdir;
	}
}