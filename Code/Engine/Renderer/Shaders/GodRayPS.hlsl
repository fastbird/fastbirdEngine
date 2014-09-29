#include "Constants.h"

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
float4 godrayps_PixelShader(QuadVS_Output Input
#ifdef _MULTI_SAMPLE
	, uint sampleIndex : SV_SampleIndex
#endif
	) : SV_TARGET
{
	float2 screenlightPos = gMaterialParam[0].xy;
	float Density = gMaterialParam[0].z;
	float Decay = gMaterialParam[0].w;
	float Weight = gMaterialParam[1].x;
	float Exposure = gMaterialParam[1].y;
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