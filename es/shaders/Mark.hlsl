#include "Constants.h"

struct a2v
{
	float3 Position : POSITION;
	float2 UV : TEXCOORD0;
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
v2p mark_VertexShader(in a2v INPUT)
{
	v2p OUTPUT;
	float3 center = {gWorld[0][3], gWorld[1][3], gWorld[2][3]};
	float4x4 vp = mul(gProj,gView);
	float3 camUp = {gCamTransform[0][2], gCamTransform[1][2], gCamTransform[2][2]};
	float radius = gMaterialParam[1].x;
	float scale = gMaterialParam[1].y;
	
	// up
	float3 barPos = center + camUp * radius;
	float4 ndcBarPos = mul(vp, float4(barPos, 1.0f));
	ndcBarPos.xy = ndcBarPos.xy / ndcBarPos.w;
	ndcBarPos.y += 34 / gScreenSize.y;	
	
	OUTPUT.Position = float4(INPUT.Position.xy, 0, 1.0);
	OUTPUT.Position.xy *= scale;
	OUTPUT.Position.xy += ndcBarPos.xy;
	OUTPUT.UV = INPUT.UV;

	return OUTPUT;
}

//---------------------------------------------------------------------------
// PIXEL SHADER
//---------------------------------------------------------------------------

float4 mark_PixelShader(in v2p INPUT) : SV_Target
{
#if DIFFUSE_TEXTURE
	return gDiffuseTexture.Sample(gLinearSampler, 
		gMaterialParam[0].xy + INPUT.UV*gMaterialParam[0].zw);
#else
	float3 color = gDiffuseColor;
	float alpha = gDiffuseColor.w;
	float3 highlightedColor = color+gEmissiveColor;
	float2 nuv;
	nuv.x	= abs(INPUT.UV.x * 2.0 - 1.0);
	nuv.y	= abs(INPUT.UV.y * 2.0 - 0.6);
	float2 glowss;
	glowss.x = smoothstep(0.6, 1.0, nuv.x);
	glowss.y = smoothstep(0.2, 0.6, nuv.y);
	float3 finalColor = lerp(color, highlightedColor, sin(gTime*4.0)*.5+.5);
	
	return float4(finalColor, alpha);
#endif	
}