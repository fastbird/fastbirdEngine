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

//----------------------------------------------------------------------------
// File: Point.hlsl
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Constant Buffer
//----------------------------------------------------------------------------
#include "EssentialEngineData/shaders/Constants.h"

//----------------------------------------------------------------------------
// Vertex Shader
//----------------------------------------------------------------------------
struct a2v
{
	float4 pos		: POSITION;
	float4 color	: COLOR;
};

//----------------------------------------------------------------------------
struct v2g
{
	float4 pos		: POSITION;
	float4 color	: COLOR;
};

struct g2p
{
	float4 pos		: SV_Position;
	float4 color	: COLOR;
};

//----------------------------------------------------------------------------
v2g Point_VertexShader( in a2v IN )
{
    v2g OUT;

	OUT.pos = mul(gWorldViewProj, IN.pos);
	OUT.color = IN.color;

	return OUT;
}

//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
cbuffer cbImmutable
{
	static float2 gUVs[4] = 
	{
		{-.5, .5},
		{-.5, -.5},
		{.5, .5},
		{.5, -.5}
	};
}
[maxvertexcount(4)]
void Point_GeometryShader(point v2g INPUT[1], inout TriangleStream<g2p> stream)
{
	g2p OUTPUT;
	for (int i=0; i<4; i++)
	{		
		float4 pos = INPUT[0].pos;
		pos.x += gUVs[i].x * 0.005f * INPUT[0].pos.w;
		pos.y += gUVs[i].y * 0.005f * gScreenRatio * INPUT[0].pos.w;
		OUTPUT.pos = pos;
		OUTPUT.color = INPUT[0].color;
		stream.Append(OUTPUT);
		if (i%4==3)
			stream.RestartStrip();
	}
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 Point_PixelShader( in g2p IN ) : SV_Target
{
	return IN.color;
}