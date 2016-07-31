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

#ifndef _ShaderCommon_h_
#define _ShaderCommon_h_

#ifdef CPP

#include "FBMathLib/Mat44.h"
#include "FBMathLib/Vec4.h"

#define cbuffer struct
#define float4x4 Mat44f
#define float4 Vec4f
#define float3 Vec3f
#define float2 Vec2f

#else // !CPP

#pragma pack_matrix( row_major )
#define vec2 float2
#define vec3 float3
#define vec4 float4
#define fract frac
#define mix lerp

//----------------------------------------------------------------------------
// SamplerState
// defined RendererEnum.h - SAMPLERS
//----------------------------------------------------------------------------
SamplerState gPointSampler : register(s0);
SamplerState gLinearSampler : register(s1);
SamplerState gAnisotropicSampler : register(s2);
SamplerComparisonState gShadowSampler : register(s3);
SamplerState gPointWrapSampler : register(s4);
SamplerState gLinearWrapSampler : register(s5);
SamplerState gLinearBlackBorderSampler : register(s6);
SamplerState gPointBlackBorderSampler : register(s7);
SamplerState gLinearMirror : register(s8);

const float3 dielectricColor = {0.1, 0.1, 0.1};

#endif // !CPP

#define MAX_POINT_LIGHT 3

#endif //_ShaderCommon_h_