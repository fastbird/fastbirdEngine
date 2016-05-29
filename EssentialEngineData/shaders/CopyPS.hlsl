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

#include "EssentialEngineData/shaders/ShaderCommon.h"
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
float4 CopyPS_PixelShader(QuadVS_Output Input
#ifdef _MULTI_SAMPLE
,uint sampleIndex : SV_SampleIndex
#endif

) : SV_TARGET
{
	//Load Function
	//void Load(in int2 coord,in int sampleindex,in int2 offset);

#ifdef _MULTI_SAMPLE
	return gSrcTexture.Load(Input.Pos.xy, sampleIndex);
#else

	#ifdef _POINT_SAMPLER
	return gSrcTexture.Sample(gPointSampler, Input.Tex);
	#else //POINT_SAMPLER
	return gSrcTexture.Sample(gLinearSampler, Input.Tex);
	#endif //POINT_SAMPLER
#endif
}