
#include "Constants.h"

//---------------------------------------------------------------------------
Texture2D gLastLumTexture : register(t0);
Texture2D gCurLumTexture : register(t1);

//---------------------------------------------------------------------------
struct QuadVS_Output
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

//---------------------------------------------------------------------------
float calculateadaptedlum_PixelShader(QuadVS_Output Input) : SV_TARGET
{
    float fAdaptedLum = gLastLumTexture.Sample(gPointSampler, float2(0.5f, 0.5f));
    float fCurrentLum = gCurLumTexture.Sample(gPointSampler, float2(0.5f, 0.5f));
	//return float4(fCurrentLum, fCurrentLum, fCurrentLum, 1.0);
    
    // The user's adapted luminance level is simulated by closing the gap between
    // adapted luminance and current luminance by 2% every frame, based on a
    // 30 fps rate. This is not an accurate model of human adaptation, which can
    // take longer than half an hour.
    float fNewAdaptation = fAdaptedLum + (fCurrentLum - fAdaptedLum) * ( 1.0 - pow( 0.98f, 30.0 * gDeltaTime ) );
    return float4(fNewAdaptation, fNewAdaptation, fNewAdaptation, 1.0f);
}
