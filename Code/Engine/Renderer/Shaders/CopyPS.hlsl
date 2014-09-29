#include "ShaderCommon.h"
#ifdef _MULTI_SAMPLE
Texture2DMS<float4> gSrcTexture : register(t0);
#else
Texture2D gSrcTexture : register(t0);
#endif


//---------------------------------------------------------------------------
struct QuadVS_Output
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

//---------------------------------------------------------------------------
// PS
//---------------------------------------------------------------------------
float4 copyps_PixelShader(QuadVS_Output Input
#ifdef _MULTI_SAMPLE
,uint sampleIndex : SV_SampleIndex
#endif

) : SV_TARGET
{
	//Load Function
	//void Load(in int2 coord,in int sampleindex,in int2 offset);

#ifdef _MULTI_SAMPLE
	return gSrcTexture.Load(Input.Pos.xy, sampleIndex);
#else

	#ifdef _POINT_SAMPLER
	return gSrcTexture.Sample(gPointSampler, Input.Tex);
	#else //POINT_SAMPLER
	return gSrcTexture.Sample(gLinearSampler, Input.Tex);
	#endif //POINT_SAMPLER
#endif
}