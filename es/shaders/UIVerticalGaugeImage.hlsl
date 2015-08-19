//--------------------------------------------------------------------------------------
// File: UIVerticalGaugeImage.hlsl
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "Constants.h"

Texture2D  gImageFilled : register(t0);
Texture2D  gImageNotFilled : register(t1);
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
struct a2v
{
	float3 Position : POSITION;
	float4 Color	: COLOR;
	float2 uvFilled		: TEXCOORD0;
	float2 uvNotFilled 	: TEXCOORD1;
};

struct v2p 
{
	float4 Position   : SV_Position;
	float2 uvFilled		: TEXCOORD0;
	float2 uvNotFilled		: TEXCOORD1;
};

v2p uiverticalgaugeimage_VertexShader( in a2v INPUT )
{
    v2p OUTPUT;
	OUTPUT.Position = mul(gWorld, float4(INPUT.Position, 1));
	OUTPUT.uvFilled = INPUT.uvFilled;
	OUTPUT.uvNotFilled = INPUT.uvNotFilled;

	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 uiverticalgaugeimage_PixelShader( in v2p INPUT ) : SV_Target
{	
	float minY = gMaterialParam[2].x;
	float maxY = gMaterialParam[2].y;
	// 0~1 range.
	float curY = 1 - (INPUT.uvFilled.y - minY) / (maxY - minY);
	float percent = gMaterialParam[1].x / gMaterialParam[1].y;
	
	if (curY > percent)
	{	
		return gImageNotFilled.Sample(gLinearSampler, INPUT.uvNotFilled);
	}
	else
	{
		return gImageFilled.Sample(gLinearSampler, INPUT.uvFilled);
	}
}