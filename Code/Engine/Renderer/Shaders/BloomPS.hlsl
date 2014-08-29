#include "Constants.h"

//---------------------------------------------------------------------------
#ifdef _MULTI_SAMPLE
Texture2DMS<float4> gSrcTexture : register(t0);
#else
Texture2D gSrcTexture : register(t0);
#endif
SamplerState gSrcSampler : register(s0);

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
	float2 vSamplePosition;

	for (int iSample = 0; iSample < 15; iSample++)
	{
		// Sample from adjacent points
	#ifdef _MULTI_SAMPLE
		int2 vSamplePosition = Input.Pos.xy + gSampleOffsets[iSample] * gScreenSize;
		vColor = gSrcTexture.Load(vSamplePosition, sampleIndex);
	#else
		vSamplePosition = Input.Tex + gSampleOffsets[iSample];
		vColor = gSrcTexture.Sample(gSrcSampler, vSamplePosition);
	#endif
		

		vSample += gSampleWeights[iSample] * vColor;
	}

	return vSample;
}