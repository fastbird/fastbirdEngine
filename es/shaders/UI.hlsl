//--------------------------------------------------------------------------------------
// File: ui.hlsl
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "Constants.h"

//
Texture2D  gDiffuseTexture : register(t0);

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
	#ifdef _UV_ROT
		float2 center = gMaterialParam[0].xy;
		float theta = gTime;
		float c = cos(theta);
		float s = sin(theta);
		float2x2 matRot = { c, s, -s, c};
		float2 uv = INPUT.UV - center;
		uv = mul(matRot, uv);
		OUTPUT.UV =   uv + center;
	#else
		OUTPUT.UV = INPUT.UV;
	#endif
#endif

	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 ui_PixelShader( in v2p INPUT ) : SV_Target
{	
#ifdef DIFFUSE_TEXTURE
	float4 color = gDiffuseTexture.Sample(gLinearSampler, INPUT.UV);
	color *= gDiffuseColor;
	float highlight = gEmissiveColor.w;
	float2 glowss = smoothstep(0.9, 1.0, abs(saturate(INPUT.UV)*2.0-1.0) * highlight);
	color = lerp(color, gEmissiveColor, glowss.x+glowss.y);
#else
	float4 color = gDiffuseColor;
#endif
	
#ifdef _DESATURATE
	color.rgb = (0.212671 * color.r + 0.71516 * color.g + 0.072169 * color.b)*.7;
#endif

	color.rgb += gAmbientColor.xyz;
	color.rgb *= gSpecularColor.xyz;

    return color;
}