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
float4 downscale2x2_PixelShader(QuadVS_Output Input
#ifdef _MULTI_SAMPLE
	, uint sampleIndex : SV_SampleIndex
#endif

	) : SV_TARGET
{
	float4 vSample = 0.0f;
	float2 textureSize = gMaterialParam[0].xy;
	for (int i = 0; i < 4; i++)
	{
#ifdef _MULTI_SAMPLE		
		vSample += gSrcTexture.Load((Input.Tex.xy + gSampleOffsets[i].xy)*textureSize, sampleIndex);
#else
		vSample += gSrcTexture.Sample(gPointSampler, Input.Tex.xy + gSampleOffsets[i]);
#endif
	}

	return vSample / 4;
}