//--------------------------------------------------------------------------------------
// File: Mesh.hlsl
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
	float3 WorldPos : TEXCOORD1;
};

v2p helpmesh_VertexShader( in a2v INPUT )
{
    v2p OUTPUT;

	OUTPUT.Position = mul( gWorldViewProj, INPUT.Position );
	OUTPUT.Normal = mul((float3x3)gWorld, INPUT.Normal);
	OUTPUT.WorldPos = mul(gWorld, INPUT.Position);
	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 helpmesh_PixelShader( in v2p INPUT ) : SV_Target
{	
	// Diffuse Light
	/*
	float intensity = gDirectionalLightDir_Intensity[0].w;
	float3 lightDir = gDirectionalLightDir_Intensity[0].xyz;
	float normalDotLight = dot(INPUT.Normal, lightDir);
	float3 diffuse = (intensity * 
		max(0, normalDotLight) * gDirectionalLightDiffuse[0].xyz);
	diffuse.xyz += gAmbientColor.xyz;
	*/

	float intensity = gDirectionalLightDir_Intensity[0].w;
	float3 lightDir = gDirectionalLightDir_Intensity[0].xyz;
	float normalDotLight = dot(INPUT.Normal, lightDir);
	float3 diffuse;
	float3 toCam = normalize(gCameraPos - INPUT.WorldPos);
	if (dot(INPUT.Normal, toCam)<0.35f)
	{
		diffuse.xy = (sin(gTime*6.0f)+1.0f)*.25f + 0.5f;
		diffuse.z = 0.f;		
	}
	else
	{
		diffuse.xyz = float3(1, 0.8, 0.0);
	}
	
	// Specular Light
    return float4( diffuse, 1.0f );    // Yellow, with Alpha = 1
}