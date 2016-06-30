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
// ShaderParameters
//--------------------------------------------------------------------------------------

// 0 ratio, width, height, empty -> all ui common
// 1 gague color
// 2 blink color
// 3 percent blinkSin minY maxY

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "EssentialEngineData/shaders/Constants.h"

#if defined(_ALPHA_TEXTURE)
Texture2D gAlphaTexture : register(t1);
#endif

Texture2D  gImageFilled : register(t0);
Texture2D  gImageNotFilled : register(t1);

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
struct a2v
{
	float4 Position : POSITION;
	float4 Color	: COLOR;
#if defined(_GAUGE_IMAGE)
	float2 uvFilled		: TEXCOORD0;
	float2 uvNotFilled 	: TEXCOORD1;
#else
	float2 UV		: TEXCOORD;
#endif
};

struct v2p 
{
	float4 Position   : SV_Position;
#if defined(_GAUGE_IMAGE)
	float2 uvFilled		: TEXCOORD0;
	float2 uvNotFilled 	: TEXCOORD1;
#else
	float2 UV		: TEXCOORD;
#endif
};

v2p UIHorizontalGauge_VertexShader( in a2v INPUT )
{
    v2p OUTPUT;
	OUTPUT.Position = INPUT.Position;
#if defined(_GAUGE_IMAGE)
	OUTPUT.uvFilled = INPUT.uvFilled;
	OUTPUT.uvNotFilled = INPUT.uvNotFilled;
#else
	OUTPUT.UV = INPUT.UV;
#endif

	return OUTPUT;
}

float4 VerticalGaugeImage(float2 uvFilled, float2 uvNotFilled){
	float minY = gShaderParams[3].z;
	float maxY = gShaderParams[3].w;
	// 0~1 range.
	float curY = 1 - (uvFilled.y - minY) / (maxY - minY);
	float percent = gShaderParams[3].x;
	
	if (curY > percent)
	{	
		return gImageNotFilled.Sample(gLinearSampler, uvNotFilled);		
	}
	else
	{
		return gImageFilled.Sample(gLinearSampler, uvFilled);
	}
}

float4 VerticalGauge(float2 snc, float a){
	// param 4
	// x : percent	
	// y : blink sin
	// z : ratio
	float percent = gShaderParams[3].x * 2.0 - 1.0;	
	float blinkSin = gShaderParams[3].y;
	float ratio = gShaderParams[0].x;
	
	float edge = max(abs(snc.x), abs(snc.y));
	float xPos = abs(snc.x);
	float yPos = abs(snc.y);
	if ( yPos >= 1.0 - 0.03  * ratio || xPos >= 0.93)
	{
		float4 color = float4(1.0, 1.0, 1.0, 1.0) * gAmbientColor;
		color.rgb *= 1.f - xPos*.5f;
		color.a *= a;
		return color;
	}
	else if (yPos >= 1.0- 0.06 * ratio || xPos >= 0.88)
	{
		float4 color = float4(0.87058, 0.87058, 0.847058, 1.0) * gAmbientColor;
		color.rgb *= 1.f - xPos*.5f;
		color.a *= a;
		return color;
	}
	
	else if (yPos >= 1.0- 0.09 * ratio || xPos >= 0.80)
	{
		float4 color = float4(0.521568, 0.521568, 0.505882, 1.0) *  gAmbientColor;
		color.rgb *= 1.f - xPos*.5f;
		color.a *= a;
		return color;
	}
	
	// param 4
	// x : percent
	// y : maximum	
	if (snc.y < percent)
	{		
		float4 color = lerp(gShaderParams[1], gShaderParams[2], blinkSin);
		color.rgb *= 1.f - xPos*.5f;
		color.a *= a;		
		return color;
	}
	
	float4 color = lerp(gDiffuseColor, gShaderParams[2], blinkSin);
	color.a *= a;
	return color;
}

float4 HorizontalGauge(float2 snc, float a){
	// param 4
	// x : percent	
	// y : blink sin
	// z : ratio
	float percent = gShaderParams[3].x * 2.0 - 1.0;	
	float blinkSin = gShaderParams[3].y;
	float ratio = gShaderParams[0].x;
	
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
	
	if (snc.x < percent)
	{
		// 1: gauge color
		// 2: Blink color		
		float4 color = lerp(gShaderParams[1], gShaderParams[2], blinkSin);
		color.rgb *= 1.f - yPos*.5f;
		color.a *= a;		
		return color;
	}
	
	float4 color = lerp(gDiffuseColor, gShaderParams[2], blinkSin);
	color.a *= a;
	return color;
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
	
	// signed nc
#if !defined(_GAUGE_IMAGE)
	float2 snc = INPUT.UV * 2.0 - 1.0f;
	snc.y = -snc.y;
#endif
	
#if defined(_VERTICAL)
	#if defined(_GAUGE_IMAGE)
		return VerticalGaugeImage(INPUT.uvFilled, INPUT.uvNotFilled);
	#else
		return VerticalGauge(snc, a);
	#endif
#else
	return HorizontalGauge(snc, a);
#endif
}