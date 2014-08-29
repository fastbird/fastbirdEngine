#include "ShaderCommon.h"
#ifdef CPP
#pragma once
namespace fastbird {
#endif

cbuffer FRAME_CONSTANTS
	#ifndef CPP
		: register(b0)
	#endif
{
	float4x4 gView;
	float4x4 gInvView;
	float4x4 gViewProj;
	float4x4 gInvViewProj;
	float4 gDirectionalLightDir_Intensity;
	float4 gDirectionalLightDiffuse;
	float4 gDirectionalLightSpecular;
	float3 gCameraPos;
	float gTime;
	float4 gMousePos; // x, y : current pos   z, w: down pos
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
	float4x4 gProj;
	float2 gNearFar;
	float2 gScreenSize;
	float gTangentTheta;
	float gScreenRatio;
	float2 gEmpty;
	float4 gFogColor;
};

cbuffer BIG_BUFFER
#ifndef CPP
	: register (b5)
#endif
{
	float4 gSampleOffsets[15];
	float4 gSampleWeights[15];
};

cbuffer IMMUTABLE_CONSTANTS
#ifndef CPP
	: register (b6)
#endif
{
	float2 gHammersley[256]; // reference http://www.cse.cuhk.edu.hk/~ttwong/papers/udpoint/udpoint.pdf
};

#ifdef CPP
}
#endif