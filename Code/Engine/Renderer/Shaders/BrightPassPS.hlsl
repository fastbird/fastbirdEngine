#include "Constants.h"
#include "ShaderCommon.h"
//---------------------------------------------------------------------------
#ifdef _MULTI_SAMPLE
Texture2DMS<float4> gHDRTexture : register(t0);
#else
Texture2D gHDRTexture : register(t0);
#endif 

Texture2D gLumTexture : register(t1);

//---------------------------------------------------------------------------
struct QuadVS_Output
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

static const float  LUM_WHITE = 1.5f;
static const float  BRIGHT_THRESHOLD = 0.7f;
static const float  BRIGHT_PASS_OFFSET     = 10.0f; // Offset for BrightPass filter
//---------------------------------------------------------------------------
// PS
//---------------------------------------------------------------------------
float4 brightpassps_PixelShader(QuadVS_Output Input
#ifdef _MULTI_SAMPLE
	,uint sampleIndex : SV_SampleIndex
#endif
) : SV_TARGET
{
	float3 vColor = 0.0f;
	float fLum = gLumTexture.Sample(gPointSampler, float2(0.5, 0.5));
#ifdef _MULTI_SAMPLE
	float2 textureSize = gMaterialParam[0].xy;
	vColor = gHDRTexture.Load(Input.Tex.xy*textureSize, sampleIndex).rgb;
#else
	vColor = gHDRTexture.Sample(gLinearSampler, Input.Tex).rgb;
#endif	

	// Bright pass and tone mapping
	vColor *= gMiddleGray_Star_Bloom_Empty.x / (fLum + 0.001f);	
	vColor = max(0.0f, vColor - BRIGHT_THRESHOLD);	
	vColor /= (BRIGHT_PASS_OFFSET + vColor);

	return float4(vColor, 1.0f);
}