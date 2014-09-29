//--------------------------------------------------------------------------------------
// File: Mesh.hlsl
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "Constants.h"

Texture2D  gDiffuseTexture : register(t0);

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
struct a2v
{
	float4 Position : POSITION;
	float3 Normal : NORMAL;
#ifdef DIFFUSE_TEXTURE
	float2 UV		: TEXCOORD;
#endif
};

struct v2p 
{
	float4 Position   : SV_Position;
	float3 Normal : TEXCOORD0;
#ifdef DIFFUSE_TEXTURE
	float2 UV		: TEXCOORD1;
#endif
};

v2p mesh_VertexShader( in a2v INPUT )
{
    v2p OUTPUT;

	//float4 worldPos = mul( gWorld, INPUT.Position);
	OUTPUT.Position = mul( gWorldViewProj, INPUT.Position );
	OUTPUT.Normal = mul((float3x3)gWorld, INPUT.Normal);
#ifdef DIFFUSE_TEXTURE
	OUTPUT.UV = INPUT.UV;
#endif

	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 mesh_PixelShader( in v2p INPUT ) : SV_Target
{	
	// Diffuse Light
	float intensity = gDirectionalLightDir_Intensity.w;
	float3 lightDir = gDirectionalLightDir_Intensity.xyz;
	float normalDotLight = dot(INPUT.Normal, lightDir);
	float3 diffuse = (intensity * 
		max(0, normalDotLight) * gDirectionalLightDiffuse.xyz);
	diffuse.xyz += gAmbientColor.xyz;

#ifdef DIFFUSE_TEXTURE
	diffuse *= gDiffuseTexture.Sample(gLinearSampler, INPUT.UV).xyz;
#endif
	
	// Specular Light
    return float4( diffuse, 1.0f );    // Yellow, with Alpha = 1
}