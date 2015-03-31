//--------------------------------------------------------------------------------------
// File: DebugDraw.hlsl
//--------------------------------------------------------------------------------------
// render as point sprite with geometry

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "Constants.h"

struct a2v
{
	float3 Position : POSITION;
};

struct v2p
{
	float4 Position : SV_Position;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
v2p debugdraw_VertexShader(in a2v INPUT)
{
	v2p OUTPUT;
	OUTPUT.Position = mul(gWorldViewProj, float4(INPUT.Position, 1));
	return OUTPUT;
}
//---------------------------------------------------------------------------
// PIXEL SHADER
//---------------------------------------------------------------------------

// struct PS_OUT
// {
	// float4 color0 : SV_Target0;
	// float4 color1 : SV_Target1;
// };

float4 debugdraw_PixelShader(in v2p INPUT): SV_Target
{
	return gMaterialParam[0];
}