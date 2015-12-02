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

#include "Constants.h"

struct a2v
{
	float3 pos		: POSITION;
};

//----------------------------------------------------------------------------
struct v2p
{
	float4 pos		: SV_Position;
	float4 viewPos		: TEXCOORD;
};

//----------------------------------------------------------------------------
v2p depth_VertexShader( in a2v IN )
{
    v2p OUT;

	float4 viewPos = mul(gWorldView, float4(IN.pos, 1));
	//if (viewPos.y<=gNearFar.x)
	{
		//viewPos *= gNearFar.x / viewPos.y;
		//viewPos.y = (gNearFar.y - gNearFar.x) + 0.01;
	}
	float4 projPos = mul( gProj, viewPos);
	
	float depthV = projPos.z/projPos.w;
	if ( depthV > 1.0)
		projPos.z = projPos.w;
		
		
	OUT.pos = projPos;
	OUT.viewPos = viewPos;
	
	return OUT;
};

float4 depth_PixelShader( in v2p IN ) : SV_Target
{
	float depth = (IN.viewPos.y- gNearFar.x) /(gNearFar.y - gNearFar.x) ; // before y-z swapping. so y is depth.

	if (depth > 1.0)
		discard;
	return float4(depth, depth, depth, 1);
}

