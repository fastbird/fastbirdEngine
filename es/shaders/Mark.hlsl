#include "Constants.h"

struct a2v
{
	float3 Position : POSITION;
};

struct v2g
{
	float4 NDCPos : POSITION;
};

struct g2p
{
	float4 Position : SV_Position;
	float2 UV : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
cbuffer cbImmutable
{
	static float2 gUVs[3] = 
	{
		{0.0, 0.0},
		{1.0, 0.0},
		{0.5, 0.6}
	};
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
v2g mark_VertexShader(uint id : SV_VertexID)
{
	v2g OUTPUT;
	OUTPUT.NDCPos = mul(gWorldViewProj, float4(0, 0, 0, 1)); // in ndc space
	return OUTPUT;
}



[maxvertexcount(3)]
void mark_GeometryShader(point v2g INPUT[1], inout TriangleStream<g2p> stream)
{
	g2p OUTPUT;
	float scale = gWorld[0][0];
	float u = scale;
	float v = -scale*gScreenRatio;
	float2 pivot = {0.5, 1.0};
	float2 NDCPos = INPUT[0].NDCPos.xy/INPUT[0].NDCPos.w;
	for (int i=0; i<3; i++)
	{		
		float2 newpos;
		newpos.x = NDCPos.x + u * (gUVs[i].x-pivot.x);
		newpos.y = NDCPos.y + v * (gUVs[i].y-pivot.y);

		OUTPUT.Position = float4(newpos, 0, 1);
		OUTPUT.UV = gUVs[i];

		stream.Append(OUTPUT);			
	}
	stream.RestartStrip();
}

//---------------------------------------------------------------------------
// PIXEL SHADER
//---------------------------------------------------------------------------

float4 mark_PixelShader(in g2p INPUT) : SV_Target
{
	float3 color = gDiffuseColor;
	float3 highlightedColor = color+gEmissiveColor;
	float2 nuv;
	nuv.x	= abs(INPUT.UV.x * 2.0 - 1.0);
	nuv.y	= abs(INPUT.UV.y * 2.0 - 0.6);
	float2 glowss;
	glowss.x = smoothstep(0.6, 1.0, nuv.x);
	glowss.y = smoothstep(0.2, 0.6, nuv.y);
	float3 finalColor = lerp(color, highlightedColor, sin(gTime*4.0)*.5+.5);
	
	return float4(finalColor, 1.0);
	// float2 tex = INPUT.UV*2.0 -1.0;
	// tex.y=-tex.y;
	// float len = length(tex);
	// if (len> 1.0)
		// discard;
	// float alpha = 1.0- smoothstep(0.5, 1.0, len);
	// float3 color = INPUT.Color*INPUT.Color*INPUT.Color*3.0;
	// return float4(color, alpha);
}