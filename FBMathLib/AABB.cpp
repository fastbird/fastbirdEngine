/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#include "stdafx.h"
#include "AABB.h"
using namespace fb;
#undef min
#undef max

AABB::AABB()
{
	Invalidate();
}

bool AABB::IsValid() const
{
	return mMax >= mMin;
}

void AABB::Invalidate()
{
	mMin = Vec3::MAX;
	mMax = Vec3::MIN;
}

void AABB::Merge(const Vec3& point)
{
	mMin.KeepLesser(point);
	mMax.KeepGreater(point);
}

void AABB::Merge(const AABB& aabb){
	mMin.x = std::min(mMin.x, aabb.mMin.x);
	mMin.y = std::min(mMin.y, aabb.mMin.y);
	mMin.z = std::min(mMin.z, aabb.mMin.z);
	mMax.x = std::max(mMax.x, aabb.mMax.x);
	mMax.y = std::max(mMax.y, aabb.mMax.y);
	mMax.z = std::max(mMax.z, aabb.mMax.z);
}

const Vec3& AABB::GetMin() const { return mMin; }
const Vec3& AABB::GetMax() const { return mMax; }

void AABB::SetMin(const Vec3& min) { mMin = min; }
void AABB::SetMax(const Vec3& max) { mMax = max; }

Vec3 AABB::GetCenter() const { return (mMin + mMax) * .5f; }

Vec3 AABB::GetExtents() const{
	return (mMax - mMin) * .5f;
}

void AABB::Translate(const Vec3& pos)
{
	mMin += pos;
	mMax += pos;
}

bool AABB::Contain(const Vec3&pos) const
{
	if (pos.x < mMin.x || pos.y < mMin.y || pos.z < mMin.z ||
		pos.x > mMax.x || pos.y > mMax.y || pos.z > mMax.z)
		return false;

	return true;
}

void AABB::GetPoints(Vec3 points[8]) const{
	points[0] = Vec3(mMax.x, mMin.y, mMax.z);
	points[1] = Vec3(mMin.x, mMin.y, mMax.z);
	points[2] = Vec3(mMax.x, mMin.y, mMin.z);
	points[3] = Vec3(mMin.x, mMin.y, mMin.z);

	points[4] = Vec3(mMax.x, mMax.y, mMax.z);
	points[5] = Vec3(mMin.x, mMax.y, mMax.z);
	points[6] = Vec3(mMax.x, mMax.y, mMin.z);
	points[7] = Vec3(mMin.x, mMax.y, mMin.z);
}