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
	float y = gYTexture.Sample(gLinearSampler, Input.Tex).a;
	float u = gUTexture.Sample(gLinearSampler, Input.Tex).a;
	float v = gVTexture.Sample(gLinearSampler, Input.Tex).a;
	float3 rgb = float3(y + 1.370705*(v-.5f), y - 0.698001*(v-0.5f) - 0.337633*(u-0.5f), y + 1.732446*(u-0.5f));
	rgb = saturate(rgb);
	return float4(rgb, 1);	
}