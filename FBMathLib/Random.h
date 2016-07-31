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

namespace fb
{
	inline Real Random()
	{
		return (Real)std::rand() / (Real)RAND_MAX;
	}

	/// max is included
	template<class T>
	inline  T Random(T min, T max)
	{
		int divider = (int)(max - min + 1);
		if (divider != 0)
			return min + (std::rand() % divider);
		return min;
	}

	inline Real Random(Real min, Real max)
	{
		auto k = max - min;
		auto offset = (Real)(((Real)std::rand() / (Real)RAND_MAX) * k);
		auto result = min + offset;
		return result;
	}

	inline Vec3 Random(const Vec3& min, const Vec3& max) 
	{
		return Vec3(Random(min.x, max.x),
			Random(min.y, max.y),
			Random(min.z, max.z));
	};
	inline Vec3 RandomDirection()
	{
		return Random(Vec3(-1, -1, -1), Vec3(1, 1, 1)).NormalizeCopy();
	}

	Vec3 RandomDirectionCone(const Vec3& axis, Real theta);

	inline Vec2 Random(const Vec2& min, const Vec2& max) 
	{
		return Vec2(Random(min.x, max.x),
			Random(min.y, max.y));
	};

	//-------------------------------------------------------------------------
	template<class T>
	class Randomizer
	{
	public:
		Randomizer(T min, T max)
		{
			mMin = min;
			mMax = max;
		}
		T Get(T value)
		{
			T factor = Random(mMin, mMax);
			return value + factor;

		}

	private:
		T mMin;
		T mMax;
	};
}