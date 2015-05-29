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
v2p depthonly_VertexShader( in a2v IN )
{
    v2p OUT;

	OUT.pos = mul(gWorldViewProj, float4(IN.pos, 1));
	
	return OUT;
};

float4 depthonly_PixelShader( in v2p IN ) : SV_Target
{
	return float4(0, 0, 0, 0);
}

