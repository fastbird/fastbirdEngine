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
#ifndef _fastbird_Plane3_header_included_
#define _fastbird_Plane3_header_included_

#include "Vec3.h"

namespace fb
{
	class Vec4;
	//Dot(N,X) = c
	class Plane
	{
	public:
		typedef std::vector<Plane> Array;
		Vec3 mNormal;
		Real mConstant;

		//Dot(N,X) = c
		Plane();
		Plane(const Plane& other);
		Plane(const Vec3& normal, Real c);
		Plane(const Vec3& normal, const Vec3& p);
		Plane(Real x, Real y, Real z, Real c);
		Plane(const Vec3& p0, const Vec3& p1, const Vec3& p2);
		/// mNormal = v.xyz
		/// mConstantc = -v.w
		Plane(const Vec4& v);

		//-------------------------------------------------------------------
		Plane operator=(const Plane& other);

		//-------------------------------------------------------------------
		int WhichSide(const Vec3& p) const;
		/// Determines whether two points are on the same side of a plane.
		/// \return 1, when both are in the positive side.
		/// -1, when both are in the negative side.
		/// 0, when both are on the plane.
		int OnSameSide(const Vec3& pa, const Vec3& pb) const;
		Real DistanceTo(const Vec3& p) const;		
		/// Calculates the 4-D dot product of this plane with a vector.
		Real Dot(const Vec4& p) const;
		/// Clip a line segment to this plane.
		/// \return two points both on the positive side of the plane. the first is
		/// intersected point when the line direction is the same with the plane normal
		/// otherwise opposite.
		/// if the segment doesn't intersect the plane, returns {Vec3(-FLT_MAX), Vec3(-FLT_MAX)}
		/// when the points reside negative side of the plane, {Vec3(FLT_MAX), Vec3(FLT_MAX)} 
		/// when the points reside positive side of the plane.
		/// returns input value when the segement is coincident on the plane.
		std::pair<Vec3, Vec3> Clip(const Vec3& pa, const Vec3& pb) const;
		/// returning (mNormal, -mConstant)
		Vec4 GetVec4() const;
		/// line intersect
		Vec3 Intersect(const Vec3& pa, const Vec3& pb);
		Real IntersectDistance(const Vec3& pa, const Vec3& pb);
	};
}

#endif //_fastbird_Plane3_header_included_