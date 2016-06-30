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
// File: UIVerticalGaugeImage.hlsl
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "EssentialEngineData/shaders/Constants.h"

Texture2D  gImageFilled : register(t0);
Texture2D  gImageNotFilled : register(t1);
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
struct a2v
{
	float4 Position : POSITION;
	float4 Color	: COLOR;
	float2 uvFilled		: TEXCOORD0;
	float2 uvNotFilled 	: TEXCOORD1;
};

struct v2p 
{
	float4 Position   : SV_Position;
	float2 uvFilled		: TEXCOORD0;
	float2 uvNotFilled		: TEXCOORD1;
};

v2p UIVerticalGaugeImage_VertexShader( in a2v INPUT )
{
    v2p OUTPUT;
	OUTPUT.Position = INPUT.Position;
	OUTPUT.uvFilled = INPUT.uvFilled;
	OUTPUT.uvNotFilled = INPUT.uvNotFilled;

	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 UIVerticalGaugeImage_PixelShader( in v2p INPUT ) : SV_Target
{	
	float minY = gShaderParams[2].x;
	float maxY = gShaderParams[2].y;
	// 0~1 range.
	float curY = 1 - (INPUT.uvFilled.y - minY) / (maxY - minY);
	float percent = gShaderParams[1].x / gShaderParams[1].y;
	
	if (curY > percent)
	{	
		return gImageNotFilled.Sample(gLinearSampler, INPUT.uvNotFilled);
	}
	else
	{
		return gImageFilled.Sample(gLinearSampler, INPUT.uvFilled);
	}
}