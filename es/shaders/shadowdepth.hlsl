#include "Constants.h"

struct a2v
{
	float3 pos		: POSITION;
};

//----------------------------------------------------------------------------
struct v2p
{
	float4 pos		: SV_Position;
};

//----------------------------------------------------------------------------
v2p shadowdepth_VertexShader( in a2v IN )
{
    v2p OUT;

	float3 worldPos = mul(gWorld, float4(IN.pos, 1));
	OUT.pos = mul(gLightViewProj, float4(worldPos, 1));
	if (OUT.pos.z <=0) {
		OUT.pos.z = 0;
	}
	return OUT;
};

//----------------------------------------------------------------------------
float4 shadowdepth_PixelShader( in v2p IN ) : SV_Target
{
	return float4(0.0, 0.0, 0.0, 0.0);
}

