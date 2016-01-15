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
	float4 Position : POSITION;
	float2 UV : TEXCOORD;
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
v2p marktextured_VertexShader(a2v IN)
{
	v2p OUTPUT;
	OUTPUT.Position = mul(gWorldViewProj, IN.Position); // in ndc space
	OUTPUT.UV = IN.UV;
	return OUTPUT;
}

//---------------------------------------------------------------------------
// PIXEL SHADER
//---------------------------------------------------------------------------
float4 marktextured_PixelShader(in v2p INPUT) : SV_Target
{	
	return gDiffuseTexture.Sample(gLinearSampler, 
		gMaterialParam[0].xy + INPUT.UV*gMaterialParam[0].zw) * gDiffuseColor;
	
}