//--------------------------------------------------------------------------------------
// File: OccPrePass.hlsl
//--------------------------------------------------------------------------------------
#include "Constants.h"
struct a2v
{
	float3 Position : POSITION;
};

struct v2g
{
	float3 Position : POSITION;
};

struct g2p
{
	float4 Position : SV_Position;
};

#include "ModulesImmutable.h"

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
v2g occprepassgs_VertexShader(in a2v INPUT)
{
	v2g OUTPUT;

	OUTPUT.Position = float4(INPUT.Position, 1.0);

	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
[maxvertexcount(24)]
void occprepassgs_GeometryShader(point v2g INPUT[1], inout TriangleStream<g2p> stream)
{
	g2p OUTPUT;
	float partSize = gMaterialParam[0].x;
	float uvStepSize = gMaterialParam[0].y;
	float radius = gMaterialParam[0].z;
	for (int i = 0; i<24; i++)
	{
		float3 localPos = gPosOffsets[i] * radius;
		float3 pos = localPos + INPUT[0].Position*partSize;
		OUTPUT.Position = mul(gWorldViewProj, float4(pos, 1.f));
		stream.Append(OUTPUT);
		if (i % 4 == 3)
			stream.RestartStrip();
	}
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 occprepassgs_PixelShader(in g2p INPUT) : SV_Target
{
	return float4(0, 0, 0, 1.0);
}