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
#include "Vec3.h"
#include <algorithm>

namespace fb
{
	class AABB
	{
		Vec3 mMin;
		Vec3 mMax;

	public:
		AABB();
		AABB(const Vec3& min, const Vec3& max);
		bool IsValid() const;
		void Invalidate();
		void Merge(const Vec3& point);
		void Merge(const AABB& aabb);
		const Vec3& GetMin() const;
		const Vec3& GetMax() const;
		void SetMin(const Vec3& min);
		void SetMax(const Vec3& max);
		Vec3 GetCenter() const;
		// (max - min) * .5
		Vec3 GetExtents() const;
		void Translate(const Vec3& pos);
		bool Contain(const Vec3&pos) const;
		// near 4
		//  right top, left top, right bottom, left bottom
		// far 4
		//  right top, left top, right bottom, left bottom
		void GetPoints(Vec3 points[8]) const;
		std::string ToString() const;
		AABB& operator*=(float scale);
	};
}