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
// File: skybox.hlsl
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Constant Buffer
//----------------------------------------------------------------------------
#include "Constants.h"

//----------------------------------------------------------------------------
// Textures
//----------------------------------------------------------------------------
TextureCube  gSkyCubeMap : register(t0);


//----------------------------------------------------------------------------
struct a2v
{
	float4 Position		: POSITION;
	float3 uvw	: TEXCOORD;
};

//----------------------------------------------------------------------------
struct v2p
{
	float4 Position   : SV_Position;
	float3 uvw : TEXCOORD;
};

//----------------------------------------------------------------------------
v2p skybox_VertexShader(in a2v INPUT)
{
	v2p OUTPUT;
	float3 camPos = {gCamTransform[0][3], gCamTransform[1][3], gCamTransform[2][3]};
	float4 pos = INPUT.Position;
	pos.xyz += camPos;
	OUTPUT.Position = mul(gViewProj, pos);
	OUTPUT.uvw = INPUT.uvw;
	return OUTPUT;
}

//----------------------------------------------------------------------------
float4 skybox_PixelShader(in v2p INPUT) : SV_Target
{
	return gSkyCubeMap.Sample(gAnisotropicSampler, INPUT.uvw);
}