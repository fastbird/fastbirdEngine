//----------------------------------------------------------------------------
// File: ThickLine.hlsl
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Constant Buffer
//----------------------------------------------------------------------------
#include "Constants.h"

Texture2D  gDiffuseTexture : register(t0);

//----------------------------------------------------------------------------
// Vertex Shader
//----------------------------------------------------------------------------
struct a2v
{
	float4 pos		: POSITION;
	float4 color	: COLOR;
	float2 uv		: TEXCOORD;
};

//----------------------------------------------------------------------------
struct v2p
{
	float4 pos		: SV_Position;
	float4 color	: COLOR;
	float2 uv		: TEXCOORD;
};

//----------------------------------------------------------------------------
v2p thickline_VertexShader( in a2v IN )
{
    v2p OUT;

	OUT.pos = mul(gWorldViewProj, IN.pos);
	OUT.color = IN.color;
	OUT.uv = IN.uv;

	return OUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
struct PS_OUT
{
	float4 color0 : SV_Target0;
	float4 color1 : SV_Target1;
};

PS_OUT thickline_PixelShader(in v2p IN) : SV_Target
{
	float4 outColor = IN.color;
#ifdef DIFFUSE_TEXTURE
	outColor *= gDiffuseTexture.Sample(gLinearSampler, IN.uv);
#endif
	PS_OUT outdata;
	outdata.color0 = outColor;
	outdata.color1 = outColor;
	return outdata;
}