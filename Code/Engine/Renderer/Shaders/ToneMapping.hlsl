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
SamplerState gBloomSampler : register(s2);
//---------------------------------------------------------------------------
struct QuadVS_Output
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

static const float  MIDDLE_GRAY = 0.52f;
static const float  LUM_WHITE = 1.5f;

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
	float vLum = max(gLumTexture.Sample(gLumSampler, float2(0, 0)).r, 0.2f);
	//float vLum = gLumTexture.Sample(gLumSampler, float2(0, 0)).r;
#ifdef _MULTI_SAMPLE
	float4 vColor = gHDRTexture.Load(int2(Input.Pos.xy),sampleIndex);
#else
	float4 vColor = gHDRTexture.Sample(gHDRSampler, Input.Tex);
#endif
	//return vColor;
	float3 vBloom = gBloomTexture.Sample(gBloomSampler, Input.Tex);
	
	//float3 CurrentColor = FilmicTonemap( vColor*16 );	
    //float3 WhiteScale = FilmicTonemap( 11.2 );
	//return float4( (CurrentColor) / WhiteScale+0.4f*vBloom, 1.0f );   
	
	// Tone mapping
	vColor.rgb *= MIDDLE_GRAY / (vLum + 0.001f);
	vColor.rgb *= (1.0f + vColor / LUM_WHITE);
	vColor.rgb /= (1.0f + vColor);

	vColor.rgb += 0.7f * vBloom;
	vColor.a = 1.0f;

	return vColor;
}