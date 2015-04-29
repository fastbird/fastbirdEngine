#ifndef _ModuleImmutable_h
#define _ModuleImmutable_h
cbuffer cbImmutable
{
	static float3 gPosOffsets[24] =
	{
		// sides
		float3(-1.0f, -1.0f, -1.0f),
		float3(-1.0f, -1.0f, 1.0f),
		float3(1.0f, -1.0f, -1.0f),
		float3(1.0f, -1.0f, 1.0f),

		float3(1.0f, -1.0f, -1.0f),
		float3(1.0f, -1.0f, 1.0f),
		float3(1.0f, 1.0f, -1.0f),
		float3(1.0f, 1.0f, 1.0f),

		float3(1.0f, 1.0f, -1.0f),
		float3(1.0f, 1.0f, 1.0f),
		float3(-1.0f, 1.0f, -1.0f),
		float3(-1.0f, 1.0f, 1.0f),

		float3(-1.0f, 1.0f, -1.0f),
		float3(-1.0f, 1.0f, 1.0f),
		float3(-1.0f, -1.0f, -1.0f),
		float3(-1.0f, -1.0f, 1.0f),

		// up
		float3(-1.0f, -1.0f, 1.0f),
		float3(-1.0f, 1.0f, 1.0f),
		float3(1.0f, -1.0f, 1.0f),
		float3(1.0f, +1.0f, 1.0f),

		// down
		float3(-1.0f, 1.0f, -1.0f),
		float3(-1.0f, -1.0f, -1.0f),
		float3(1.0f, 1.0f, -1.0f),
		float3(1.0f, -1.0f, -1.0f),
	};

	static float3 gNormal[24] =
	{
		float3(0, -1, 0),
		float3(0, -1, 0),
		float3(0, -1, 0),
		float3(0, -1, 0),

		float3(1, 0, 0),
		float3(1, 0, 0),
		float3(1, 0, 0),
		float3(1, 0, 0),

		float3(0, 1, 0),
		float3(0, 1, 0),
		float3(0, 1, 0),
		float3(0, 1, 0),

		float3(-1, 0, 0),
		float3(-1, 0, 0),
		float3(-1, 0, 0),
		float3(-1, 0, 0),

		float3(0, 0, 1),
		float3(0, 0, 1),
		float3(0, 0, 1),
		float3(0, 0, 1),

		float3(0, 0, -1),
		float3(0, 0, -1),
		float3(0, 0, -1),
		float3(0, 0, -1),
	};

	static float3 gTangent[24] =
	{
		float3(1, 0, 0),
		float3(1, 0, 0),
		float3(1, 0, 0),
		float3(1, 0, 0),

		float3(0, 1, 0),
		float3(0, 1, 0),
		float3(0, 1, 0),
		float3(0, 1, 0),

		float3(-1, 0, 0),
		float3(-1, 0, 0),
		float3(-1, 0, 0),
		float3(-1, 0, 0),

		float3(0, -1, 0),
		float3(0, -1, 0),
		float3(0, -1, 0),
		float3(0, -1, 0),

		float3(1, 0, 0),
		float3(1, 0, 0),
		float3(1, 0, 0),
		float3(1, 0, 0),

		float3(-1, 0, 0),
		float3(-1, 0, 0),
		float3(-1, 0, 0),
		float3(-1, 0, 0),
	};

	static float2 gUV[24] =
	{
		// sides
		float2(0, 1),
		float2(0, 0),
		float2(1, 1),
		float2(1, 0),

		float2(0, 1),
		float2(0, 0),
		float2(1, 1),
		float2(1, 0),

		float2(0, 1),
		float2(0, 0),
		float2(1, 1),
		float2(1, 0),

		float2(0, 1),
		float2(0, 0),
		float2(1, 1),
		float2(1, 0),

		float2(0, 1),
		float2(0, 0),
		float2(1, 1),
		float2(1, 0),

		float2(0, 1),
		float2(0, 0),
		float2(1, 1),
		float2(1, 0),
	};
};
#endif 