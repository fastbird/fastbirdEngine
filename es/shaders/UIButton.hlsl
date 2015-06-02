//--------------------------------------------------------------------------------------
// File: ui.hlsl
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "Constants.h"

#ifdef _ALPHA_TEXTURE
Texture2D  gDiffuseTexture : register(t0);
#endif 

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

	OUTPUT.Position = mul(gWorld, float4(INPUT.Position, 1));
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
		
#if defined(DIFFUSE_TEXTURE) || defined(_ALPHA_TEXTURE)
    
#endif

	float4 color = gDiffuseColor;
	color.rgb += gAmbientColor.rgb;
	
#ifdef _ALPHA_TEXTURE	
	float4 t_color = gDiffuseTexture.Sample(gLinearSampler, INPUT.UV);
	color.a *= t_color.a;
#endif
    return color;
}