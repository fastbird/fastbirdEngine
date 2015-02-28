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
#ifdef _CPU_LUMINANCE
	float vLum = max(0.2, gMaterialParam[0].x);
#else
	float vLum = max(0.2, gLumTexture.Sample(gPointSampler, float2(0.0, 0.0)).r);
#endif
	//return float4(vLum.xxx, 1);
#ifdef _MULTI_SAMPLE
	float4 vColor = gHDRTexture.Load(int2(Input.Pos.xy),sampleIndex);
#else
	float4 vColor = gHDRTexture.Sample(gPointSampler, Input.Tex);
#endif
	float3 originalColor = vColor.rgb;
	float3 vBloom = gBloomTexture.Sample(gLinearSampler, Input.Tex);
	float4 vStar = gStarGlareTexture.Sample(gLinearSampler, Input.Tex);

#ifdef _FILMIC_TONEMAP
	float3 CurrentColor = FilmicTonemap( vColor*2.0 );
    float3 WhiteScale = FilmicTonemap( 2.0 );
	vColor.rgb = (CurrentColor) / WhiteScale;
	vColor.rgb += gStarPower * vStar;
	vColor.rgb += gBloomPower * vBloom;
	return float4( vColor.rgb, 1.0f );   
#else
	
	
	// Lerp between current color and blue, desaturated copy
	float fBlueShiftCoefficient = 1.0f - (vLum + 3.0)/2.0;
	fBlueShiftCoefficient = saturate(fBlueShiftCoefficient);	
	float3 vRodColor = dot( (float3)vColor, LUMINANCE_VECTOR ) * BLUE_SHIFT_VECTOR;
	vColor.rgb = lerp( (float3)vColor, vRodColor, fBlueShiftCoefficient );
	
	// Tone mapping
	vColor.rgb *= gMiddleGray / vLum;
	vColor.rgb *= (1.0f + vColor / LUM_WHITE);
	vColor.rgb /= (1.0f + vColor);

	vColor.rgb += gStarPower * vStar;
	vColor.rgb += gBloomPower * vBloom;
	
	vColor.a = 1.0f;
	
	return vColor;
#endif
}