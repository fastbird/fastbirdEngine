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
#include "FBCommonHeaders/Types.h"
namespace fb
{
	static const Real PI = 3.1415926535897932384626433832795f;
	static const Real INV_PI = 0.31830988618379067153776752674503f;
	static const Real HALF_PI = 1.5707963267948966192313216916398f;
	static const Real INV_HALF_PI = 0.63661977236758134307553505349004f;
	static const Real F_PI = 0.78539816339744830961566084581988f;
	static const Real TWO_PI = 6.283185307179586476925286766559f;
	static const Real EPSILON = 0.00001f;
	static const Real ZERO_TOLERANCE = 1e-06f;
	static const Real ROOT_2 = 1.4142135623730950488016887242097f;
	static const Real INV_ROOT_2 = 0.70710678118654752440084436210485f;
	static const Real ROOT_3 = 1.7320508075688772935274463415059f;
	static const Real LOG2 = 0.69314718055994530941723212145818f;
	static const Real INV_LOG2 = 1.4426950408889634073599246810019f;
	static const Real LARGE_Real = 1e18f; // keep LARGE_Real*LARGE_Real < FLT_MAX
}