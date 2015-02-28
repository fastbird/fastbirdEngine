#include "Constants.h"

Texture2D gSmallMap : register(t0);
Texture2D gBigMap : register(t1);
Texture2D gSceneDepth : register(t2);

//---------------------------------------------------------------------------
struct QuadVS_Output
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

float4 silouette_PixelShader( in QuadVS_Output IN ) : SV_Target
{
	float fromSamll = gSmallMap.Sample(gLinearSampler, IN.Tex);
	float fromBig = gBigMap.Sample(gPointSampler, IN.Tex);
	float sceneDepth = gSceneDepth.Sample(gPointSampler, IN.Tex);
	float gap = abs(fromSamll - fromBig);
	if (gap<0.01)
		discard;
		
	//if (fromBig > sceneDepth)
		//discard;
		
		

		
	return float4(0.7, 0.7, 0.7, 0.5);
}

