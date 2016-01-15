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
#include "EssentialEngineData/shaders/CommonFunctions.h"

//---------------------------------------------------------------------------
struct QuadVS_Output
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

float GGX_D(float ndh, float roughness)
{
	float alpha = roughness * roughness;
	float tmp = alpha / (ndh*ndh*(alpha*alpha-1.0)+1.0);
	return tmp * tmp * INV_PI;
}

float2 GGX_FV(float dotLH, float roughness) {
	float alpha = roughness*roughness;
	
	// F
	float F_a, F_b;
	float dotLH5 = pow(1.0f - dotLH, 5);
	F_a = 1.0f;
	F_b = dotLH5;
	
	
	// V
	float vis;
	float k = alpha * 0.5f;
	float k2 = k*k;
	float invK2 = 1.0f - k2;
	vis = rcp(dotLH * dotLH * invK2 + k2);
	return float2(F_a * vis, F_b * vis);
	
}

float4 ggxgen_PixelShader(QuadVS_Output Input) : SV_TARGET
{
	float4 output;
	float dotNH = pow(Input.Tex.x, 0.25f);
	float roughness = Input.Tex.y;
	output.x = GGX_D(dotNH, roughness);	
	output.yz = GGX_FV(Input.Tex.x, roughness);
	output.w = 1.0f;
	
	return output;	
}