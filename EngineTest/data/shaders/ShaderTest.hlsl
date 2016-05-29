#include "EssentialEngineData/shaders/FullScreenQuadVS.hlsl"

float4 ShaderTest_PixelShader(QuadVS_Output INPUT){
	float3 camPos = {gCamTransform[0][3], gCamTransform[1][3], gCamTransform[2][3]};
	float4 projectedPos = {INPUT.UV.x*2.f - 1.f, INPUT.UV.y * 2.f - 1.f, 0.99f, 1.0f};
	float4 worldPos = mul(gInvViewProj, projectedPos);	
	worldPos.xyz /= worldPos.w;
	
	float3 dir = normalize(worldPos);
	return float4(0, 0, 1, 1);
}
