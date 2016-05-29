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
// File: Trail.hlsl
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Constant Buffer
//----------------------------------------------------------------------------
#include "EssentialEngineData/shaders/Constants.h"

Texture2D  gDiffuseTexture : register(t0);

//----------------------------------------------------------------------------
// Vertex Shader
//----------------------------------------------------------------------------
struct a2v
{
	float4 pos		: POSITION; // pos.w : is alpha (length)
};

struct v2g
{
	float4 pos : TEXTURE0; // pos.w : is u (length)
};

//----------------------------------------------------------------------------
struct g2p
{
	float4 pos		: SV_Position;
	float3 uv		: TEXCOORD; // z is alpha
};

//----------------------------------------------------------------------------
v2g Trail_VertexShader( in a2v IN )
{
    v2g OUT;
	OUT.pos = IN.pos;

	return OUT;
}

//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
[maxvertexcount(4)]
void Trail_GeometryShader(lineadj v2g In[4], inout TriangleStream<g2p> streamObj)
{
	float3 camPos = {gCamTransform[0][3], gCamTransform[1][3], gCamTransform[2][3]};
	float3 toCam = normalize(camPos - In[1].pos.xyz);
	float3 lineDirPrev = normalize(In[1].pos.xyz - In[0].pos.xyz);
	float3 up0 = normalize(cross(toCam, lineDirPrev));
	
	float3 lineDirCur = normalize(In[3].pos.xyz - In[2].pos.xyz);
	float3 up1 = normalize(cross(toCam, lineDirCur))	;
	
	// 1    3
	//
	// 0    2
	float width = gShaderParams[0].x;
	
	g2p OUTPUT;
	OUTPUT.pos = mul(gViewProj, float4(In[1].pos.xyz - up0 * width, 1));
	float u = In[1].pos.w;
	OUTPUT.uv = float3(0, 1, In[1].pos.w);
	streamObj.Append(OUTPUT);
	
	OUTPUT.pos = mul(gViewProj, float4(In[1].pos.xyz + up0 * width, 1));
	OUTPUT.uv = float3(0, 0, In[1].pos.w);
	streamObj.Append(OUTPUT);
	
	OUTPUT.pos = mul(gViewProj, float4(In[2].pos.xyz - up1 * width, 1));
	u = In[2].pos.w;
	OUTPUT.uv = float3(0, 1, In[2].pos.w);
	streamObj.Append(OUTPUT);
	
	OUTPUT.pos = mul(gViewProj, float4(In[2].pos.xyz + up1 * width, 1));
	OUTPUT.uv = float3(0, 0, In[2].pos.w);
	streamObj.Append(OUTPUT);
	
	streamObj.RestartStrip();
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
struct PS_OUT
{
	float4 color0 : SV_Target0;
	float4 color1 : SV_Target1;
};

PS_OUT Trail_PixelShader(in g2p IN) : SV_Target
{
	float4 outColor = gDiffuseColor;
	outColor.a = IN.uv.z;
#ifdef DIFFUSE_TEXTURE
	outColor *= gDiffuseTexture.Sample(gLinearSampler, IN.uv.xy);
#endif
	PS_OUT outdata;
	outdata.color0 = outColor;
	outdata.color1 = outColor;
	return outdata;
}