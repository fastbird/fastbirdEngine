/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

//--------------------------------------------------------------------------------------
// File: MeshPBR.hlsl
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "Constants.h"
#include "CommonFunctions.h"
#include "CommonLighting.h"

Texture2D  gDiffuseTexture : register(t0);
#ifdef NORMAL_TEXTURE
Texture2D  gNormalTexture : register(t1);
#endif //NORMAL_TEXTURE

#ifdef MergedMaterialTexture
Texture2D  gMaterialTexture : register(t2);  // r: metallic, g: roughness, b: emissive
#else
Texture2D  gMetallicTexture : register(t2);  
Texture2D	gRoughnessTexture : register(t3);
#endif

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
	float4 WorldPos : TEXCOORD4; // worldPos x, y, z + current pixel depth w
	float4 TexShadow : TEXCOORD5;	
};

v2p meshpbr_VertexShader( in a2v INPUT )
{
    v2p OUTPUT;

	OUTPUT.Position = mul( gWorldViewProj, INPUT.Position );
	OUTPUT.Normal = normalize(mul((float3x3)gWorld, INPUT.Normal));
	OUTPUT.UV = INPUT.UV;
	OUTPUT.Tangent = normalize(mul((float3x3)gWorld, INPUT.Tangent));
	OUTPUT.Binormal= normalize(cross(OUTPUT.Tangent, OUTPUT.Normal));
	float3 worldPos = mul(gWorld, INPUT.Position).xyz;
	OUTPUT.WorldPos.xyz = worldPos;
	OUTPUT.WorldPos.w = mul(gWorldView, Input.Position).y;
	OUTPUT.TexShadow = mul(gLightView, float4(worldPos, 1.0));

	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
#if Glow
struct PS_OUT
{
	float4 color0 : SV_Target0;
	float4 color1 : SV_Target1;
};
PS_OUT meshpbr_PixelShader(in v2p INPUT) : SV_Target
#else
float4 meshpbr_PixelShader( in v2p INPUT ) : SV_Target
#endif
{
	INPUT.UV.y = 1.0 - INPUT.UV.y;
	// process normal map.
	float3 baseColor = gDiffuseTexture.Sample(gLinearSampler, INPUT.UV).xyz;
#if MergedMaterialTexture
	float4 materialData = gMaterialTexture.Sample(gLinearSampler, INPUT.UV);
	float metallic = materialData.x;
	float roughness = materialData.y;
	float emissive = materialData.z;
#else
	float metallic = gMetallicTexture.Sample(gLinearSampler, INPUT.UV).r;	
	float roughness = gRoughnessTexture.Sample(gLinearSampler, INPUT.UV).r;
	float emissive = 0;
#endif
#ifdef NORMAL_TEXTURE
	float3 normalT = gNormalTexture.Sample(gLinearSampler, INPUT.UV).xyz;
#endif //NORMAL_TEXTURE
	
	float3 normal = INPUT.Normal;
#ifdef NORMAL_TEXTURE
	normalT = fixNormalSample(normalT);
	float3 normalMapWS = normalize(normalT.x*INPUT.Tangent + normalT.y*INPUT.Binormal + normalT.z*INPUT.Normal);
	normal = normalMapWS;
#endif //NORMAL_TEXTURE

	// shading	
	float3 lightColor = gDirectionalLightDiffuse[0].xyz * gDirectionalLightDir_Intensity[0].w;
	float3 lightColor2 = gDirectionalLightDiffuse[1].xyz * gDirectionalLightDir_Intensity[1].w;
	float3 toLightDir = gDirectionalLightDir_Intensity[0].xyz;
	float3 camPos = {gCamTransform[0][3], gCamTransform[1][3], gCamTransform[2][3]};
	float3 toViewDir = normalize(camPos - INPUT.WorldPos.xyz);	

	const float3 dielectricColor = float3(0.04, 0.04, 0.04);
	const float minRoughness = 1e-4;
	roughness = max(minRoughness, roughness);
	float3 diffColor = baseColor * (1.0 - metallic);
	float3 specColor = lerp(dielectricColor, baseColor, metallic);	
	specColor *= gDirectionalLightSpecular[0];

	float dotNL = dot(normal, toLightDir);
	float ndl = max(dotNL, 0);
	float ndl2 = max(-dotNL, 0);
	float3 h = normalize(toViewDir + toLightDir);
	float ndh = max( dot(normal, h), 1e-8);
	float vdh = max( dot(toViewDir, h), 1e-8);
	//INPUT.WorldPos.w == current pixel depth
	float invShadow = GetShadow(INPUT.TexShadow, INPUT.WorldPos.w);
	float3 shadedColor = ndl * lightColor * (diffColor + CookTorrance(vdh, ndh, specColor, roughness));
	shadedColor +=  ndl2 * lightColor2 * max(float3(0.2f, 0.2f, 0.2f), diffColor);
	float3 irrad = GetIrrad(float4(normal, 1.0f));
	shadedColor+=irrad;	
	
	float3 envContrib = {0, 0, 0};
	
	// need to work further.
#ifdef ENV_TEXTURE
	envContrib = CalcEnvContrib(normal, INPUT.Tangent, INPUT.Binormal, roughness, toViewDir, diffColor, specColor);
#endif

	float3 pointLightResult = CalcPointLights(INPUT.WorldPos.xyz, normal) * baseColor;
	float2 screenUV = GetScreenUV(INPUT.Position.xy);
	float3 foggedColor = GetFoggedColor(screenUV, shadedColor + envContrib + pointLightResult, normalize(INPUT.WorldPos));
#if Glow
	PS_OUT psout;
	psout.color0 = float4(foggedColor * invShadow + baseColor * emissive + gAmbientColor.rgb, gDiffuseColor.a);
	psout.color1 = psout.color0 * emissive;
	return psout;
#else
	return float4(foggedColor * invShadow + baseColor * emissive + gAmbientColor.rgb, gDiffuseColor.a);
#endif
}