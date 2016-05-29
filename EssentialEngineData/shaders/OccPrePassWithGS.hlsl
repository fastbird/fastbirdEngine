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
// File: OccPrePass.hlsl
//--------------------------------------------------------------------------------------
#include "EssentialEngineData/shaders/Constants.h"
struct a2v
{
	float3 Position : POSITION;
};

struct v2g
{
	float3 Position : POSITION;
};

struct g2p
{
	float4 Position : SV_Position;
};

#include "Data/shaders/ModulesImmutable.h"

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
v2g OccPrePassWithGS_VertexShader(in a2v INPUT)
{
	v2g OUTPUT;

	OUTPUT.Position = float4(INPUT.Position, 1.0);

	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
[maxvertexcount(24)]
void OccPrePassWithGS_GeometryShader(point v2g INPUT[1], inout TriangleStream<g2p> stream)
{
	g2p OUTPUT;
	float partSize = gShaderParams[0].x;
	float uvStepSize = gShaderParams[0].y;
	float radius = gShaderParams[0].z;
	for (int i = 0; i<24; i++)
	{
		float3 localPos = gPosOffsets[i] * radius;
		float3 pos = localPos + INPUT[0].Position*partSize;
		OUTPUT.Position = mul(gWorldViewProj, float4(pos, 1.f));
		stream.Append(OUTPUT);
		if (i % 4 == 3)
			stream.RestartStrip();
	}
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 OccPrePassWithGS_PixelShader(in g2p INPUT) : SV_Target
{
	return float4(0, 0, 0, 1.0);
}