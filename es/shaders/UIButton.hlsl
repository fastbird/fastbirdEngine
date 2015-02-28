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
	float2 UV		: TEXCOORD;
};

struct v2p 
{
	float4 Position   : SV_Position;
	float2 UV		: TEXCOORD;
};

v2p uibutton_VertexShader( in a2v INPUT )
{
    v2p OUTPUT;

	OUTPUT.Position = float4(INPUT.Position, 1.0);
	OUTPUT.Position.x += gWorld[0][3];
	OUTPUT.Position.y += gWorld[1][3];
	OUTPUT.UV = INPUT.UV;

	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 uibutton_PixelShader( in v2p INPUT ) : SV_Target
{
	float ratio = gMaterialParam[0].x;
	float width = gMaterialParam[0].y;
	float height = gMaterialParam[0].z;
	
	float2 uv = abs(INPUT.UV*2.0-1.0);
	
	// gMaterialParam[1] is edge color
#ifdef BUTTON_EDGE
	if (uv.x > 1.0-(2.0/width))
		return gMaterialParam[1];
	if (uv.y> 1.0-(2.0/height))
		return gMaterialParam[1];
#endif
		
#ifdef DIFFUSE_TEXTURE
    float4 t_color = gDiffuseTexture.Sample(gLinearSampler, INPUT.UV);
    t_color.rgb = t_color.rgb;
	float4 color = gDiffuseColor * t_color;
	float2 uv2 = (INPUT.UV - gMaterialParam[0].zw) / gMaterialParam[0].xy;
	
	float3 edgeColor = {0.3, 0.3, 0.3};
	float gx = smoothstep(1.0-(0.1/ratio), 1.0, abs(uv2.x*2.0-1.0));
	float gy = smoothstep(0.9, 1.0, abs(uv2.y*2.0-1.0));
	color.xyz = lerp(color.xyz, edgeColor, max(gx,gy));
	
	float highlight = gEmissiveColor.w;	
	float glowx = smoothstep(.9 / ratio, 1.0, abs(uv2.x*2.0-1.0) * highlight);
	//float glowx = smoothstep(1.0 - (0.1/ratio), 1.0, abs(uv2.x*2.0-1.0) * highlight);
	//float glowx = smoothstep(0.9, 1.0, abs(uv2.x*2.0-1.0) * highlight);
	float glowy = smoothstep(0.9, 1.0, abs(uv2.y*2.0-1.0) * highlight);
	color.xyz = lerp(color.xyz, gEmissiveColor.xyz, glowx+glowy);
#else
	float4 color = gDiffuseColor;
	color.rgb += gAmbientColor.rgb;
#endif
	
    return color;
}