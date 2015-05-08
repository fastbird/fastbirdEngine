#include "Constants.h"

struct a2v
{
	float3 pos		: POSITION;
};

//----------------------------------------------------------------------------
struct v2p
{
	float4 pos		: SV_Position;
	float depth		: TEXCOORD;
};

//----------------------------------------------------------------------------
v2p depth_cloud_VertexShader( in a2v IN )
{
    v2p OUT;

	float4 viewPos = mul(gWorldView, float4(IN.pos, 1));
	if (viewPos.y<=gNearFar.x)
	{
		//viewPos *= gNearFar.x / viewPos.y;
		viewPos.y = gNearFar.x+0.01;
	}
	float4 projPos = mul( gProj, viewPos);
	
	float depthV = projPos.z/projPos.w;
	if ( depthV > 1.0)
		projPos.z = projPos.w;
		
		
	OUT.pos = projPos;
	OUT.depth = (viewPos.y- gNearFar.x) /(gNearFar.y - gNearFar.x) ; // before y-z swapping. so y is depth.
	return OUT;
};

float4 depth_cloud_PixelShader( in v2p IN ) : SV_Target
{
	return float4(IN.depth, IN.depth, IN.depth, 1);
}

