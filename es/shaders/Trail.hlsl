//----------------------------------------------------------------------------
// File: Trail.hlsl
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Constant Buffer
//----------------------------------------------------------------------------
#include "Constants.h"

Texture2D  gDiffuseTexture : register(t0);

//----------------------------------------------------------------------------
// Vertex Shader
//----------------------------------------------------------------------------
struct a2v
{
	float4 pos		: POSITION; // pos.w : is u (length)
	float4 color	: COLOR;
};

struct v2g
{
	float4 pos : TEXTURE0; // pos.w : is u (length)
	float4 color : TEXTURE1;
};

//----------------------------------------------------------------------------
struct g2p
{
	float4 pos		: SV_Position;
	float4 color	: COLOR;
	float2 uv		: TEXCOORD;
};

//----------------------------------------------------------------------------
v2g trail_VertexShader( in a2v IN )
{
    v2g OUT;
	OUT.pos = IN.pos;
	OUT.color = IN.color;

	return OUT;
}

//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
[maxvertexcount(4)]
void trail_GeometryShader(lineadj v2g In[4], inout TriangleStream<g2p> streamObj)
{
	float3 camPos = {gCamTransform[0][3], gCamTransform[1][3], gCamTransform[2][3]};
	float3 toCam = normalize(camPos - In[1].pos.xyz);
	float3 lineDirPrev = normalize(In[1].pos.xyz - In[0].pos.xyz);
	float3 up0 = normalize(cross(toCam, lineDirPrev));
	
	float3 lineDirCur = normalize(In[3].pos.xyz - In[2].pos.xyz);
	float3 up1 = normalize(cross(toCam, lineDirCur))	;
	
	// 1    3
	//
	// 0    2
	float width = gMaterialParam[0].x;
	
	g2p OUTPUT;
	OUTPUT.pos = mul(gViewProj, float4(In[1].pos.xyz - up0 * width, 1));
	OUTPUT.color = In[1].color;
	float u = In[1].pos.w;
	OUTPUT.uv = float2(u, 1);
	streamObj.Append(OUTPUT);
	
	OUTPUT.pos = mul(gViewProj, float4(In[1].pos.xyz + up0 * width, 1));
	OUTPUT.color = In[1].color;
	OUTPUT.uv = float2(u, 0);
	streamObj.Append(OUTPUT);
	
	OUTPUT.pos = mul(gViewProj, float4(In[2].pos.xyz - up1 * width, 1));
	OUTPUT.color = In[2].color;
	u = In[2].pos.w;
	OUTPUT.uv = float2(u, 1);
	streamObj.Append(OUTPUT);
	
	OUTPUT.pos = mul(gViewProj, float4(In[2].pos.xyz + up1 * width, 1));
	OUTPUT.color = In[2].color;
	OUTPUT.uv = float2(u, 0);
	streamObj.Append(OUTPUT);
	
	streamObj.RestartStrip();
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
struct PS_OUT
{
	float4 color0 : SV_Target0;
	float4 color1 : SV_Target1;
};

PS_OUT trail_PixelShader(in g2p IN) : SV_Target
{
	float4 outColor = gDiffuseColor;
	outColor.a = IN.color.a;
#ifdef DIFFUSE_TEXTURE
	outColor *= gDiffuseTexture.Sample(gLinearSampler, IN.uv);
#endif
	PS_OUT outdata;
	outdata.color0 = outColor;
	outdata.color1 = outColor;
	return outdata;
}