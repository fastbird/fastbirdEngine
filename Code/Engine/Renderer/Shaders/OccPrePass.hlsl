//--------------------------------------------------------------------------------------
// File: OccPrePass.hlsl
//--------------------------------------------------------------------------------------
#include "Constants.h"
struct a2v
{
	float3 Position : POSITION;
};

struct v2p
{
	float4 Position   : SV_Position;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
v2p occprepass_VertexShader(in a2v INPUT)
{
	v2p OUTPUT;

	OUTPUT.Position = mul(gWorldViewProj, float4(INPUT.Position, 1.0));

	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 occprepass_PixelShader(in v2p INPUT) : SV_Target
{
	return float4(0, 0, 0, 1.0);
}