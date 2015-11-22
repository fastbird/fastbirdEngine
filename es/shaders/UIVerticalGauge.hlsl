//--------------------------------------------------------------------------------------
// File: UIVerticalGauge.hlsl
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "Constants.h"

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

v2p uiverticalgauge_VertexShader( in a2v INPUT )
{
    v2p OUTPUT;

	OUTPUT.Position = INPUT.Position;
	OUTPUT.UV = INPUT.UV;

	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 uiverticalgauge_PixelShader( in v2p INPUT ) : SV_Target
{	
	// signed nc
	float2 snc = INPUT.UV * 2.0 - 1.0f;
	snc.y = -snc.y;
	float edge = max(abs(snc.x), abs(snc.y));
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
	
	float percent = gMaterialParam[1].x * 2.0 - 1.0;
	
	if (snc.y < percent)
	{
		return lerp(gMaterialParam[2], gMaterialParam[3], gMaterialParam[4].x);
	}
	
	float maximum = gMaterialParam[1].y * 2.0 - 1.0;
	if (abs(snc.y-maximum)<0.01)
	{
		return float4(1, 1, 0, 1);
	}	

    return float4(0.1, 0.1, 0.1, 1);
}