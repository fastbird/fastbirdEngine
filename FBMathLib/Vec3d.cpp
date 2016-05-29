#include "stdafx.h"
#include "Vec3d.h"
#include "Vec3.h"
using namespace fb;

Vec3d::Vec3d()
{

}

Vec3d::Vec3d(double x, double y, double z) 
	: x(x)
	, y(y)
	, z(z)
{

}
Vec3d::Vec3d(const Vec3& other) 
	: x(other.x)
	, y(other.y)
	, z(other.z)
{

}

Vec3d Vec3d::NormalizeCopy() const
{
	Vec3d result = *this;
	result.Normalize();
	return result;
}

double Vec3d::Normalize() {
	double length = sqrt(x*x + y*y + z*z);
	if (length > 0.0)
	{
		double invLength = 1.0 / length;
		x *= invLength;
		y *= invLength;
		z *= invLength;
	}

	return length;
}

Vec3d Vec3d::operator-(const Vec3d& other) const {
	return Vec3d{ x - other.x, y - other.y, z - other.z };
}