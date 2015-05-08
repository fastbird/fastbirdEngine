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