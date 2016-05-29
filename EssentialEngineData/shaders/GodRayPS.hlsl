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

#ifdef _MULTI_SAMPLE
Texture2DMS<float4> gFrameTexture : register(t0);
#else
Texture2D gFrameTexture : register(t0);
#endif 

//---------------------------------------------------------------------------
struct QuadVS_Output
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

static const float  NUM_SAMPLES = 16.0;

//---------------------------------------------------------------------------
// PS
//---------------------------------------------------------------------------
float4 GodRayPS_PixelShader(QuadVS_Output Input
#ifdef _MULTI_SAMPLE
	, uint sampleIndex : SV_SampleIndex
#endif
	) : SV_TARGET
{
	float2 screenlightPos = gShaderParams[0].xy;
	float Density = gShaderParams[0].z;
	float Decay = gShaderParams[0].w;
	float Weight = gShaderParams[1].x;
	float Exposure = gShaderParams[1].y;
	float2 texCoord = Input.Tex;
	// Calculate vector from pixel to light source in screen space.  
	float2 deltaTexCoord = (texCoord - screenlightPos);
	// Divide by number of samples and scale by control factor.  
	deltaTexCoord *= 1.0f / NUM_SAMPLES * Density;
#ifdef _MULTI_SAMPLE
	// Store initial sample.  
	float3 color = gFrameTexture.Load(int2(Input.Pos.xy), sampleIndex);
#else
	float3 color = gFrameTexture.Sample(gLinearSampler, Input.Tex);
#endif
	// Set up illumination decay factor.  
	half illuminationDecay = 1.0f;
	// Evaluate summation from Equation 3 NUM_SAMPLES iterations.  
	for (int i = 0; i < NUM_SAMPLES; i++)
	{
		// Step sample location along ray.  
		texCoord -= deltaTexCoord;
#ifdef _MULTI_SAMPLE
		// Retrieve sample at new location.  
		float3 smp = gFrameTexture.Load(int2(texCoord.xy * gScreenSize), sampleIndex);
#else
		float3 smp = gFrameTexture.Sample(gLinearSampler, texCoord);
#endif
		// Apply sample attenuation scale/decay factors.  
		smp *= illuminationDecay * Weight;
		// Accumulate combined color.  
		color += smp;
		// Update exponential decay factor.  
		illuminationDecay *= Decay;
	}
	// Output final color with a further scale control factor.  
	return float4(color * Exposure, 1);
}