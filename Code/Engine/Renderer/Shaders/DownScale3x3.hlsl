#include "Constants.h"
//---------------------------------------------------------------------------
Texture2D ToneMap : register(t0);
SamplerState ToneMapSampler : register (s0);


//---------------------------------------------------------------------------
struct QuadVS_Output
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

//---------------------------------------------------------------------------
// PS
//---------------------------------------------------------------------------
float downscale3x3_PixelShader(QuadVS_Output Input) : SV_TARGET
{
	//return ToneMap.Sample(ToneMapSampler, Input.Tex).r;	
    float l=0;
    float2 screenSize = gMaterialParam[0].xy;
    for( int y = -1; y <= 1; y++ )
    {
        for( int x = -1; x <= 1; x++ )
        {
            // Compute the sum of color values
            l += ToneMap.Load( int3((Input.Tex)*screenSize, 0) , int2(x,y) ).r;
        }
    }
	l *= 0.1111111111111111111;
    
    return l;
}