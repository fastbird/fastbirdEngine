//--------------------------------------------------------------------------------------
// File: UIVerticalGauge.hlsl
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "Constants.h"

#if defined(_ALPHA_TEXTURE)
Texture2D gAlphaTexture : register(t1);
#endif

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
struct a2v
{
	float4 Position : POSITION;
	float4 Color	: COLOR;
	float2 UV		: TEXCOORD;
};

struct v2p 
{
	float4 Position   : SV_Position;
	float2 UV		: TEXCOORD;
};

v2p uihorizontalgauge_VertexShader( in a2v INPUT )
{
    v2p OUTPUT;
	OUTPUT.Position = INPUT.Position;
	OUTPUT.UV = INPUT.UV;

	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 uihorizontalgauge_PixelShader( in v2p INPUT ) : SV_Target
{	
#if defined(_ALPHA_TEXTURE)
	float a = gAlphaTexture.Sample(gPointSampler, INPUT.UV).a;
#else
	float a = 1.0;
#endif
	float ratio = gMaterialParam[0].x;
	
	// signed nc
	float2 snc = INPUT.UV * 2.0 - 1.0f;
	snc.y = -snc.y;
	
	float edge = max(abs(snc.x), abs(snc.y));
	float xPos = abs(snc.x);
	float yPos = abs(snc.y);
	if ( xPos >= 1.0 - 0.03  / ratio || yPos >= 0.93)
	{
		float4 color = float4(1.0, 1.0, 1.0, 1.0) * gAmbientColor;
		color.a *= a;
		return color;
	}
	else if (xPos >= 1.0- 0.06 / ratio || yPos >= 0.88)
	{
		float4 color = float4(0.87058, 0.87058, 0.847058, 1.0) * gAmbientColor;
		color.a *= a;
		return color;
	}
	
	else if (xPos >= 1.0- 0.09 / ratio || yPos >= 0.80)
	{
		float4 color = float4(0.521568, 0.521568, 0.505882, 1.0) *  gAmbientColor;
		color.a *= a;
		return color;
	}
	
	// param 4
	// x : percent
	// y : maximum
	float percent = gMaterialParam[4].x * 2.0 - 1.0;
	if (snc.x < percent)
	{
		// 1: gauge color
		// 2: Blink color
		// 3x: sin
		float4 color = lerp(gMaterialParam[1], gMaterialParam[2], gMaterialParam[3].x);
		color.a *= a;
		return color;
	}
	
	float4 color = lerp(gDiffuseColor, gMaterialParam[2], gMaterialParam[3].x);
	color.a *= a;
    return color;
}