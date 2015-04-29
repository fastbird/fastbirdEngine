#include "Constants.h"

struct a2v
{
	float3 pos		: POSITION;
};

//----------------------------------------------------------------------------
struct v2p
{
	float4 pos		: SV_Position;
	float4 viewPos		: TEXCOORD;
};

//----------------------------------------------------------------------------
v2p depth_VertexShader( in a2v IN )
{
    v2p OUT;

	float4 viewPos = mul(gWorldView, float4(IN.pos, 1));
	//if (viewPos.y<=gNearFar.x)
	{
		//viewPos *= gNearFar.x / viewPos.y;
		//viewPos.y = (gNearFar.y - gNearFar.x) + 0.01;
	}
	float4 projPos = mul( gProj, viewPos);
	
	float depthV = projPos.z/projPos.w;
	if ( depthV > 1.0)
		projPos.z = projPos.w;
		
		
	OUT.pos = projPos;
	OUT.viewPos = viewPos;
	
	return OUT;
};

float4 depth_PixelShader( in v2p IN ) : SV_Target
{
	float depth = (IN.viewPos.y- gNearFar.x) /(gNearFar.y - gNearFar.x) ; // before y-z swapping. so y is depth.

	if (depth > 1.0)
		discard;
	return float4(depth, depth, depth, 1);
}

