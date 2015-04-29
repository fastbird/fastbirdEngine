#include "Constants.h"

//---------------------------------------------------------------------------
#ifdef _MULTI_SAMPLE
Texture2DMS<float4> gHDRTexture : register(t0);
#else
Texture2D gHDRTexture : register(t0);
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
float sampleluminitialnew_PixelShader(QuadVS_Output Input
#ifdef _MULTI_SAMPLE
	, uint sampleIndex : SV_SampleIndex
#endif
	) : SV_TARGET
{
	float l = dot(gHDRTexture.Sample(gLinearSampler, Input.Tex).xyz, LUMINANCE_VECTOR);
	return log(l+0.0001f);
}