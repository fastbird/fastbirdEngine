//--------------------------------------------------------------------------------------
// File: UIVerticalGauge.hlsl
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
	float2 UV		: TEXCOORD;
};

struct v2p 
{
	float4 Position   : SV_Position;
	float2 UV		: TEXCOORD;
};

v2p uiverticalgauge_VertexShader( in a2v INPUT )
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
float4 uiverticalgauge_PixelShader( in v2p INPUT ) : SV_Target
{	
	float2 ndc = INPUT.UV * 2.0 - 1.0f;
	ndc.y = -ndc.y;
	float edge = max(abs(ndc.x), abs(ndc.y));
	if (edge > 0.98)
	{
		return float4(0.6274, 0.6274, 0.6117, 1.0);
	}
	else if ( edge > 0.96)
	{
		return float4(0.87058, 0.87058, 0.847058, 1.0);
	}
	else if ( edge > 0.94 )
	{
		return float4(0.521568, 0.521568, 0.505882, 1.0);
	}
	
	float percent = gMaterialParam[0].x * 2.0 - 1.0;
	
	if (ndc.y < percent)
	{
		return lerp(gMaterialParam[1], gMaterialParam[2], gMaterialParam[3].x);
	}
	
	float maximum = gMaterialParam[0].y * 2.0 - 1.0;
	if (abs(ndc.y-maximum)<0.01)
	{
		return float4(1, 1, 0, 1);
	}	

    return float4(0.1, 0.1, 0.1, 1);
}