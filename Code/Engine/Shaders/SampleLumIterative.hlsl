#include "Constants.h"

//---------------------------------------------------------------------------
#ifdef _MULTI_SAMPLE
Texture2DMS<float4> gSrcTexture : register(t0);
#else
Texture2D gSrcTexture : register(t0);
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
float samplelumiterative_PixelShader(QuadVS_Output Input) : SV_TARGET
{

	float fResampleSum = 0.0f;

	for (int y=-2; y<=2; ++y)
	{
		for(int x=-2; x<=2; ++x)
		{
			fResampleSum += gSrcTexture.Sample(gPointSampler, Input.Tex, int2(x, y));
		}
	}

	// Divide the sum to complete the average
	fResampleSum /= 25.0;

	return float4(fResampleSum, fResampleSum, fResampleSum, 1.0f);
}