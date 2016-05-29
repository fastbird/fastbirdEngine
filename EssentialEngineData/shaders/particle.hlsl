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
// File: Particle.hlsl
//--------------------------------------------------------------------------------------
// render as point sprite with geometry

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "EssentialEngineData/shaders/Constants.h"
#include "EssentialEngineData/shaders/CommonFunctions.h"

struct a2v
{
	float3 Position : POSITION;
	float4 UDirection_Intensity : TEXCOORD0;
	float3 VDirection : TEXCOORD1;
	float4 mPivot_Size : TEXCOORD2;
	float4 mRot_Alpha_uv : TEXCOORD3;
	float2 mUVStep : TEXCOORD4;
	float4 mColor : COLOR0;
};

struct v2g
{
#if _SCREEN_SPACE
	float3 PixelPosition : POSITION;
#else
	float3 ViewPosition : POSITION;
#endif
	float4 UDirection_Intensity : TEXCOORD0;
	float3 VDirection : TEXCOORD1;
	float4 mPivot_Size : TEXCOORD2;
	float4 mRot_Alpha_uv : TEXCOORD3;
	float2 mUVStep : TEXCOORD4;
	float4 mColor : COLOR0;
};

struct g2p
{
	float4 Position : SV_Position;
	float4 UV_Intensity_Alpha : TEXCOORD0;
	float4 mColor : TEXCOORD1;
	float2 ScreenUVData : TEXCOORD2;
	float3 ViewPos : TEXCOORD3;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
v2g Particle_VertexShader(in a2v INPUT)
{
	v2g OUTPUT;
#if _SCREEN_SPACE
	OUTPUT.PixelPosition = INPUT.Position;
#else	
	OUTPUT.ViewPosition = mul(gView, float4(INPUT.Position, 1));
#endif
	OUTPUT.UDirection_Intensity = INPUT.UDirection_Intensity;
	OUTPUT.VDirection = INPUT.VDirection;
	OUTPUT.mPivot_Size = INPUT.mPivot_Size;
	OUTPUT.mRot_Alpha_uv = INPUT.mRot_Alpha_uv;
	OUTPUT.mUVStep = INPUT.mUVStep;
	OUTPUT.mColor = INPUT.mColor;
	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
cbuffer cbImmutable
{
	static float2 gUVs[4] = 
	{
		{0.0, 1.0},
		{0.0, 0.0},
		{1.0, 1.0},
		{1.0, 0.0}
	};
}

float2x2 GetMatRot(float rot)
{
	float c = cos(rot);
	float s = sin(rot);
	return float2x2(c, -s, s, c);
}
[maxvertexcount(4)]
void Particle_GeometryShader(point v2g INPUT[1], inout TriangleStream<g2p> stream)
{
	g2p OUTPUT;
	for (int i=0; i<4; i++)
	{
		float2 pivot = INPUT[0].mPivot_Size.xy;
		float2 scale = INPUT[0].mPivot_Size.zw;
		float3 u = INPUT[0].UDirection_Intensity.xyz * scale.x;
		float3 v = INPUT[0].VDirection * scale.y;
		float2x2 matRot = GetMatRot(INPUT[0].mRot_Alpha_uv.x);		
#if _SCREEN_SPACE
		u.xy = mul(matRot, u.xy);
		v.xy = mul(matRot, v.xy);
		float3 viewPos = INPUT[0].PixelPosition + u * (gUVs[i].x-pivot.x) + v * (gUVs[i].y - pivot.y);
		// gWorldViewProj : pixel to ndc
		OUTPUT.Position = mul(gWorldViewProj, float4(viewPos, 1.0));
#else
		u.xz = mul(matRot, u.xz);
		v.xz = mul(matRot, v.xz);

		float3 viewPos = INPUT[0].ViewPosition + u * (gUVs[i].x-pivot.x) + v * (gUVs[i].y - pivot.y);
		OUTPUT.Position = mul(gProj, float4(viewPos, 1.0));
#endif		

		float2 uvIndex = INPUT[0].mRot_Alpha_uv.zw;
		float2 uvStep = INPUT[0].mUVStep;
		OUTPUT.UV_Intensity_Alpha.xy = uvIndex * uvStep + gUVs[i] * uvStep;
		OUTPUT.UV_Intensity_Alpha.z = INPUT[0].UDirection_Intensity.w;
		OUTPUT.UV_Intensity_Alpha.w = INPUT[0].mRot_Alpha_uv.y;
		OUTPUT.mColor = INPUT[0].mColor;
		OUTPUT.ScreenUVData = OUTPUT.Position.xy / OUTPUT.Position.w;
		OUTPUT.ViewPos = viewPos;
		stream.Append(OUTPUT);
		if (i%4==3)
			stream.RestartStrip();
	}
}

//---------------------------------------------------------------------------
// PIXEL SHADER
//---------------------------------------------------------------------------
Texture2D gDiffuseTexture : register(t0);
Texture2D  gDepthTexture : register(t5);

#if !defined(_NO_GLOW)
struct PS_OUT
{
	float4 color0 : SV_Target0;
	float4 color1 : SV_Target1;
};
#endif

#ifdef _NO_GLOW
float4 Particle_PixelShader(in g2p INPUT):SV_TARGET
#else
PS_OUT Particle_PixelShader(in g2p INPUT):SV_TARGET
#endif
{
	#ifdef _NO_GLOW
	float4 output;
	#else
	PS_OUT output;
	#endif
	
	
#if _NO_DEPTH_FADE || _SCREEN_SPACE
	float depthFade = 1.0f;
#else	
	float2 screenTex = 0.5*( (INPUT.ScreenUVData) + float2(1,1));
    screenTex.y = 1 - screenTex.y;
	float particleDepth = (INPUT.ViewPos.y - gNearFar.x) / (gNearFar.y - gNearFar.x);
	float sceneDepth = gDepthTexture.Sample(gLinearSampler, screenTex);	
	float diff = abs(sceneDepth - particleDepth);
	diff *= diff;
	float depthFadeScale = 70000000;
	float depthFade = saturate(diff*depthFadeScale);			
#endif
	
	float4 diffuse = gDiffuseTexture.Sample(gLinearWrapSampler, INPUT.UV_Intensity_Alpha.xy);
	diffuse.xyz *= INPUT.mColor.rgb * INPUT.UV_Intensity_Alpha.z;
	//diffuse.w *= INPUT.UV_Intensity_Alpha.z;
	diffuse.w *= INPUT.mColor.a * INPUT.UV_Intensity_Alpha.w;
	
#if defined(_INV_COLOR_BLEND) || defined(_PRE_MULTIPLIED_ALPHA)
	float4 outColor = float4(diffuse.xyz*diffuse.w, diffuse.w);
#else
	float4 outColor = diffuse;
#endif


#ifdef _NO_GLOW
	return float4(outColor.rgb, outColor.a * depthFade);	
#else
	output.color0 = float4(outColor.rgb, outColor.a*depthFade);	
	float glowPower = gShaderParams[0].x;
	output.color1 = float4(outColor.xyz * glowPower, outColor.a*depthFade);
	return output;
#endif
}