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

#include "Constants.h"

//---------------------------------------------------------------------------
#ifdef _MULTI_SAMPLE
Texture2DMS<float4> gSrcTexture : register(t0);
#else
Texture2D gSrcTexture : register(t0);
#endif

//---------------------------------------------------------------------------
struct QuadVS_Output
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

//---------------------------------------------------------------------------
// PS
//---------------------------------------------------------------------------
float4 bloomps_PixelShader(QuadVS_Output Input
#ifdef _MULTI_SAMPLE
,uint sampleIndex : SV_SampleIndex
#endif

) : SV_TARGET
{
	float4 vSample = 0.0f;
	float4 vColor = 0.0f;
	float2 textureSize = gMaterialParam[0].xy;
	for (int iSample = 0; iSample < 15; iSample++)
	{
		// Sample from adjacent points
	#ifdef _MULTI_SAMPLE
		//Load Function
		//void Load(in int2 coord,in int sampleindex,in int2 offset);
		int2 vSamplePosition = (Input.Tex.xy + gSampleOffsets[iSample]) * textureSize;
		vColor = gSrcTexture.Load(vSamplePosition, sampleIndex);
	#else
		float2 vSamplePosition = Input.Tex + gSampleOffsets[iSample];
		vColor = gSrcTexture.Sample(gLinearSampler, vSamplePosition);
	#endif
		

		vSample += gSampleWeights[iSample] * vColor;
	}

	return vSample;
}