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
// File: terrain.hlsl
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "EssentialEngineData/shaders/Constants.h"

Texture2D  gDiffuseTexture : register(t0);
Texture2D  gPatternTexture : register(t1);
Texture2D  gRampTexture : register(t2);

// dirtColor = gShaderParams[0].rgb;
// grassColor = gShaderParams[1].rgb;
// rockColor = gShaderParams[2].rgb;
// specularIntensity = gShaderParams[0].w

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

v2p Terrain_VertexShader( in a2v INPUT )
{
    v2p OUTPUT;
	float3 camPos = {gCamTransform[0][3], gCamTransform[1][3], gCamTransform[2][3]};
	OUTPUT.Position = mul( gViewProj, INPUT.Pos );
	OUTPUT.WorldPos = INPUT.Pos.xyz;
	OUTPUT.ViewDir = normalize(camPos -  INPUT.Pos.xyz);
	OUTPUT.Normal = INPUT.Normal;
	
	/*float3x3 mToTangent;
	mToTangent[0] = INPUT.Binormal;
	mToTangent[1] = INPUT.Tangent;
	mToTangent[2] = INPUT.Normal;
	float3 lightDir = gDirectionalLightDir_Intensity[0].xyz;
	OUTPUT.LightDir = mul( mToTangent, lightDir );
	float3 viewDir = normalize(camPos.xyz -  INPUT.Pos);
	OUTPUT.ViewDir = mul( mToTangent, viewDir );*/

	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 Terrain_PixelShader( in v2p INPUT ) : SV_Target
{	
	const float3 dirtColor = gShaderParams[0].rgb;
	const float3 grassColor = gShaderParams[1].rgb;
	const float3 rockColor = gShaderParams[2].rgb;
	
	const float specularIntensity = gShaderParams[0].w;
	const float specularPower = gShaderParams[1].w;

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
	float intensity = gDirectionalLightDir_Intensity[0].w;
	float3 lightDir = gDirectionalLightDir_Intensity[0].xyz;
	float3 normal = normalize(INPUT.Normal);
	float3 viewDir = normalize(INPUT.ViewDir);
	float normalDotLight = dot(normal, lightDir);
	float3 diffuse = intensity * 
		max(0, normalDotLight) * gDirectionalLightDiffuse[0].xyz;
	// Specular Light
	//float3 r = 2.f * normalDotLight * normal - INPUT.LightDir;
	float3 r = reflect(-lightDir, normal);
	float3 specular = weightedSpecularIntensity * intensity* pow(saturate(dot(viewDir, r)), specularPower) * gDirectionalLightSpecular.xyz;
	float3 color = diffuse * finalDiffuseTex + specular;
    return float4( color, 1.0f );    // Yellow, with Alpha = 1
}