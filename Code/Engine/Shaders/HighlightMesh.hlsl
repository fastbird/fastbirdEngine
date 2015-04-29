//--------------------------------------------------------------------------------------
// File: HighlightMesh.hlsl
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
	float3 Normal : NORMAL;
};

struct v2p 
{
	float4 Position   : SV_Position;
	float3 Normal : TEXCOORD0;
};

v2p highlightmesh_VertexShader( in a2v INPUT )
{
    v2p OUTPUT;
	float3 scaledPos = INPUT.Position.xyz * 1.005f;
	OUTPUT.Position = mul( gWorldViewProj, float4(scaledPos, 1) );
	OUTPUT.Normal = mul((float3x3)gWorld, INPUT.Normal);

	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 highlightmesh_PixelShader( in v2p INPUT ) : SV_Target
{	
    return float4( 1.0f, 1.0f, 0.0f, 1.0f );    // Yellow, with Alpha = 1
}