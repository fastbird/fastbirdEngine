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
// File: UIVerticalGauge.hlsl
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "EssentialEngineData/shaders/Constants.h"

#if defined(_ALPHA_TEXTURE)
Texture2D gAlphaTexture : register(t1);
#endif

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
struct a2v
{
	float4 Position : POSITION;
	float4 Color	: COLOR;
	float2 UV		: TEXCOORD;
};

struct v2p 
{
	float4 Position   : SV_Position;
	float2 UV		: TEXCOORD;
};

v2p UIHorizontalGauge_VertexShader( in a2v INPUT )
{
    v2p OUTPUT;
	OUTPUT.Position = INPUT.Position;
	OUTPUT.UV = INPUT.UV;

	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 UIHorizontalGauge_PixelShader( in v2p INPUT ) : SV_Target
{	
#if defined(_ALPHA_TEXTURE)
	float a = gAlphaTexture.Sample(gPointSampler, INPUT.UV).a;
#else
	float a = 1.0;
#endif	
	float ratio = gShaderParams[0].x;
	
	// signed nc
	float2 snc = INPUT.UV * 2.0 - 1.0f;
	snc.y = -snc.y;
	
	float edge = max(abs(snc.x), abs(snc.y));
	float xPos = abs(snc.x);
	float yPos = abs(snc.y);
	if ( xPos >= 1.0 - 0.03  / ratio || yPos >= 0.93)
	{
		float4 color = float4(1.0, 1.0, 1.0, 1.0) * gAmbientColor;
		color.rgb *= 1.f - yPos*.5f;
		color.a *= a;
		return color;
	}
	else if (xPos >= 1.0- 0.06 / ratio || yPos >= 0.88)
	{
		float4 color = float4(0.87058, 0.87058, 0.847058, 1.0) * gAmbientColor;
		color.rgb *= 1.f - yPos*.5f;
		color.a *= a;
		return color;
	}
	
	else if (xPos >= 1.0- 0.09 / ratio || yPos >= 0.80)
	{
		float4 color = float4(0.521568, 0.521568, 0.505882, 1.0) *  gAmbientColor;
		color.rgb *= 1.f - yPos*.5f;
		color.a *= a;
		return color;
	}
	
	// param 4
	// x : percent
	// y : maximum
	float percent = gShaderParams[4].x * 2.0 - 1.0;
	if (snc.x < percent)
	{
		// 1: gauge color
		// 2: Blink color
		// 3x: sin
		float4 color = lerp(gShaderParams[1], gShaderParams[2], gShaderParams[3].x);
		color.a *= a;
		color.rgb *= 1.f - yPos*.5f;
		return color;
	}
	
	float4 color = lerp(gDiffuseColor, gShaderParams[2], gShaderParams[3].x);
	color.a *= a;	
    return color;
}