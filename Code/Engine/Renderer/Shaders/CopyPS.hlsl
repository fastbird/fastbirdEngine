#ifdef _MULTI_SAMPLE
Texture2DMS<float4> gSrcTexture : register(t0);
#else
Texture2D gSrcTexture : register(t0);
#endif
SamplerState gSrcSampler : register(s0);


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
#ifdef _MULTI_SAMPLE
	return gSrcTexture.Load(Input.Pos.xy, sampleIndex);
#else
	return gSrcTexture.Sample(gSrcSampler, Input.Tex);
#endif
}