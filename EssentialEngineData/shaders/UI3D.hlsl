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
// File: ui.hlsl
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "EssentialEngineData/shaders/Constants.h"
#include "EssentialEngineData/shaders/CommonFunctions.h"
//
Texture2D  gDiffuseTexture : register(t0);
Texture2D  gDepthTexture : register(t5);

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
struct a2v
{
	float3 Position : POSITION;
	float2 UV		: TEXCOORD;
};

struct v2p 
{
	float4 Position   : SV_Position;
	float2 UV		: TEXCOORD0;
	float2 ScreenUVData : TEXCOORD1;
	float3 ViewPos : TEXCOORD2;
};

v2p ui3d_VertexShader( in a2v INPUT )
{
    v2p OUTPUT;

	OUTPUT.Position = mul(gWorldViewProj, float4(INPUT.Position, 1.0));
	OUTPUT.UV = INPUT.UV;
	OUTPUT.ScreenUVData = OUTPUT.Position.xy / OUTPUT.Position.w;
	OUTPUT.ViewPos = mul(gWorldView, float4(INPUT.Position, 1.0));
	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 ui3d_PixelShader( in v2p INPUT ) : SV_Target
{	
	float2 screenTex = 0.5*( (INPUT.ScreenUVData) + float2(1,1));
    screenTex.y = 1 - screenTex.y;
	
	float sceneDepth = gDepthTexture.Sample(gLinearSampler, screenTex);
	float uiDepth = (INPUT.ViewPos.y - gNearFar.x) / (gNearFar.y - gNearFar.x);
	float diff = abs(sceneDepth - uiDepth);
	float depthFade = saturate(diff*2000);
	if (depthFade<0.01f)
		discard;
	
	float4 color = gDiffuseTexture.Sample(gLinearSampler, INPUT.UV) * gDiffuseColor;
	color.a *= depthFade;
	return color;
}