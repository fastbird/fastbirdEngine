//--------------------------------------------------------------------------------------
// File: CommandModule.hlsl
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "Constants.h"
#include "CommonFunctions.h"
#include "CommonLighting.h"

Texture2D  gDiffuseTexture : register(t0);

Texture2D  gNormalTexture : register(t1);

Texture2D  gMetallicTexture : register(t2);

Texture2D  gRoughnessTexture : register(t3);

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
struct a2v
{
	float4 Position : POSITION;
#ifndef SHADOW_PASS	
	float3 Normal : NORMAL;
	float2 UV		: TEXCOORD;
	float3 Tangent : TANGENT;
#endif
};

struct v2p 
{
	float4 Position   : SV_Position;
#ifndef SHADOW_PASS	
	float3 Normal : TEXCOORD0;
	float3 Tangent : TEXCOORD1;
	float3 Binormal : TEXCOORD2;
	float2 UV		: TEXCOORD3;
	float2 OriginalUV : TEXCOORD4;
	float3 LocalPos : TEXCOORD5;
	float3 WorldPos : TEXCOORD6;
	float4 LightPos : TEXCOORD7;
#endif
};

v2p module_VertexShader( in a2v INPUT )
{
    v2p OUTPUT;

	OUTPUT.Position = mul( gWorldViewProj, INPUT.Position );
#ifndef SHADOW_PASS	
	OUTPUT.Normal = normalize(mul((float3x3)gWorld, INPUT.Normal));

#ifdef TEXTURE_ATLAS
	float2 uvOffset = gMaterialParam[0].xy;
	float uvStep = gMaterialParam[0].z;
	OUTPUT.UV = uvOffset + INPUT.UV * uvStep;
#else
	OUTPUT.UV = INPUT.UV;
#endif
	OUTPUT.OriginalUV = INPUT.UV;
	OUTPUT.Tangent = normalize(mul((float3x3)gWorld, INPUT.Tangent));
	OUTPUT.Binormal= normalize(cross(OUTPUT.Tangent, OUTPUT.Normal));
	OUTPUT.LocalPos = INPUT.Position;
	float3 worldPos = mul(gWorld, INPUT.Position);
	OUTPUT.WorldPos = worldPos;
	OUTPUT.LightPos = mul(gLightViewProj, float4(worldPos, 1.0));
#endif
	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 module_PixelShader( in v2p INPUT ) : SV_Target
{
#ifndef SHADOW_PASS
	// process normal map.
	float3 diffuseT = srgb_to_linear(gDiffuseTexture.Sample(gLinearSampler, INPUT.UV).xyz);
	float3 normalT = gNormalTexture.Sample(gPointSampler, INPUT.UV).xyz;
	float metallicT = gMetallicTexture.Sample(gLinearSampler, INPUT.UV).x;
	float roughnessT = gRoughnessTexture.Sample(gLinearSampler, INPUT.UV).x;
	float3 normal = INPUT.Normal;
	if (length(normalT)>0.0001)
	{
		normalT = fixNormalSample(normalT);
		float3 normalMapWS = normalize(normalT.x*INPUT.Tangent + normalT.y*INPUT.Binormal + normalT.z*INPUT.Normal);
		normal = normalMapWS;
	}

	// shading	
	float3 lightColor = gDirectionalLightDiffuse[0].xyz * gDirectionalLightDir_Intensity[0].w;
	float3 lightColor2 = gDirectionalLightDiffuse[1].xyz * gDirectionalLightDir_Intensity[1].w;
	float3 toLightDir = gDirectionalLightDir_Intensity[0].xyz;
	float3 toLightDir2 = gDirectionalLightDir_Intensity[0].xyz;
	float3 camPos = { gCamTransform[0][3], gCamTransform[1][3], gCamTransform[2][3] };
	float3 toViewDir = normalize(camPos - INPUT.WorldPos);

	float dotNL = dot(normal, toLightDir);
	float ndl = max(dotNL, 0);
	float ndl2 = max(-dotNL, 0);
	float3 h = normalize(toViewDir + toLightDir);
	float ndh = max(dot(normal, h), 0);
	float vdh = max(dot(toViewDir, h), 0);
	float ndv = max(dot(normal, toViewDir), 0);
	const float3 dielectricColor = float3(0.04, 0.04, 0.04);
	const float minRoughness = 1e-4;
	float roughness = max(minRoughness, roughnessT);
	float3 diffColor = diffuseT * (1.0 - metallicT);
	float3 specColor = lerp(dielectricColor, diffuseT, metallicT);
	float3 specColor2 = specColor * gDirectionalLightSpecular[1];
	specColor *= gDirectionalLightSpecular[0];

	float invshadow = GetShadow(INPUT.LightPos);
	float3 shadedColor = ndl * lightColor * (diffColor + CookTorrance(ndl, vdh, ndh, ndv, specColor, roughness));
	shadedColor += ndl2 * lightColor2 * diffColor;
	//float3 irrad = GetIrrad(float4(normal, 1.0f));
	//shadedColor += irrad;

	float3 envContrib = { 0, 0, 0 };
	// need to work further.

#ifdef ENV_TEXTURE	
	envContrib = CalcEnvContrib(normal, INPUT.Tangent, INPUT.Binormal, roughness, toViewDir, ndv, diffColor, specColor);
#endif // end of ENV_TEXTURE

	shadedColor += envContrib;

	float3 finalColor = shadedColor * invshadow;
	float2 screenUV = GetScreenUV(INPUT.Position.xy);
	float3 foggedColor = GetFoggedColor(screenUV, finalColor, normalize(INPUT.WorldPos));

#ifdef DAMAGE_REPORT
	foggedColor += 0.1;
	foggedColor *= lerp(float3(1.5, 0.1, 0.1), float3(0.0, 0.8, 0.0), INPUT.Etc.y);
#endif //end of DAMAGE_REPORT

#ifdef _CONSTRUCTING_MODULE
	// gMaterialParam[1] : z min, max height, alpha, x
	float height = smoothstep(gMaterialParam[1].x, gMaterialParam[1].y, INPUT.LocalPos.z);
	float alpha;
	if (height < gMaterialParam[1].z)
		alpha = 1.0f;
	else
		alpha = 0.3f;
#else
	float alpha = 1.0f;
#endif
		
#ifdef _DRAW_EDGE
	float2 uv2 = abs(INPUT.OriginalUV.xy*2.0-1.0);
	float3 edgeLitColor = gMaterialParam[2].xyz;
	float2 glowss = smoothstep(0.9, 1.0, uv2);
	float glowAlpha =  min(1, glowss.x + glowss.y);
	foggedColor = lerp(foggedColor, edgeLitColor,glowAlpha);
	return float4(foggedColor, max(alpha, glowAlpha));    // Yellow, with Alpha = 1
#else
	return float4(foggedColor, alpha);    // Yellow, with Alpha = 1
#endif
#else // SHADOW_PASS
	return float4(0, 0, 0, 0);
#endif	 // endof !SHADOW_PASS
}