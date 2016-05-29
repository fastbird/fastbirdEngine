// ScreenLocalPosVS.hlsl

#include "EssentialEngineData/shaders/Constants.h"
struct ScreenLocalPos{
	float4 screenPos : SV_Position;
	float3 localPos : TEXCOORD0;
};

ScreenLocalPos ScreenLocalPosVS_VertexShader(in float4 localPos : POSITION)
{
	ScreenLocalPos OUTPUT;
	OUTPUT.screenPos = mul(gWorldViewProj, localPos);
	OUTPUT.localPos = localPos.xyz;
	return OUTPUT;
}