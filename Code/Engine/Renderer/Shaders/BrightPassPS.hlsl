//---------------------------------------------------------------------------
#ifdef _MULTI_SAMPLE
Texture2DMS<float4> gHDRTexture : register(t0);
#else
Texture2D gHDRTexture : register(t0);
#endif 

SamplerState gHDRSampler : register (s0);

Texture2D gLumTexture : register(t1);
SamplerState gLumSampler : register (s1);

Texture2D gBloomTexture : register(t2);
SamplerState gBloomSampler : register (s2);

//---------------------------------------------------------------------------
struct QuadVS_Output
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

static const float  MIDDLE_GRAY = 0.72f;
static const float  LUM_WHITE = 1.5f;
static const float  BRIGHT_THRESHOLD = 0.7f;
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
	float fLum = gLumTexture.Sample(gLumSampler, float2(0, 0));
#ifdef _MULTI_SAMPLE
	vColor = gHDRTexture.Load(int2(Input.Pos.xy), sampleIndex).rgb;
#else
	vColor = gHDRTexture.Sample(gHDRSampler, Input.Tex).rgb;
#endif	

	// Bright pass and tone mapping
	vColor = max(0.0f, vColor - BRIGHT_THRESHOLD);
	vColor *= MIDDLE_GRAY / (fLum + 0.001f);
	vColor *= (1.0f + vColor / LUM_WHITE);
	vColor /= (1.0f + vColor);

	return float4(vColor, 1.0f);
}