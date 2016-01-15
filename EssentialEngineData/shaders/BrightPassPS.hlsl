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
#include "EssentialEngineData/shaders/ShaderCommon.h"
//---------------------------------------------------------------------------
#ifdef _MULTI_SAMPLE
Texture2DMS<float4> gHDRTexture : register(t0);
#else
Texture2D gHDRTexture : register(t0);
#endif 

Texture2D gLumTexture : register(t1);

//---------------------------------------------------------------------------
struct QuadVS_Output
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

static const float  LUM_WHITE = 1.5f;
static const float  BRIGHT_THRESHOLD = 0.7f;
static const float  BRIGHT_PASS_OFFSET     = 1.0f; // Offset for BrightPass filter
//---------------------------------------------------------------------------
// PS
//---------------------------------------------------------------------------
float4 brightpassps_PixelShader(QuadVS_Output Input
#ifdef _MULTI_SAMPLE
	,uint sampleIndex : SV_SampleIndex
#endif
) : SV_TARGET
{
	float3 vColor = 0.0f;
	float fLum = max(0.2, gLumTexture.Sample(gPointSampler, float2(0, 0)));
	//float fLum = gLumTexture.Sample(gPointSampler, float2(0, 0));
#ifdef _MULTI_SAMPLE
	float2 textureSize = gMaterialParam[0].xy;
	vColor = gHDRTexture.Load(Input.Tex.xy*textureSize, sampleIndex).rgb;
#else
	vColor = gHDRTexture.Sample(gPointSampler, Input.Tex).rgb;
#endif	

	// Bright pass and tone mapping
	vColor *= gMiddleGray / fLum;	
	vColor = max(0.0f, vColor - BRIGHT_THRESHOLD);	
	vColor /= (BRIGHT_PASS_OFFSET + vColor);

	return float4(vColor, 1.0f);
}