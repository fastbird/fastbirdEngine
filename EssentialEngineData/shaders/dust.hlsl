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
// File: dust.hlsl
//--------------------------------------------------------------------------------------
// render as point sprite with geometry

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "EssentialEngineData/shaders/Constants.h"

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
struct a2v
{
	float3 Position : POSITION;
	float4 Color : COLOR;
};

struct v2g
{
	float3 Position : POSITION;
	float4 Color : COLOR;
};

struct g2p
{
	float4 Position   : SV_Position;
	float4 Color : COLOR;
};

cbuffer cbImmutable
{
	static float3 gPosOffsets[4] =
    {
        float3( -1, 0, 1 ),
        float3( 1, 0, 1 ),
        float3( -1, 0, -1 ),
        float3( 1, 0, -1 ),
    };
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
v2g dust_VertexShader( in a2v INPUT )
{
    v2g OUTPUT;

	OUTPUT.Position = INPUT.Position;
	OUTPUT.Color = INPUT.Color;

	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
[maxvertexcount(4)]
void dust_GeometryShader(point v2g INPUT[1], inout TriangleStream<g2p> stream)
{
	g2p OUTPUT;
	
	float3 camPos = {gCamTransform[0][3], gCamTransform[1][3], gCamTransform[2][3]};
#ifdef STAR
	float distance = length(INPUT[0].Position - camPos);
	float scale = min(1.f, distance * .0003f) + 0.3f;
	for(int i=0; i<4; i++)
    {
        float3 pos = gPosOffsets[i] * scale;
        pos = mul((float3x3)gInvView, pos) + INPUT[0].Position;
        OUTPUT.Position = mul(gViewProj, float4(pos,1.0)); 
        OUTPUT.Color = INPUT[0].Color;
        stream.Append(OUTPUT);
    }
#else
	// sizing in world space
	float distance = length(INPUT[0].Position - camPos);
	float scale = min(1.f, distance*.003f);
	for(int i=0; i<4; i++)
    {
        float3 pos = gPosOffsets[i] * scale;
        pos = mul((float3x3)gInvView, pos) + INPUT[0].Position;
        OUTPUT.Position = mul(gViewProj, float4(pos,1.0)); 
        OUTPUT.Color = INPUT[0].Color;
        stream.Append(OUTPUT);
    }
#endif
	

	//The output is always assumed to be a triangle strip. 
	//To make the output a triangle list, you must call RestartStrip between each triangle. 
	//Triangle fans are unsupported.
    stream.RestartStrip();

}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 dust_PixelShader( in g2p INPUT ) : SV_Target
{	
    return INPUT.Color;    // Yellow, with Alpha = 1
}