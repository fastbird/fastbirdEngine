#include "Constants.h"

struct a2v
{
	float4 Position : POSITION;
	float2 UV : TEXCOORD;
};

struct v2p
{
	float4 Position : SV_Position;
	float2 UV : TEXCOORD0;
};

Texture2D  gDiffuseTexture : register(t0);

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
v2p marktextured_VertexShader(a2v IN)
{
	v2p OUTPUT;
	OUTPUT.Position = mul(gWorldViewProj, IN.Position); // in ndc space
	OUTPUT.UV = IN.UV;
	return OUTPUT;
}

//---------------------------------------------------------------------------
// PIXEL SHADER
//---------------------------------------------------------------------------
float4 marktextured_PixelShader(in v2p INPUT) : SV_Target
{	
	return gDiffuseTexture.Sample(gLinearSampler, 
		gMaterialParam[0].xy + INPUT.UV*gMaterialParam[0].zw) * gDiffuseColor;
	
}