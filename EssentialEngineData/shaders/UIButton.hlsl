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
#include "Constants.h"

#ifdef _ALPHA_TEXTURE
Texture2D  gAlphaTexture : register(t1);
#endif 

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

v2p uibutton_VertexShader( in a2v INPUT )
{
    v2p OUTPUT;

	OUTPUT.Position = INPUT.Position;
	OUTPUT.UV = INPUT.UV;

	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 uibutton_PixelShader( in v2p INPUT ) : SV_Target
{
	float ratio = gMaterialParam[0].x;
	float width = gMaterialParam[0].y;
	float height = gMaterialParam[0].z;
	
	float2 uv = abs(INPUT.UV*2.0-1.0);
	
	// gMaterialParam[1] is edge color
#ifdef BUTTON_EDGE
	if (uv.x > 1.0-(2.0/width))
		return gMaterialParam[1];
	if (uv.y> 1.0-(2.0/height))
		return gMaterialParam[1];
#endif
		
#if defined(DIFFUSE_TEXTURE) || defined(_ALPHA_TEXTURE)
    
#endif

	float4 color = gDiffuseColor;
	color.rgb += gAmbientColor.rgb;
	
#ifdef _ALPHA_TEXTURE	
	float4 t_color = gAlphaTexture.Sample(gLinearSampler, INPUT.UV);
	color.a *= t_color.a;
#endif
    return color;
}