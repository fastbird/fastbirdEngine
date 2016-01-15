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
// File: UIHexagonal.hlsl
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "EssentialEngineData/shaders/Constants.h"


Texture2D  gDiffuseTexture : register(t0);

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
struct a2v
{
	float4 Position : POSITION;
	float4 Color	: COLOR;
	float2 UV		: TEXCOORD;
};

struct v2p
{
	float4 Position   : SV_Position;
	float2 UV		: TEXCOORD;
};

v2p uihexagonal_VertexShader(in a2v INPUT)
{
	v2p OUTPUT;

	OUTPUT.Position = INPUT.Position;	

	OUTPUT.UV = INPUT.UV;


	return OUTPUT;
}

cbuffer cbImmutable
{
	static vec2 gOrigins[6] = {
		vec2(0.0, 0.61),
		vec2(0.52, 0.30),
		vec2(0.52, -0.30),
		vec2(0.0, -0.61),
		vec2(-0.52, -0.30),
		vec2(-0.52, 0.30),
		
	};
//--------------------------------------------------------------------------------------
}
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 uihexagonal_PixelShader(in v2p INPUT) : SV_Target
{
	float3 diffuse = gDiffuseTexture.Sample(gLinearSampler, INPUT.UV);
	float gEnables[6];

	// param1 : HexaEnable for indices 0, 1, 2, 3
	// param2 : HexaEnable for indices 4, 5, UI World Normalized Size.xy
	// param3 : World Normalized pos.xy
	
	gEnables[0] = gMaterialParam[1].x;
	gEnables[1] = gMaterialParam[1].y;
	gEnables[2] = gMaterialParam[1].z;
	gEnables[3] = gMaterialParam[1].w;
	gEnables[4] = gMaterialParam[2].x;
	gEnables[5] = gMaterialParam[2].y;

	vec2 uv = INPUT.UV;
	uv = uv*2.0 - 1.0;
	uv.y = -uv.y;

	vec2 mouse = gMousePos.xy / gScreenSize.xy;
	mouse = (mouse - gMaterialParam[3].xy) / gMaterialParam[2].zw ;
	mouse = mouse*2.0 - 1.0;
	mouse.y = -mouse.y;
	
	vec2 mouse2 = gMousePos.zw / gScreenSize.xy;
	mouse2 = (mouse2 - gMaterialParam[3].xy) / gMaterialParam[2].zw ;
	mouse2 = mouse2*2.0 - 1.0;
	mouse2.y = -mouse2.y;
	
	vec4 retColor = vec4(0.0, 0.0, 0.0, 1.0);
	//return vec4(1, 1, 1, 1);
	for (int i = 0; i<6; ++i)
	{
		if (gEnables[i] == 0.0f)
			continue;
		// world means just entire Hexagonal Area.
		vec2 worldUV = gOrigins[i] - uv;
		worldUV = abs(worldUV);
		
		vec2 worldMouse = gOrigins[i] - mouse;
		worldMouse = abs(worldMouse);
		
		vec2 worldClick = gOrigins[i] - mouse2;
		worldClick = abs(worldClick);
		
		float r = max(worldUV.x + worldUV.y*0.57735, worldUV.y*1.1547) - 0.2;
		float m = max(worldMouse.x + worldMouse.y*0.57735, worldMouse.y*1.1547) - 0.2;
		float c = max(worldClick.x + worldClick.y*0.57735, worldClick.y*1.1547) - 0.2;
		c = 1.0 - c;
		r = 1.0 - r;
		if (r>0.9)
		{
			if (r<0.92)
				retColor=float4(diffuse * vec3(0.75, 0.7, 0.7), 1.0);
			else if (r < 0.93)
				retColor=vec4(0.752, 0.7, 0.7, 1.0);
			else if (m<0.1)
			{
				// on mouse over and click
				retColor = vec4(diffuse*5 * vec3(r, c, 0), 1.0);
			}
			else
			{
				retColor = float4(diffuse*1.2, 1.0);
			}
			return retColor;
		}
	}
	discard;

	return retColor;
}