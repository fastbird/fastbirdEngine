//--------------------------------------------------------------------------------------
// File: ui.hlsl
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "Constants.h"
#include "CommonFunctions.h"
//
Texture2D  gDiffuseTexture : register(t0);
Texture2D  gDepthTexture : register(t5);

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
struct a2v
{
	float3 Position : POSITION;
	float2 UV		: TEXCOORD;
};

struct v2p 
{
	float4 Position   : SV_Position;
	float2 UV		: TEXCOORD0;
	float2 ScreenUVData : TEXCOORD1;
	float3 ViewPos : TEXCOORD2;
};

v2p ui3d_VertexShader( in a2v INPUT )
{
    v2p OUTPUT;

	OUTPUT.Position = mul(gWorldViewProj, float4(INPUT.Position, 1.0));
	OUTPUT.UV = INPUT.UV;
	OUTPUT.ScreenUVData = OUTPUT.Position.xy / OUTPUT.Position.w;
	OUTPUT.ViewPos = mul(gWorldView, float4(INPUT.Position, 1.0));
	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 ui3d_PixelShader( in v2p INPUT ) : SV_Target
{	
	float2 screenTex = 0.5*( (INPUT.ScreenUVData) + float2(1,1));
    screenTex.y = 1 - screenTex.y;
	
	float sceneDepth = gDepthTexture.Sample(gLinearSampler, screenTex);
	float uiDepth = (INPUT.ViewPos.y - gNearFar.x) / (gNearFar.y - gNearFar.x);
	float diff = abs(sceneDepth - uiDepth);
	float depthFade = saturate(diff*2000);
	if (depthFade<0.01f)
		discard;
	
	float4 color = gDiffuseTexture.Sample(gLinearSampler, INPUT.UV) * gDiffuseColor;
	color.a *= depthFade;
	return color;
}