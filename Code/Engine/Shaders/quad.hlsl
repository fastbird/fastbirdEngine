//----------------------------------------------------------------------------
// File: Quad.hlsl
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Constant Buffer
//----------------------------------------------------------------------------
#include "Constants.h"

#ifdef DIFFUSE_TEXTURE
Texture2D gTexture : register(t0);
#endif

//----------------------------------------------------------------------------
// Vertex Shader
//----------------------------------------------------------------------------
struct a2v
{
	float4 pos		: POSITION;
	float4 color	: COLOR;
#ifdef DIFFUSE_TEXTURE
	float2 uv : TEXCOORD;
#endif
};

//----------------------------------------------------------------------------
struct v2p
{
	float4 pos		: SV_Position;
	float4 color	: COLOR;
#ifdef DIFFUSE_TEXTURE	
	float2 uv		: TEXCOORD;
#endif	
};

//----------------------------------------------------------------------------
v2p quad_VertexShader( in a2v IN )
{
    v2p OUT;

	OUT.pos = mul(gWorld, IN.pos);
	OUT.color = IN.color;
#ifdef DIFFUSE_TEXTURE	
	OUT.uv = IN.uv;
#endif

	return OUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 quad_PixelShader( in v2p IN ) : SV_Target
{
#ifdef DIFFUSE_TEXTURE	
	return IN.color * gTexture.Sample(gLinearSampler, IN.uv) * gDiffuseColor;
#else
	return IN.color * gDiffuseColor;
#endif
}