#include "Constants.h"
#include "CommonFunctions.h"

//---------------------------------------------------------------------------
struct QuadVS_Output
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

float GGX_D(float ndh, float roughness)
{
	float alpha = roughness * roughness;
	float tmp = alpha / (ndh*ndh*(alpha*alpha-1.0)+1.0);
	return tmp * tmp * INV_PI;
}

float2 GGX_FV(float dotLH, float roughness) {
	float alpha = roughness*roughness;
	
	// F
	float F_a, F_b;
	float dotLH5 = pow(1.0f - dotLH, 5);
	F_a = 1.0f;
	F_b = dotLH5;
	
	
	// V
	float vis;
	float k = alpha * 0.5f;
	float k2 = k*k;
	float invK2 = 1.0f - k2;
	vis = rcp(dotLH * dotLH * invK2 + k2);
	return float2(F_a * vis, F_b * vis);
	
}

float4 ggxgen_PixelShader(QuadVS_Output Input) : SV_TARGET
{
	float4 output;
	float dotNH = pow(Input.Tex.x, 0.25f);
	float roughness = Input.Tex.y;
	output.x = GGX_D(dotNH, roughness);	
	output.yz = GGX_FV(Input.Tex.x, roughness);
	output.w = 1.0f;
	
	return output;	
}