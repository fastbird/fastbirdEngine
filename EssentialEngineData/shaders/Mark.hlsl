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

#include "EssentialEngineData/shaders/Constants.h"

struct a2v
{
	float3 Position : POSITION;
	float2 UV : TEXCOORD0;
};

struct v2p
{
	float4 Position : SV_Position;
	float2 UV : TEXCOORD0;
};

Texture2D  gDiffuseTexture : register(t0);

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
v2p mark_VertexShader(in a2v INPUT)
{
	v2p OUTPUT;
	float3 center = {gWorld[0][3], gWorld[1][3], gWorld[2][3]};
	float4x4 vp = mul(gProj,gView);
	float3 camUp = {gCamTransform[0][2], gCamTransform[1][2], gCamTransform[2][2]};
	float radius = gMaterialParam[1].x;
	float scale = gMaterialParam[1].y;
	
	// up
	float3 barPos = center + camUp * radius;
	float4 ndcBarPos = mul(vp, float4(barPos, 1.0f));
	ndcBarPos.xy = ndcBarPos.xy / ndcBarPos.w;
	ndcBarPos.y += 34 / gScreenSize.y;	
	
	OUTPUT.Position = float4(INPUT.Position.xy, 0, 1.0);
	OUTPUT.Position.xy *= scale;
	OUTPUT.Position.xy += ndcBarPos.xy;
	OUTPUT.UV = INPUT.UV;

	return OUTPUT;
}

//---------------------------------------------------------------------------
// PIXEL SHADER
//---------------------------------------------------------------------------

float4 mark_PixelShader(in v2p INPUT) : SV_Target
{
#if DIFFUSE_TEXTURE
	return gDiffuseTexture.Sample(gLinearSampler, 
		gMaterialParam[0].xy + INPUT.UV*gMaterialParam[0].zw);
#else
	float3 color = gDiffuseColor;
	float alpha = gDiffuseColor.w;
	float3 highlightedColor = color+gEmissiveColor;
	float2 nuv;
	nuv.x	= abs(INPUT.UV.x * 2.0 - 1.0);
	nuv.y	= abs(INPUT.UV.y * 2.0 - 0.6);
	float2 glowss;
	glowss.x = smoothstep(0.6, 1.0, nuv.x);
	glowss.y = smoothstep(0.2, 0.6, nuv.y);
	float3 finalColor = lerp(color, highlightedColor, sin(gTime*4.0)*.5+.5);
	
	return float4(finalColor, alpha);
#endif	
}