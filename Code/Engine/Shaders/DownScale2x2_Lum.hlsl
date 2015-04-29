#include "Constants.h"

//---------------------------------------------------------------------------
#ifdef _MULTI_SAMPLE
Texture2DMS<float4> gHDRTexture : register(t0);
#else
Texture2D gHDRTexture : register(t0);
#endif 


//---------------------------------------------------------------------------
static const float3 LUM_VECTOR = float3(.299, .587, .114);

//---------------------------------------------------------------------------
struct QuadVS_Output
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

//---------------------------------------------------------------------------
// PS
//---------------------------------------------------------------------------
float downscale2x2_lum_PixelShader(QuadVS_Output Input
#ifdef _MULTI_SAMPLE
	,uint sampleIndex : SV_SampleIndex
#endif
) : SV_TARGET
{
#ifdef _MULTI_SAMPLE
	float l = gHDRTexture.Load(Input.Pos.xy, sampleIndex).rgb * LUM_VECTOR;
#else
	float l = gHDRTexture.Sample(gLinearSampler, Input.Tex).rgb * LUM_VECTOR;
#endif	
	return max(l, 0.0);
}