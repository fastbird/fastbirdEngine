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
#include "Plane3.h"

namespace fb
{
	
Plane3::Plane3()
{
}

Plane3::Plane3(const Plane3& other)
	: mNormal(other.mNormal)
	, mConstant(other.mConstant)
{
}

Plane3::Plane3(const Vec3& normal, Real c)
	: mNormal(normal)
	, mConstant(c)
{
}

Plane3::Plane3(const Vec3& normal, const Vec3& p)
	: mNormal(normal)
{
	mConstant = mNormal.Dot(p);

}

Plane3::Plane3(const Vec3& p0, const Vec3& p1, const Vec3& p2)
{
	Vec3 edge1 = p1 - p0;
	Vec3 edge2 = p2 - p0;
	mNormal = edge1.Cross(edge2);
	mNormal.Normalize();
	mConstant = mNormal.Dot(p0);
}

//---------------------------------------------------------------------------
Plane3 Plane3::operator=(const Plane3& other)
{
	mNormal = other.mNormal;
	mConstant = other.mConstant;
	return *this;
}

//---------------------------------------------------------------------------
int Plane3::WhichSide(const Vec3& p) const
{
	Real distance = DistanceTo(p);
	if (distance<0.0f)
		return -1;
	if (distance>0.0)
		return 1;
	return 0;
}

Real Plane3::DistanceTo(const Vec3& p) const
{
	return mNormal.Dot(p) - mConstant;
}

}