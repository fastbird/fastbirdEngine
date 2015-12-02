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
	class Plane3
	{
	public:
		Vec3 mNormal;
		Real mConstant;

		//Dot(N,X) = c
		Plane3();
		Plane3(const Plane3& other);
		Plane3(const Vec3& normal, Real c);
		Plane3(const Vec3& normal, const Vec3& p);
		Plane3(const Vec3& p0, const Vec3& p1, const Vec3& p2);

		//-------------------------------------------------------------------
		Plane3 operator=(const Plane3& other);

		//-------------------------------------------------------------------
		int WhichSide(const Vec3& p) const;
		Real DistanceTo(const Vec3& p) const;		
	};
}

#endif //_fastbird_Plane3_header_included_