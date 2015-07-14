#include "Constants.h"

Texture2D gYTexture : register(t0);
Texture2D gUTexture : register(t1);
Texture2D gVTexture : register(t2);


//---------------------------------------------------------------------------
struct QuadVS_Output
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

//---------------------------------------------------------------------------
// PS
//---------------------------------------------------------------------------
float4 yuvmovie_PixelShader(QuadVS_Output Input) : SV_TARGET
{
	float y = gYTexture.Sample(gLinearSampler, Input.Tex);
	float u = gUTexture.Sample(gLinearSampler, Input.Tex);
	float v = gVTexture.Sample(gLinearSampler, Input.Tex);
	float3 rgb = float3(y + 1.28033*v, u - 0.21482*u - 0.38059*v, y + 2.12798*u)
	return float4(rgb, 1);	
}