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

#include "ShaderCommon.h"
#ifndef __SHADER_CONSTANTS_H_
#define __SHADER_CONSTANTS_H_
#ifdef CPP
#pragma once
namespace fb {
#endif

cbuffer FRAME_CONSTANTS
	#ifndef CPP
		: register(b0)
	#endif
{
	float4 gMousePos; // x, y : current pos   z, w: button down pos // per frame	
	float gDeltaTime; // per frame
	float gTime; // per frame
	float2 gFrameConstDummy;
};

cbuffer OBJECT_CONSTANTS
	#ifndef CPP
		: register (b1)
	#endif
{
	float4x4 gWorld;
	float4x4 gWorldView;
	float4x4 gWorldViewProj;
};

cbuffer MATERIAL_CONSTANTS
	#ifndef CPP
		: register (b2)
	#endif
{
	float4 gAmbientColor;
	float4 gDiffuseColor;
	float4 gSpecularColor;
	float4 gEmissiveColor;
};

cbuffer MATERIAL_PARAMETERS
#ifndef CPP
	: register (b3)
#endif
{
	float4 gMaterialParam[5];
};

cbuffer RARE_CONSTANTS
#ifndef CPP
	: register (b4)
#endif
{	
	float gMiddleGray;
	float gStarPower;	
	float gBloomPower;
	float gRareDummy;
};

cbuffer BIG_BUFFER
#ifndef CPP
	: register (b5)
#endif
{
	float4 gSampleOffsets[16];
	float4 gSampleWeights[16];
};

cbuffer IMMUTABLE_CONSTANTS
#ifndef CPP
	: register (b6)
#endif
{
	float4 gIrradConstsnts[9];
	// will be casted float2[16]
	float4 gHammersley[8]; // reference http://www.cse.cuhk.edu.hk/~ttwong/papers/udpoint/udpoint.pdf
};

cbuffer POINT_LIGHT_CONSTANTS
#ifndef CPP
	: register (b7)
#endif
{
	float4 gPointLightPos[MAX_POINT_LIGHT];
	float4 gPointLightColor[MAX_POINT_LIGHT];
	
#ifdef CPP	
	POINT_LIGHT_CONSTANTS()
	{
		memset(this, 0, sizeof(POINT_LIGHT_CONSTANTS));
	}
#endif
	};

cbuffer CAMERA_CONSTANTS
	#ifndef CPP
		: register(b8)
	#endif
{
	float4x4 gView;
	float4x4 gInvView;
	float4x4 gViewProj;
	float4x4 gInvViewProj;
	float4x4 gCamTransform;
	float4x4 gProj;
	float4x4 gInvProj;
	float2 gNearFar;
	float gTangentTheta;
	float camera_constants_dummy;	
};

cbuffer RENDERTARGET_CONSTANTS
	#ifndef CPP
		: register(b9)
	#endif
{	
	float2 gScreenSize;
	float gScreenRatio;	
	float rendertarget_dummy;	
};

cbuffer SCENE_CONSTANTS
	#ifndef CPP
		: register(b10)
	#endif
{	
	float4x4 gLightView;	
	float4 gDirectionalLightDir_Intensity[2]; // per render target(actually per scene
	float4 gDirectionalLightDiffuse[2]; // per render target(actually per scene
	float4 gDirectionalLightSpecular[2]; // per render target(actually per scene	
	float4 gFogColor;
};

cbuffer SHADOW_CONSTANTS
	#ifndef CPP
		: register(b11)
	#endif
{	
	// will be accessed like 'float gCascadeFrustumsEyeSpaceDepthsFloat[8]
	float4 gCascadeFrustumsEyeSpaceDepthsFloat[2];
	
	float gCascadeBlendArea; // boundary	
	float gShadowPartitionSize;
	float gShadowTexelSize;
	float gShadowTexelSizeX;	//boundary
	
	float4 gCascadeOffset[8];
	float4 gCascadeScale[8];
};
	
#ifdef CPP
} // namespace
#endif


#endif //__SHADER_CONSTANTS_H_