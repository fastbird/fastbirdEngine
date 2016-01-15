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
// File: skySphere.hlsl
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Constant Buffer
//----------------------------------------------------------------------------
#include "EssentialEngineData/shaders/Constants.h"
#include "EssentialEngineData/shaders/CommonFunctions.h"

//----------------------------------------------------------------------------
// Textures
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
struct a2v
{
	float3 Position		: POSITION;
	float2 UV			: TEXCOORD;
};

//----------------------------------------------------------------------------
struct v2p
{
	float4 Position		: SV_Position;
	float3 WorldDir		: TEXCOORD0;
};

//----------------------------------------------------------------------------
// VertexShader
//----------------------------------------------------------------------------
v2p skysphere_VertexShader(in a2v INPUT)
{
	v2p OUTPUT;
	float4 pos = float4(INPUT.Position, 1.0);
	pos.z = 0.9999;
	OUTPUT.Position = pos;
	float4 worldPos = mul(gInvViewProj, pos);
	worldPos.xyz /= worldPos.w;
	float3 camPos = {gCamTransform[0][3], gCamTransform[1][3], gCamTransform[2][3]};
	OUTPUT.WorldDir = worldPos.xyz - camPos.xyz;

	return OUTPUT;
}
/*
float2 SphericalCoord2(float3 dir)
{
	float n = length(dir.xy);
	float x_xyPlane = (n > 0.000001) ? dir.x / n : 0;
	float2 uv;
	uv.x = acos(x_xyPlane)*INV_PI*.5;
	uv.x = (dir.y > 0.0) ? uv.x : 1.0-uv.x;
	uv.y = 1.0 - (atan(dir.z / n) * INV_PI+ 0.5);
	//uv.y = 1.0f - (dir.z*.5 + .5);
	
	return uv;
}*/

//----------------------------------------------------------------------------
// PIXEL shader
//----------------------------------------------------------------------------
float4 skysphere_PixelShader(in v2p INPUT) : SV_Target
{
	float3 dir =normalize(INPUT.WorldDir);
	float3 starColor = {0, 0, 0};
	float3 star = gDirectionalLightDir_Intensity[0].xyz;
	float3 color= gDirectionalLightDiffuse[0].xyz * gDirectionalLightDir_Intensity[0].w;
	float d = pow(saturate(dot(star, dir)-0.6), 5) * 2;
	float3 starColor += color * d;
	
	float ringIntencity=0;
	if (dir.z>0)
	{
		ringIntencity = sin(dir.x*2)*cos(dir.y*2)*tan((dir.z)*2);
	}
	float noise = PerlinNoise2D(((dir.x + dir.z)*0.25+0.5)*2.0f, ((dir.y+dir.z)*0.25+0.5)*2.0f)*.5 +.5; //abs(PerlinNoise1D(abs(dir.x*2) + abs(dir.y*2) + abs(dir.z)));
	float fogIntencity = noise;
	
	float3 starRing = starColor*abs(1.0-ringIntencity) + ringIntencity*float3(0.01, 0.0, 0.001);
	
	return float4(lerp(starRing, vec3(0.1, 0.25, 0.25), fogIntencity), 1);
	//float2 uv = SphericalCoord2(dir);
	//return gEnvMap.Sample(gEnvSampler, uv);	
}