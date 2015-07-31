//--------------------------------------------------------------------------------------
// File: BillboardQuad.hlsl
//--------------------------------------------------------------------------------------
// render as point sprite with geometry

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "Constants.h"

struct a2v
{
	float3 Position : POSITION;
	float4 Size_Offset : TEXCOORD0;
};

struct v2g
{
	float3 ViewPosition : POSITION;
	float4 Size_Offset : TEXCOORD0;
};

struct g2p
{
	float4 Position : SV_Position;
	float2 UV : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
v2g billboardquad_VertexShader(in a2v INPUT)
{
	v2g OUTPUT;
	OUTPUT.ViewPosition = mul(gView, float4(INPUT.Position, 1));
	OUTPUT.Size_Offset = INPUT.Size_Offset;
	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
cbuffer cbImmutable
{
	static float2 gUVs[4] = 
	{
		{0.0, 1.0},
		{0.0, 0.0},
		{1.0, 1.0},
		{1.0, 0.0}
	};
}

float2x2 GetMatRot(float rot)
{
	float c = cos(rot);
	float s = sin(rot);
	return float2x2(c, -s, s, c);
}
[maxvertexcount(4)]
void billboardquad_GeometryShader(point v2g INPUT[1], inout TriangleStream<g2p> stream)
{
	g2p OUTPUT;
	for (int i=0; i<4; i++)
	{
		float2 scale = INPUT[0].Size_Offset.xy;
		float2 pivot = INPUT[0].Size_Offset.zw;
		
		float3 u = vec3(1.0f, 0, 0) * scale.x;
		float3 v = vec3(0.0f, 0.0f, -1.0f) * scale.y;
		//float2x2 matRot = GetMatRot(INPUT[0].mRot_Alpha_uv.x);
		//u.xz = mul(matRot, u.xz);
		//v.xz = mul(matRot, v.xz);
		float3 viewPos = INPUT[0].ViewPosition + u * (gUVs[i].x-pivot.x) + v * (gUVs[i].y - pivot.y);

		OUTPUT.Position = mul(gProj, float4(viewPos, 1.0));
		OUTPUT.UV = gUVs[i];

		stream.Append(OUTPUT);			
	}
	stream.RestartStrip();
}

//---------------------------------------------------------------------------
// PIXEL SHADER
//---------------------------------------------------------------------------
Texture2D gDiffuseTexture : register(t0);
float4 billboardquad_PixelShader(in g2p INPUT) : SV_Target
{
	return float4(1, 1, 1, 1);
}