//----------------------------------------------------------------------------
// File: skySphere.hlsl
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Constant Buffer
//----------------------------------------------------------------------------
#include "Constants.h"
#include "CommonFunctions.hlsl"

//----------------------------------------------------------------------------
// Textures
//----------------------------------------------------------------------------
//Texture2D  gEnvMap : register(t0);
//SamplerState gEnvSampler : register(s0);

//----------------------------------------------------------------------------
struct a2v
{
	float4 Position		: POSITION;
	float2 UV			: TEXCOORD;
};

//----------------------------------------------------------------------------
struct v2p
{
	float4 Position		: SV_Position;
	float3 WorldDir		: TEXCOORD0;
};

//----------------------------------------------------------------------------
// VertexShader
//----------------------------------------------------------------------------
v2p skysphere_VertexShader(in a2v INPUT)
{
	v2p OUTPUT;
	float4 pos = INPUT.Position;
	pos.z = 0.9999;
	OUTPUT.Position = pos;
	float4 worldPos = mul(gInvViewProj, pos);
	worldPos.xyz /= worldPos.w;
	
	OUTPUT.WorldDir = worldPos.xyz - gCameraPos.xyz;

	return OUTPUT;
}
/*
float2 SphericalCoord2(float3 dir)
{
	float n = length(dir.xy);
	float x_xyPlane = (n > 0.000001) ? dir.x / n : 0;
	float2 uv;
	uv.x = acos(x_xyPlane)*INV_PI*.5;
	uv.x = (dir.y > 0.0) ? uv.x : 1.0-uv.x;
	uv.y = 1.0 - (atan(dir.z / n) * INV_PI+ 0.5);
	//uv.y = 1.0f - (dir.z*.5 + .5);
	
	return uv;
}*/
//----------------------------------------------------------------------------
// PIXEL shader
//----------------------------------------------------------------------------
float4 skysphere_PixelShader(in v2p INPUT) : SV_Target
{
	float3 dir =normalize(INPUT.WorldDir);
	float3 starColor = Star(dir, gCameraPos);
	return float4(starColor, 1);
	//float2 uv = SphericalCoord2(dir);
	//return gEnvMap.Sample(gEnvSampler, uv);	
}