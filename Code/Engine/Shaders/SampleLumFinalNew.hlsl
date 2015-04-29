#include "Constants.h"

//---------------------------------------------------------------------------
Texture2D gSrcTexture : register(t0);

//---------------------------------------------------------------------------
struct QuadVS_Output
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

//---------------------------------------------------------------------------
// PS
//---------------------------------------------------------------------------
float samplelumfinalnew_PixelShader(QuadVS_Output Input) : SV_TARGET
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

	return exp(fResampleSum);
}