//--------------------------------------------------------------------------------------
// File: ui.hlsl
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "Constants.h"

//
Texture2D  gDiffuseTexture : register(t0);

#if defined(_ALPHA_TEXTURE) || defined(_ALPHA_TEXTURE_SEPERATED_UV)
Texture2D gAlphaTexture : register(t1);
#endif

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
struct a2v
{
	float3 Position : POSITION;
	float4 Color	: COLOR;
#if defined(DIFFUSE_TEXTURE) || defined(_ALPHA_TEXTURE)	
	float2 UV		: TEXCOORD0;
#endif
#if defined(_ALPHA_TEXTURE_SEPERATED_UV)
	float2 UV2 : TEXCOORD1;
#endif
};

struct v2p 
{
	float4 Position   : SV_Position;
#if defined(DIFFUSE_TEXTURE) || defined(_ALPHA_TEXTURE)	
	float2 UV		: TEXCOORD;
#endif
#if defined(_ALPHA_TEXTURE_SEPERATED_UV)
	float2 UV2 : TEXCOORD1;
#endif
};

v2p ui_VertexShader( in a2v INPUT )
{
    v2p OUTPUT;

	OUTPUT.Position = mul(gWorld, float4(INPUT.Position, 1));
#if defined(DIFFUSE_TEXTURE) || defined(_ALPHA_TEXTURE)
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

#if defined(_ALPHA_TEXTURE_SEPERATED_UV)
	OUTPUT.UV2 = INPUT.UV2;
#endif
	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 ui_PixelShader( in v2p INPUT ) : SV_Target
{	
#ifdef DIFFUSE_TEXTURE
	float4 color = gDiffuseTexture.Sample(gPointBlackBorderSampler, INPUT.UV);
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
	color.a *= gMaterialParam[4].w;
	
#if defined(_ALPHA_TEXTURE)
	float a = gAlphaTexture.Sample(gPointSampler, INPUT.UV).a;
	color.a *= a;
#endif

#if defined(_ALPHA_TEXTURE_SEPERATED_UV)
	float a = gAlphaTexture.Sample(gPointSampler, INPUT.UV2).a;
	color.a *= a;
#endif
    return color;

}