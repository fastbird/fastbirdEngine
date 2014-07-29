//--------------------------------------------------------------------------------------
// File: CommandModule.hlsl
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "Constants.h"
#include "CommonFunctions.hlsl"

Texture2D  gDiffuseTexture : register(t0);
SamplerState gDiffuseSampler : register(s0);

Texture2D  gNormalTexture : register(t1);
SamplerState gNormalSampler : register(s1);

Texture2D  gSpecularTexture : register(t2);
SamplerState gSpecularSampler : register(s2);

Texture2D  gAOTexture : register(t3);
SamplerState gAOSampler : register(s3);

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
struct a2v
{
	float4 Position : POSITION;
	float3 Normal : NORMAL;
	float2 UV		: TEXCOORD;
	float3 Tangent : TANGENT;
};

struct v2p 
{
	float4 Position   : SV_Position;
	float3 Normal : TEXCOORD0;
	float3 Tangent : TEXCOORD1;
	float3 Binormal : TEXCOORD2;
	float2 UV		: TEXCOORD3;
	float3 LocalPos : TEXCOORD4;
	float3 WorldPos : TEXCOORD5;
};

v2p module_VertexShader( in a2v INPUT )
{
    v2p OUTPUT;

	OUTPUT.Position = mul( gWorldViewProj, INPUT.Position );
	OUTPUT.Normal = mul((float3x3)gWorld, INPUT.Normal);
#ifdef TEXTURE_ATLAS
	float2 uvOffset = gMaterialParam[0].xy;
	float uvStep = gMaterialParam[0].z;
	OUTPUT.UV = uvOffset + INPUT.UV * uvStep;
#else
	OUTPUT.UV = INPUT.UV;
#endif
	OUTPUT.Tangent = mul((float3x3)gWorld, INPUT.Tangent);
	OUTPUT.Binormal= cross(OUTPUT.Tangent, OUTPUT.Normal);
	OUTPUT.LocalPos = INPUT.Position;
	OUTPUT.WorldPos = mul(gWorld, INPUT.Position);

	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 module_PixelShader( in v2p INPUT ) : SV_Target
{	
	// process normal map.
	float3 diffuseT = gDiffuseTexture.Sample(gDiffuseSampler, INPUT.UV).xyz;
	float3 normalT = gNormalTexture.Sample(gNormalSampler, INPUT.UV).xyz;
	float3 specularT = gSpecularTexture.Sample(gSpecularSampler, INPUT.UV).xyz;
	float aoT = gAOTexture.Sample(gAOSampler, INPUT.UV).xyz;
	
	float3 normal = INPUT.Normal;
	if (length(normalT)>0.0001)
	{
		normalT = fixNormalSample(normalT);
	    float3 normalMapWS = normalT.x*INPUT.Tangent + normalT.y*INPUT.Binormal;
		normal = normalize(normal + normalMapWS);
	}  

	// shading	
	float3 lightColor = gDirectionalLightDiffuse.xyz * gDirectionalLightDir_Intensity.w;
	float3 toLightDir = gDirectionalLightDir_Intensity.xyz;
	float3 toCamDir = normalize(gCameraPos - INPUT.WorldPos);
	float3 diffuse, specular;
	BlinnPhongShading(lightColor, normal, toLightDir, toCamDir, specularT, diffuse, specular);		
	float3 shadedColor = gAmbientColor.xyz*aoT + diffuse * diffuseT + specular;
	
	// edge emission and glow
#ifdef TEXTURE_ATLAS
	float2 uvOffset = gMaterialParam[0].xy;
	float uvStep = gMaterialParam[0].z;
	float2 uv2 = (INPUT.UV - uvOffset) / uvStep;
	uv2 = abs(uv2 * 2.0 - 1.0);
#else
	float2 uv2 = abs(INPUT.UV.xy*2.0-1.0);
#endif
	
	float3 edgeLitColor = gMaterialParam[1].xyz;
	float3 edgeGlow = edgeLitColor * 0.3;
	float3 localPos = INPUT.LocalPos;
	float edgeLitPower = (sin(gTime+localPos.x+localPos.y) + sin(gTime*2.0+localPos.y+localPos.z))*.5f
		* .5f + .5f;
	edgeLitPower = max(0.01, edgeLitPower);
	float2 glowss = smoothstep(0.7, 0.97, uv2);
	float glowIntencity = min(1, max(glowss.x,glowss.y) * 1.0f);
	float3 glow = edgeGlow * glowIntencity * edgeLitPower;	
	shadedColor += glow*aoT;
	
	float2 ss = smoothstep(0.97, 1.0, uv2);
	float3 finalColor = lerp( shadedColor, edgeLitColor * edgeLitPower, ss.x + ss.y);
	
    return float4( finalColor, 1.0f );    // Yellow, with Alpha = 1
}