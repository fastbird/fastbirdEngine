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
float samplelumfinal_PixelShader(QuadVS_Output Input) : SV_TARGET
{
	float fResampleSum = 0.0f;    
	
    for (int y=-2; y<=2; ++y)
	{
		for(int x=-2; x<=2; ++x)
		{
			fResampleSum += gSrcTexture.Sample(gPointSampler, Input.Tex, int2(x, y));
		}
	}
    
    // Divide the sum to complete the average, and perform an exp() to complete
    // the average luminance calculation
    fResampleSum = min(exp(fResampleSum/25.0), 10.0);
    
    return float4(fResampleSum, fResampleSum, fResampleSum, 1.0f);
}