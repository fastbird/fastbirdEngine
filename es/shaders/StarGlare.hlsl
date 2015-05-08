#include "Constants.h"

//---------------------------------------------------------------------------
Texture2D gSrcTexture : register(t0);

//---------------------------------------------------------------------------
struct QuadVS_Output
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

//-----------------------------------------------------------------------------
// Name: StarPS
// Type: Pixel shader                                      
// Desc: Each star is composed of up to 8 lines, and each line is created by
//       up to three passes of this shader, which samples from 8 points along
//       the current line.
//-----------------------------------------------------------------------------
float4 starglare_PixelShader(QuadVS_Output Input) : SV_Target
{
	float4 vSample = 0.0f;
	float4 vColor = 0.0f;

	float2 vSamplePosition;

	// Sample from eight points along the star line
	for (int iSample = 0; iSample < 8; iSample++)
	{
		vSamplePosition = Input.Tex + gSampleOffsets[iSample];
		vSample = gSrcTexture.Sample(gLinearSampler, vSamplePosition);
		vColor += gSampleWeights[iSample] * vSample;
	}

	return vColor;
}
