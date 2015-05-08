#include "Constants.h"

//---------------------------------------------------------------------------
Texture2D gSrcTexture0 : register(t0);
Texture2D gSrcTexture1 : register(t1);

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
float4 mergetextures2ps_PixelShader(QuadVS_Output Input) : SV_Target
{
	float4 vColor = 0.0f;

	vColor += gSrcTexture0.Sample(gLinearSampler, Input.Tex);
	vColor += gSrcTexture1.Sample(gLinearSampler, Input.Tex);

	return vColor*0.5;
}
