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

#ifndef _ModuleImmutable_h
#define _ModuleImmutable_h
cbuffer cbImmutable
{
	static float3 gPosOffsets[24] =
	{
		// sides
		float3(-1.0f, -1.0f, -1.0f),
		float3(-1.0f, -1.0f, 1.0f),
		float3(1.0f, -1.0f, -1.0f),
		float3(1.0f, -1.0f, 1.0f),

		float3(1.0f, -1.0f, -1.0f),
		float3(1.0f, -1.0f, 1.0f),
		float3(1.0f, 1.0f, -1.0f),
		float3(1.0f, 1.0f, 1.0f),

		float3(1.0f, 1.0f, -1.0f),
		float3(1.0f, 1.0f, 1.0f),
		float3(-1.0f, 1.0f, -1.0f),
		float3(-1.0f, 1.0f, 1.0f),

		float3(-1.0f, 1.0f, -1.0f),
		float3(-1.0f, 1.0f, 1.0f),
		float3(-1.0f, -1.0f, -1.0f),
		float3(-1.0f, -1.0f, 1.0f),

		// up
		float3(-1.0f, -1.0f, 1.0f),
		float3(-1.0f, 1.0f, 1.0f),
		float3(1.0f, -1.0f, 1.0f),
		float3(1.0f, +1.0f, 1.0f),

		// down
		float3(-1.0f, 1.0f, -1.0f),
		float3(-1.0f, -1.0f, -1.0f),
		float3(1.0f, 1.0f, -1.0f),
		float3(1.0f, -1.0f, -1.0f),
	};

	static float3 gNormal[24] =
	{
		float3(0, -1, 0),
		float3(0, -1, 0),
		float3(0, -1, 0),
		float3(0, -1, 0),

		float3(1, 0, 0),
		float3(1, 0, 0),
		float3(1, 0, 0),
		float3(1, 0, 0),

		float3(0, 1, 0),
		float3(0, 1, 0),
		float3(0, 1, 0),
		float3(0, 1, 0),

		float3(-1, 0, 0),
		float3(-1, 0, 0),
		float3(-1, 0, 0),
		float3(-1, 0, 0),

		float3(0, 0, 1),
		float3(0, 0, 1),
		float3(0, 0, 1),
		float3(0, 0, 1),

		float3(0, 0, -1),
		float3(0, 0, -1),
		float3(0, 0, -1),
		float3(0, 0, -1),
	};

	static float3 gTangent[24] =
	{
		float3(1, 0, 0),
		float3(1, 0, 0),
		float3(1, 0, 0),
		float3(1, 0, 0),

		float3(0, 1, 0),
		float3(0, 1, 0),
		float3(0, 1, 0),
		float3(0, 1, 0),

		float3(-1, 0, 0),
		float3(-1, 0, 0),
		float3(-1, 0, 0),
		float3(-1, 0, 0),

		float3(0, -1, 0),
		float3(0, -1, 0),
		float3(0, -1, 0),
		float3(0, -1, 0),

		float3(1, 0, 0),
		float3(1, 0, 0),
		float3(1, 0, 0),
		float3(1, 0, 0),

		float3(-1, 0, 0),
		float3(-1, 0, 0),
		float3(-1, 0, 0),
		float3(-1, 0, 0),
	};

	static float2 gUV[24] =
	{
		// sides
		float2(0, 1),
		float2(0, 0),
		float2(1, 1),
		float2(1, 0),

		float2(0, 1),
		float2(0, 0),
		float2(1, 1),
		float2(1, 0),

		float2(0, 1),
		float2(0, 0),
		float2(1, 1),
		float2(1, 0),

		float2(0, 1),
		float2(0, 0),
		float2(1, 1),
		float2(1, 0),

		float2(0, 1),
		float2(0, 0),
		float2(1, 1),
		float2(1, 0),

		float2(0, 1),
		float2(0, 0),
		float2(1, 1),
		float2(1, 0),
	};
};
#endif 