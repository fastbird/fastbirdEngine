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
// File: Mesh.hlsl
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "EssentialEngineData/shaders/Constants.h"

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

v2p Mesh_VertexShader( in a2v INPUT )
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
float4 Mesh_PixelShader( in v2p INPUT ) : SV_Target
{	
	// Diffuse Light
	float3 diffuse = 0;
	for (int i=0; i<2; i++)
	{
		float intensity = gDirectionalLightDir_Intensity[i].w;
		float3 lightDir = gDirectionalLightDir_Intensity[i].xyz;
		float normalDotLight = dot(INPUT.Normal, lightDir);
		diffuse += (intensity * 
		max(0, normalDotLight) * gDirectionalLightDiffuse[i].xyz);
	}
	diffuse.xyz += gAmbientColor.xyz;

#ifdef DIFFUSE_TEXTURE
	diffuse *= gDiffuseTexture.Sample(gLinearSampler, INPUT.UV).xyz;
#endif	
	// Specular Light
    return float4( diffuse, 1.0f );    // Yellow, with Alpha = 1
}