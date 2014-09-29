#include <CommonLib/StdAfx.h>
#include <CommonLib/Curve.h>

using namespace fastbird;

Curve::Curve()
{

}
Curve::~Curve()
{

}

size_t Curve::GetNumPoints() const
{
	return mPoints.size();
}

const fastbird::Vec3 Curve::GetPoint(size_t index, float scale) const
{
	assert(index < mPoints.size());
	return mPoints[index] * scale;
}

void Curve::AddPoints(const fastbird::Vec3& p)
{
	mPoints.push_back(p);
}