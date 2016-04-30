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

#pragma once
#include "stdafx.h"
#include "Plane.h"
#include "Vec4.h"

namespace fb
{
	
Plane::Plane()
{
}

Plane::Plane(const Plane& other)
	: mNormal(other.mNormal)
	, mConstant(other.mConstant)
{
}

Plane::Plane(const Vec3& normal, Real c)
	: mNormal(normal)
	, mConstant(c)
{
}

Plane::Plane(const Vec3& normal, const Vec3& p)
	: mNormal(normal)
{
	mConstant = mNormal.Dot(p);

}

Plane::Plane(Real x, Real y, Real z, Real c) 
	: mNormal(Vec3(x, y, z))
	, mConstant(c)
{

}

Plane::Plane(const Vec3& p0, const Vec3& p1, const Vec3& p2)
{
	Vec3 edge1 = p1 - p0;
	Vec3 edge2 = p2 - p0;
	mNormal = edge1.Cross(edge2);
	mNormal.Normalize();
	mConstant = mNormal.Dot(p0);
}

Plane::Plane(const Vec4& v)
{
	mNormal = v.GetXYZ();
	mConstant = -v.w;
}

//---------------------------------------------------------------------------
Plane Plane::operator=(const Plane& other)
{
	mNormal = other.mNormal;
	mConstant = other.mConstant;
	return *this;
}

//---------------------------------------------------------------------------
int Plane::WhichSide(const Vec3& p) const
{
	Real distance = DistanceTo(p);
	if (distance<0.0f)
		return -1;
	if (distance>0.0)
		return 1;
	return 0;
}

int Plane::OnSameSide(const Vec3& pa, const Vec3& pb) const {
	auto da = DistanceTo(pa);
	auto db = DistanceTo(pb);

	if (da < 0 && db < 0)
		return -1;

	if (da > 0 && db > 0)
		return 1;

	return 0;
}

Real Plane::DistanceTo(const Vec3& p) const
{
	return mNormal.Dot(p) - mConstant;
}

Real Plane::Dot(const Vec4& p) const {
	return mNormal.x * p.x + mNormal.y * p.y + mNormal.z * p.z - mConstant * p.w;
}

std::pair<Vec3, Vec3> Plane::Clip(const Vec3& pa, const Vec3& pb) const {
	if (pa == pb) {
		auto whichSide = WhichSide(pa);
		if (whichSide >= 0) {
			return{ Vec3(FLT_MAX), Vec3(FLT_MAX) };
		}		
		else {
			return{ Vec3(-FLT_MAX), Vec3(-FLT_MAX) };
		}
	}

	// Get the projection of the segment onto the plane.
	auto direction = pb - pa;
	auto ldotv = mNormal.Dot(direction);
	// Are the line and plane parallel?
	if (ldotv == 0) // line and plane are parallel and maybe coincident
	{
		auto whichSide = WhichSide(pa);
		if (whichSide > 0) {
			return{ Vec3(FLT_MAX), Vec3(FLT_MAX) };
		}
		else if (whichSide == 0.f) {
			return{ pa, pb };
		}
		else {
			return{ Vec3(-FLT_MAX), Vec3(-FLT_MAX) };
		}
	}

	// Not parallel so the line intersects. But does the segment intersect?
	Real t = -Dot(Vec4(pa)) / ldotv; // ldots / ldotv
	// segment does not intersect
	if (t < 0 || t > 1){
		auto whichSide = WhichSide(pa);
		if (whichSide >= 0) {
			return{ Vec3(FLT_MAX), Vec3(FLT_MAX) };
		}	
		else {
			return{ Vec3(-FLT_MAX), Vec3(-FLT_MAX) };
		}
	}
	
	auto p = pa + direction * t;
	if (ldotv > 0)
		return { p, pb };
	else
		return { pa, p };
}

Vec4 Plane::GetVec4() const
{
	return Vec4(mNormal, -mConstant);
}

Vec3 Plane::Intersect(const Vec3& pa, const Vec3& pb) {
	auto t = IntersectDistance(pa, pb);

	if (t== FB_INVALID_REAL)
		return Vec3(FB_INVALID_REAL);

	if (t == FLT_MAX)
		return pa;

	return pa + (pb - pa) * t;
}

Real Plane::IntersectDistance(const Vec3& pa, const Vec3& pb) {
	auto direction = pb - pa;
	Real ldotv = mNormal.Dot(direction);
	if (ldotv == 0) // parallel
	{
		Real ldots = Dot(Vec4(pa));
		if (ldots == 0)
			return FLT_MAX; // coincident
		else
			return FB_INVALID_REAL; // not coincident
	}

	return -Dot(Vec4(pa)) / ldotv; // ldots / ldotv
}

}