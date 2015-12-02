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
// File: ThickLine.hlsl
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Constant Buffer
//----------------------------------------------------------------------------
#include "Constants.h"

Texture2D  gDiffuseTexture : register(t0);

//----------------------------------------------------------------------------
// Vertex Shader
//----------------------------------------------------------------------------
struct a2v
{
	float4 pos		: POSITION;
	float4 color	: COLOR;
	float4 uv		: TEXCOORD0;// uv.z is thickness, uv.w is direction
	float3 next 	: TEXCOORD1; 
};

//----------------------------------------------------------------------------
struct v2p
{
	float4 pos		: SV_Position;
	float4 color	: COLOR;
	float2 uv		: TEXCOORD;
};

//----------------------------------------------------------------------------
v2p thickline_VertexShader( in a2v IN )
{
    v2p OUT;

	float4 curProjected = mul(gWorldViewProj, IN.pos);
	float2 curScreen = curProjected.xy / curProjected.w;
	curScreen.x *= gScreenRatio;
	
	float4 nextProjected = mul(gWorldViewProj, float4(IN.next.xyz, 1.f));
	float2 nextScreen = nextProjected.xy / nextProjected.w;
	nextScreen.x *= gScreenRatio;
	
	float2 dir = normalize(nextScreen - curScreen);
	float2 normal = float2(dir.y, -dir.x);
	
	float thickness = IN.uv.z;
	normal *= thickness * .5f;
	normal.x /= gScreenRatio;
	float direction = IN.uv.w; // direction is 1 or -1
	float4 offset = float4(normal * direction * curProjected.w, 0, 0);
	OUT.pos = curProjected + offset;
	OUT.color = IN.color;
	OUT.uv = IN.uv.xy;

	return OUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
struct PS_OUT
{
	float4 color0 : SV_Target0;
	float4 color1 : SV_Target1;
};

PS_OUT thickline_PixelShader(in v2p IN) : SV_Target
{
	float4 outColor = IN.color;
#ifdef DIFFUSE_TEXTURE
	outColor *= gDiffuseTexture.Sample(gLinearSampler, IN.uv);
#endif
	PS_OUT outdata;
	outdata.color0 = outColor;
	outdata.color1 = outColor;
	return outdata;
}