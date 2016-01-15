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
// File: ui.hlsl
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "EssentialEngineData/shaders/Constants.h"

//
Texture2D  gDiffuseTexture : register(t0);

#if defined(_ALPHA_TEXTURE) || defined(_ALPHA_TEXTURE_SEPERATED_UV)
Texture2D gAlphaTexture : register(t1);
#endif

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
struct a2v
{
	float4 Position : POSITION;
	float4 Color	: COLOR;
#if defined(DIFFUSE_TEXTURE) || defined(_ALPHA_TEXTURE)	
	float2 UV		: TEXCOORD0;
#endif
#if defined(_ALPHA_TEXTURE_SEPERATED_UV)
	float2 UV2 : TEXCOORD1;
#endif
};

struct v2p 
{
	float4 Position   : SV_Position;
#if defined(DIFFUSE_TEXTURE) || defined(_ALPHA_TEXTURE)	
	float2 UV		: TEXCOORD0;
#endif
#if defined(_ALPHA_TEXTURE_SEPERATED_UV)
	float2 UV2 : TEXCOORD1;
#endif
};

v2p ui_VertexShader( in a2v INPUT )
{
    v2p OUTPUT;

	OUTPUT.Position = INPUT.Position;
#if defined(DIFFUSE_TEXTURE) || defined(_ALPHA_TEXTURE)
	#ifdef _UV_ROT
		float2 center = gMaterialParam[0].xy;
		float theta = gTime;
		float c = cos(theta);
		float s = sin(theta);
		float2x2 matRot = { c, s, -s, c};
		float2 uv = INPUT.UV - center;
		uv = mul(matRot, uv);
		OUTPUT.UV =   uv + center;
	#else
		OUTPUT.UV = INPUT.UV;
	#endif
#endif

#if defined(_ALPHA_TEXTURE_SEPERATED_UV)
	OUTPUT.UV2 = INPUT.UV2;
#endif
	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 ui_PixelShader( in v2p INPUT ) : SV_Target
{	
#ifdef DIFFUSE_TEXTURE
	#ifdef _USE_LINEAR_SAMPLER
	float4 color = gDiffuseTexture.Sample(gLinearBlackBorderSampler, INPUT.UV);
	#else
	float4 color = gDiffuseTexture.Sample(gPointBlackBorderSampler, INPUT.UV);
	#endif
	color *= gDiffuseColor;
	float highlight = gEmissiveColor.w;
	float2 glowss = smoothstep(0.9, 1.0, abs(saturate(INPUT.UV)*2.0-1.0) * highlight);
	color = lerp(color, gEmissiveColor, glowss.x+glowss.y);
#else
	float4 color = gDiffuseColor;
#endif
	
#ifdef _DESATURATE
	color.rgb = (0.212671 * color.r + 0.71516 * color.g + 0.072169 * color.b)*.7;
#endif

	color.rgb += gAmbientColor.xyz;
	color.rgb *= gSpecularColor.xyz;
	color.a *= gMaterialParam[4].w;
	
#if defined(_ALPHA_TEXTURE)
	float a = gAlphaTexture.Sample(gPointSampler, INPUT.UV).a;
	color.a *= a;
#endif

#if defined(_ALPHA_TEXTURE_SEPERATED_UV)
	float a = gAlphaTexture.Sample(gPointSampler, INPUT.UV2).a;
	color.a *= a;
#endif
    return color;

}