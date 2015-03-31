//--------------------------------------------------------------------------------------
// File: particle.hlsl
//--------------------------------------------------------------------------------------
// render as point sprite with geometry

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "Constants.h"
#include "CommonFunctions.h"

struct a2v
{
	float3 Position : POSITION;
	float4 UDirection_Intensity : TEXCOORD0;
	float3 VDirection : TEXCOORD1;
	float4 mPivot_Size : TEXCOORD2;
	float4 mRot_Alpha_uv : TEXCOORD3;
	float2 mUVStep : TEXCOORD4;
	float4 mColor : COLOR0;
};

struct v2g
{
	float3 ViewPosition : POSITION;
	float4 UDirection_Intensity : TEXCOORD0;
	float3 VDirection : TEXCOORD1;
	float4 mPivot_Size : TEXCOORD2;
	float4 mRot_Alpha_uv : TEXCOORD3;
	float2 mUVStep : TEXCOORD4;
	float3 mColor : COLOR0;
};

struct g2p
{
	float4 Position : SV_Position;
	float4 UV_Intensity_Alpha : TEXCOORD0;
	float3 mColor : TEXCOORD1;
	float2 ScreenUVData : TEXCOORD2;
	float3 ViewPos : TEXCOORD3;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
v2g particle_VertexShader(in a2v INPUT)
{
	v2g OUTPUT;
	OUTPUT.ViewPosition = mul(gView, float4(INPUT.Position, 1));
	OUTPUT.UDirection_Intensity = INPUT.UDirection_Intensity;
	OUTPUT.VDirection = INPUT.VDirection;
	OUTPUT.mPivot_Size = INPUT.mPivot_Size;
	OUTPUT.mRot_Alpha_uv = INPUT.mRot_Alpha_uv;
	OUTPUT.mUVStep = INPUT.mUVStep;
	OUTPUT.mColor = INPUT.mColor.xyz;
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
void particle_GeometryShader(point v2g INPUT[1], inout TriangleStream<g2p> stream)
{
	g2p OUTPUT;
	for (int i=0; i<4; i++)
	{
		float2 pivot = INPUT[0].mPivot_Size.xy;
		float2 scale = INPUT[0].mPivot_Size.zw;
		float3 u = INPUT[0].UDirection_Intensity.xyz * scale.x;
		float3 v = INPUT[0].VDirection * scale.y;
		float2x2 matRot = GetMatRot(INPUT[0].mRot_Alpha_uv.x);
		u.xz = mul(matRot, u.xz);
		v.xz = mul(matRot, v.xz);

		float3 viewPos = INPUT[0].ViewPosition + u * (gUVs[i].x-pivot.x) + v * (gUVs[i].y - pivot.y);

		OUTPUT.Position = mul(gProj, float4(viewPos, 1.0));

		float2 uvIndex = INPUT[0].mRot_Alpha_uv.zw;
		float2 uvStep = INPUT[0].mUVStep;
		OUTPUT.UV_Intensity_Alpha.xy = uvIndex * uvStep + gUVs[i] * uvStep;
		OUTPUT.UV_Intensity_Alpha.z = INPUT[0].UDirection_Intensity.w;
		OUTPUT.UV_Intensity_Alpha.w = INPUT[0].mRot_Alpha_uv.y;
		OUTPUT.mColor = INPUT[0].mColor;
		OUTPUT.ScreenUVData = OUTPUT.Position.xy / OUTPUT.Position.w;
		OUTPUT.ViewPos = viewPos;
		stream.Append(OUTPUT);
		if (i%4==3)
			stream.RestartStrip();
	}
}

//---------------------------------------------------------------------------
// PIXEL SHADER
//---------------------------------------------------------------------------
Texture2D gDiffuseTexture : register(t0);
Texture2D  gDepthTexture : register(t5);

#if !defined(_NO_GLOW)
struct PS_OUT
{
	float4 color0 : SV_Target0;
	float4 color1 : SV_Target1;
};
#endif

#ifdef _NO_GLOW
float4 particle_PixelShader(in g2p INPUT):SV_TARGET
#else
PS_OUT particle_PixelShader(in g2p INPUT):SV_TARGET
#endif
{
	#ifdef _NO_GLOW
	float4 output;
	#else
	PS_OUT output;
	#endif
	
	float2 screenTex = 0.5*( (INPUT.ScreenUVData) + float2(1,1));
    screenTex.y = 1 - screenTex.y;
	
	float sceneDepth = gDepthTexture.Sample(gLinearSampler, screenTex);	
	float particleDepth = (INPUT.ViewPos.y - gNearFar.x) / (gNearFar.y - gNearFar.x);

	float diff = abs(sceneDepth - particleDepth);
	diff *= diff;
	float depthFadeScale = 70000000;
	float depthFade = saturate(diff*depthFadeScale);		
	
	float4 diffuse = gDiffuseTexture.Sample(gLinearWrapSampler, INPUT.UV_Intensity_Alpha.xy);
	diffuse.xyz *= INPUT.mColor * INPUT.UV_Intensity_Alpha.z;
	//diffuse.w *= INPUT.UV_Intensity_Alpha.z;
	diffuse.w *= INPUT.UV_Intensity_Alpha.w;	
	
#if defined(_INV_COLOR_BLEND) || defined(_PRE_MULTIPLIED_ALPHA)
	float4 outColor = float4(diffuse.xyz*diffuse.w, diffuse.w);
#else
	float4 outColor = diffuse;
#endif


#ifdef _NO_GLOW
	return float4(outColor.rgb, outColor.a * depthFade);
#else
	output.color0 = float4(outColor.rgb, outColor.a*depthFade);
	float glowPower = gMaterialParam[0].x;
	output.color1 = float4(outColor.xyz * glowPower, outColor.a*depthFade);
	return output;
#endif
}