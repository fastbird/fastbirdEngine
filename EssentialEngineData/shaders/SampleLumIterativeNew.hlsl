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

#include "EssentialEngineData/shaders/Constants.h"

//---------------------------------------------------------------------------
#ifdef _MULTI_SAMPLE
Texture2DMS<float4> gSrcTexture : register(t0);
#else
Texture2D gSrcTexture : register(t0);
#endif 


//---------------------------------------------------------------------------
static const float3 LUMINANCE_VECTOR = float3(0.2125f, 0.7154f, 0.0721f);
//static const float3 LUMINANCE_VECTOR = float3(.299f, .587f, .114f);

//---------------------------------------------------------------------------
struct QuadVS_Output
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

//---------------------------------------------------------------------------
// PS
//---------------------------------------------------------------------------
float samplelumiterativenew_PixelShader(QuadVS_Output Input) : SV_TARGET
{
	float fResampleSum = 0.0f;
	float vSample;
#ifdef _MULTI_SAMPLE
	float2 textureSize = gMaterialParam[0].xy;
#endif
	for (int y = -1; y <= 1; ++y)
	{
		for (int x = -1; x <= 1; ++x)
		{
#ifdef _MULTI_SAMPLE
			vSample = gSrcTexture.Load(int2(Input.Tex.xy * textureSize), sampleIndex, int(x, y)).r;
#else
			vSample = gSrcTexture.Sample(gPointSampler, Input.Tex, int2(x, y)).r;
#endif
			fResampleSum += vSample;
		}
	}

	// Divide the sum to complete the average
	fResampleSum /= 9.0;

	return fResampleSum;
}