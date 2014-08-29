Texture2D gSrcLumTexture0 : register(t0);
Texture2D gSrcLumTexture1 : register(t1);
Texture2D gSrcLumTexture2 : register(t2);

//---------------------------------------------------------------------------
struct QuadVS_Output
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

//---------------------------------------------------------------------------
// PS
//---------------------------------------------------------------------------
float luminanceavgps_PixelShader(QuadVS_Output Input) : SV_TARGET
{
	 float lum = gSrcLumTexture0.Load(int3(0, 0, 0)).r*0.005 +
				 gSrcLumTexture1.Load(int3(0, 0, 0)).r*0.005 +
				 gSrcLumTexture2.Load(int3(0, 0, 0)).r*0.99;
				 
	return lum;

}

