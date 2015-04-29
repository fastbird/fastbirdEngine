#include "Constants.h"

//---------------------------------------------------------------------------
#ifdef _MULTI_SAMPLE
Texture2DMS<float4> gHDRTexture : register(t0);
#else
Texture2D gHDRTexture : register(t0);
#endif 


//---------------------------------------------------------------------------
static const float3 LUMINANCE_VECTOR = float3(0.2125f, 0.7154f, 0.0721f);

//---------------------------------------------------------------------------
struct QuadVS_Output
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

//---------------------------------------------------------------------------
// PS
//---------------------------------------------------------------------------
float sampleluminitial_PixelShader(QuadVS_Output Input
#ifdef _MULTI_SAMPLE
	, uint sampleIndex : SV_SampleIndex
#endif
	) : SV_TARGET
{
	float3 vSample = 0.0f;
	float  fLogLumSum = 0.0f;
#ifdef _MULTI_SAMPLE
	float2 textureSize = gMaterialParam[0].xy;
#endif
	for (int y = -1; y <= 1; ++y)
	{
		for (int x = -1; x <= 1; ++x)
		{
#ifdef _MULTI_SAMPLE
			vSample = gHDRTexture.Load(int2(Input.Tex.xy * textureSize), sampleIndex, int(x, y));
#else
			vSample = gHDRTexture.Sample(gLinearSampler, Input.Tex, int2(x, y));
#endif
			fLogLumSum += log(dot(vSample, LUMINANCE_VECTOR) + 0.0001f);
		}
	}

	// Divide the sum to complete the average
	fLogLumSum /= 9.0;

	return float4(fLogLumSum, fLogLumSum, fLogLumSum, 1.0f);
}