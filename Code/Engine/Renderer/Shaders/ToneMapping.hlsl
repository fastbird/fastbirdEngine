#include "Constants.h"
#include "ShaderCommon.h"
//---------------------------------------------------------------------------
#ifdef _MULTI_SAMPLE
Texture2DMS<float4> gHDRTexture : register(t0);
#else
Texture2D gHDRTexture : register(t0);
#endif 

Texture2D gLumTexture : register(t1);

Texture2D gBloomTexture : register(t2);

Texture2D gStarGlareTexture : register(t3);
//---------------------------------------------------------------------------
struct QuadVS_Output
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

static const float  LUM_WHITE = 1.5f;
static const float3 LUMINANCE_VECTOR = float3(0.2125f, 0.7154f, 0.0721f);
static const float3 BLUE_SHIFT_VECTOR = float3(1.05f, 0.97f, 1.27f); 

float3 FilmicTonemap( in float3 x )
{
  float A = 0.15f; // Shoulder Strength.
  float B = 0.50f; // Linear Strength.
  float C = 0.10f; // Linear Angle.
  float D = 0.20f; // Toe Strength.
  float E = 0.02f; // Toe Numerator.
  float F = 0.30f; // Toe Denominator.
  return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F)) - E/F;
}

//---------------------------------------------------------------------------
// PS
//---------------------------------------------------------------------------
float4 tonemapping_PixelShader(QuadVS_Output Input
#ifdef _MULTI_SAMPLE
	,uint sampleIndex : SV_SampleIndex
#endif
	) : SV_TARGET
{
	float vLum = max(0.2, gLumTexture.Sample(gPointSampler, float2(0.5, 0.5)).r);
#ifdef _MULTI_SAMPLE
	float4 vColor = gHDRTexture.Load(int2(Input.Pos.xy),sampleIndex);
#else
	float4 vColor = gHDRTexture.Sample(gLinearSampler, Input.Tex);
#endif
	//return vColor;
	float3 vBloom = gBloomTexture.Sample(gLinearSampler, Input.Tex);
	float4 vStar = gStarGlareTexture.Sample(gLinearSampler, Input.Tex);
	
	//float3 CurrentColor = FilmicTonemap( vColor*16 );	
    //float3 WhiteScale = FilmicTonemap( 11.2 );
	//return float4( (CurrentColor) / WhiteScale+0.4f*vBloom, 1.0f );   
	
	float fBlueShiftCoefficient = 1.0f - (vLum + 3.0)/2.0;
	fBlueShiftCoefficient = saturate(fBlueShiftCoefficient);

	// Lerp between current color and blue, desaturated copy
	float3 vRodColor = dot( (float3)vColor, LUMINANCE_VECTOR ) * BLUE_SHIFT_VECTOR;
	vColor.rgb = lerp( (float3)vColor, vRodColor, fBlueShiftCoefficient );
	
	// Tone mapping
	vColor.rgb *= gMiddleGray_Star_Bloom_Empty.x / (vLum + 0.001f);
	vColor.rgb *= (1.0f + vColor / LUM_WHITE);
	vColor.rgb /= (1.0f + vColor);

	vColor.rgb += gMiddleGray_Star_Bloom_Empty.y * vStar;
	vColor.rgb += gMiddleGray_Star_Bloom_Empty.z * vBloom;
	
	vColor.a = 1.0f;

	return vColor;
}