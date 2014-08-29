//--------------------------------------------------------------------------------------
// File: ui.hlsl
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "Constants.h"

//
Texture2D  gDiffuseTexture : register(t0);
SamplerState gDiffuseSampler : register(s0);

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
struct a2v
{
	float3 Position : POSITION;
	float4 Color	: COLOR;
#ifdef DIFFUSE_TEXTURE
	float2 UV		: TEXCOORD;
#endif
};

struct v2p 
{
	float4 Position   : SV_Position;
#ifdef DIFFUSE_TEXTURE
	float2 UV		: TEXCOORD;
#endif
};

v2p ui_VertexShader( in a2v INPUT )
{
    v2p OUTPUT;

	OUTPUT.Position = float4(INPUT.Position, 1.0);
	OUTPUT.Position.x += gWorld[0][3];
	OUTPUT.Position.y += gWorld[1][3];
#ifdef DIFFUSE_TEXTURE
	OUTPUT.UV = INPUT.UV;
#endif

	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 ui_PixelShader( in v2p INPUT ) : SV_Target
{	
#ifdef DIFFUSE_TEXTURE
	float4 color = gDiffuseColor * gDiffuseTexture.Sample(gDiffuseSampler, INPUT.UV);
	float2 uv2 = (INPUT.UV - gMaterialParam[0].zw) / gMaterialParam[0].xy;
	float highlight = gEmissiveColor.w;
	float2 glowss = smoothstep(0.9, 1.0, abs(uv2*2.0-1.0) * highlight);
	color.xyz = lerp(color.xyz, gEmissiveColor.xyz, glowss.x+glowss.y);
#else
	float4 color = gDiffuseColor;
#endif
    return color;
}