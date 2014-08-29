#include "Constants.h"

//---------------------------------------------------------------------------
#ifdef _MULTI_SAMPLE
Texture2DMS<float4> gHDRTexture : register(t0);
#else
Texture2D gHDRTexture : register(t0);
#endif 
SamplerState gHDRSampler : register (s0);


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
	//float l = gHDRTexture.Sample(gHDRSampler, Input.Tex).xyz * LUM_VECTOR;
#ifdef _MULTI_SAMPLE	
	float l = gHDRTexture.Load(Input.Pos.xy, sampleIndex).rgb * LUM_VECTOR;
#else
	float l = gHDRTexture.Sample(gHDRSampler, Input.Tex).rgb * LUM_VECTOR;
#endif	
	return max(l, 0.0);
/*
	float4 vColor = {0, 0, 0, 0};
    
    for( int y = -1; y < 1; y++ )
    {
        for( int x = -1; x < 1; x++ )
        {
            // Compute the sum of color values
            vColor += gHDRTexture.Load( int3(Input.Tex*gScreenSize, 0), int2(x,y) );
        }
    }
    float lum = dot(vColor, LUM_VECTOR);
    lum*=0.25;
    
    return float4(lum, lum, lum, 1.0f);*/
}