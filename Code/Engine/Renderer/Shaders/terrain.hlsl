//--------------------------------------------------------------------------------------
// File: terrain.hlsl
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "Constants.h"

Texture2D  gDiffuseTexture : register(t0);
Texture2D  gPatternTexture : register(t1);
Texture2D  gRampTexture : register(t2);

// dirtColor = gMaterialParam[0].rgb;
// grassColor = gMaterialParam[1].rgb;
// rockColor = gMaterialParam[2].rgb;
// specularIntensity = gMaterialParam[0].w

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
struct a2v
{
	float4 Pos : POSITION;
	float3 Normal : NORMAL;
	//float3 Tangent : TANGENT;
	//float3 Binormal : BINORMAL;
};

struct v2p 
{
	float4 Position   : SV_Position;
	float3 ViewDir : TEXCOORD0;
	float3 Normal : TEXCOORD1;
	float3 WorldPos : TEXCOORD2;
};

v2p terrain_VertexShader( in a2v INPUT )
{
    v2p OUTPUT;

	OUTPUT.Position = mul( gViewProj, INPUT.Pos );
	OUTPUT.WorldPos = INPUT.Pos.xyz;
	OUTPUT.ViewDir = normalize(gCameraPos -  INPUT.Pos.xyz);
	OUTPUT.Normal = INPUT.Normal;
	
	/*float3x3 mToTangent;
	mToTangent[0] = INPUT.Binormal;
	mToTangent[1] = INPUT.Tangent;
	mToTangent[2] = INPUT.Normal;
	float3 lightDir = gDirectionalLightDir_Intensity.xyz;
	OUTPUT.LightDir = mul( mToTangent, lightDir );
	float3 viewDir = normalize(gCameraPos.xyz -  INPUT.Pos);
	OUTPUT.ViewDir = mul( mToTangent, viewDir );*/

	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 terrain_PixelShader( in v2p INPUT ) : SV_Target
{	
	const float3 dirtColor = gMaterialParam[0].rgb;
	const float3 grassColor = gMaterialParam[1].rgb;
	const float3 rockColor = gMaterialParam[2].rgb;
	
	const float specularIntensity = gMaterialParam[0].w;
	const float specularPower = gMaterialParam[1].w;

	// Sample
	float2 uv = INPUT.WorldPos.xy;
	uv.y = 1.f - uv.y;
	float2 categoryUV = uv * 0.0009765625f;
	float2 detailUV = uv * 0.04f;
	float texCategory = gDiffuseTexture.Sample(gLinearSampler, categoryUV);
	float3 weights = gRampTexture.Sample(gLinearSampler, float2(texCategory, .5f)).xyz;
	float4 patterns = gPatternTexture.Sample(gLinearSampler, detailUV);
	float3 finalDirt = patterns.x * weights.x * dirtColor;
	float3 finalGrass = patterns.y * weights.y * grassColor;
	float3 finalRock = patterns.z * weights.z * rockColor;
	float3 finalDiffuseTex = finalDirt + finalGrass + finalRock;
	
	const float weightedSpecularIntensity = specularIntensity * (1.f + weights.z*4.0f);
	
	// Diffuse Light
	float intensity = gDirectionalLightDir_Intensity.w;
	float3 lightDir = gDirectionalLightDir_Intensity.xyz;
	float3 normal = normalize(INPUT.Normal);
	float3 viewDir = normalize(INPUT.ViewDir);
	float normalDotLight = dot(normal, lightDir);
	float3 diffuse = intensity * 
		max(0, normalDotLight) * gDirectionalLightDiffuse.xyz;
	// Specular Light
	//float3 r = 2.f * normalDotLight * normal - INPUT.LightDir;
	float3 r = reflect(-lightDir, normal);
	float3 specular = weightedSpecularIntensity * intensity* pow(saturate(dot(viewDir, r)), specularPower) * gDirectionalLightSpecular.xyz;
	float3 color = diffuse * finalDiffuseTex + specular;
    return float4( color, 1.0f );    // Yellow, with Alpha = 1
}