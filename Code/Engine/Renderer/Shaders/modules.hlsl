//--------------------------------------------------------------------------------------
// File: modules.hlsl
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "Constants.h"
#include "CommonFunctions.hlsl"

Texture2D  gDiffuseTexture : register(t0);
SamplerState gDiffuseSampler : register(s0);

Texture2D  gNormalTexture : register(t1);
SamplerState gNormalSampler : register(s1);

Texture2D  gSpecularTexture : register(t2);
SamplerState gSpecularSampler : register(s2);

Texture2D  gAOTexture : register(t3);
SamplerState gAOSampler : register(s3);

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
struct a2v
{
	float3 Position : POSITION;
	float2 UV		: TEXCOORD;
	float4 Color	: COLOR;
};

struct v2g
{
	float3 Position : POSITION;
	float2 UV		: TEXCOORD;
	float4 Color    : COLOR;
};

struct g2p
{
	float4 Position : SV_Position;
	float3 Normal	: TEXCOORD0;
	float3 Tangent 	: TEXCOORD1;
	float3 Binormal : TEXCOORD2;
	float2 UV		: TEXCOORD3;
	float2 UV2		: TEXCOORD4;
	float3 LocalPos : TEXCOORD5;
	float3 WorldPos : TEXCOORD6;
	float3 Offset 	: TEXCOORD7;
	float4 Color 	: TEXCOORD8;
};

#include "ModulesImmutable.h"

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
v2g modules_VertexShader( in a2v INPUT )
{
    v2g OUTPUT;

	OUTPUT.Position = INPUT.Position;
	OUTPUT.UV = INPUT.UV;
	OUTPUT.Color = INPUT.Color;

	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
[maxvertexcount(24)]
void modules_GeometryShader(point v2g INPUT[1], inout TriangleStream<g2p> stream)
{
	g2p OUTPUT;
	float partSize = gMaterialParam[0].x;
	float uvStepSize = gMaterialParam[0].y;
	float radius = gMaterialParam[0].z;
	for (int i=0; i<24; i++)
	{
		float3 localPos = gPosOffsets[i]*radius;
		float3 pos = localPos + INPUT[0].Position*partSize;
		OUTPUT.Position = mul(gWorldViewProj, float4(pos, 1.f) );
		OUTPUT.Normal = mul((float3x3)gWorld, gNormal[i]);
		OUTPUT.Tangent = mul((float3x3)gWorld, gTangent[i]);
		OUTPUT.Binormal = cross(OUTPUT.Tangent, OUTPUT.Normal);
		OUTPUT.UV = INPUT[0].UV + (gUV[i] * uvStepSize);
		OUTPUT.UV2 = -1.0 + 2.0 * gUV[i];
		OUTPUT.LocalPos = localPos;
		OUTPUT.WorldPos = mul(gWorld, float4(pos, 1.0f));
		OUTPUT.Offset = gPosOffsets[i];
		OUTPUT.Color = INPUT[0].Color;
		stream.Append(OUTPUT);
		if (i%4==3)
			stream.RestartStrip();
	}
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 modules_PixelShader( in g2p INPUT ) : SV_Target
{
	float3 diffuseT = gDiffuseTexture.Sample(gDiffuseSampler, INPUT.UV).xyz;
	float3 normalT = gNormalTexture.Sample(gNormalSampler, INPUT.UV).xyz;
	float3 specularT = gSpecularTexture.Sample(gSpecularSampler, INPUT.UV).xyz;
	float aoT = gAOTexture.Sample(gAOSampler, INPUT.UV).xyz;
	float3 normal = INPUT.Normal;
	if (length(normalT)>0.0001)
	{
		normalT = fixNormalSample(normalT);
	    float3 normalMapWS = normalT.x*INPUT.Tangent + normalT.y*INPUT.Binormal;
		normal = normalize(normal + normalMapWS);
	}  
	
	// shading	
	float3 lightColor = gDirectionalLightDiffuse.xyz * gDirectionalLightDir_Intensity.w;
	float3 toLightDir = gDirectionalLightDir_Intensity.xyz;
	float3 toCamDir = normalize(gCameraPos - INPUT.WorldPos);
	float3 diffuse, specular;
	BlinnPhongShading(lightColor, normal, toLightDir, toCamDir, specularT, diffuse, specular);		
	float3 shadedColor = gAmbientColor.xyz*aoT + diffuse * diffuseT + specular;
	
	// edge emission and glow
	float3 edgeLitColor = INPUT.Color;
	float3 edgeGlow = edgeLitColor * 0.3;
	float3 localPos = INPUT.LocalPos;
	float3 offset = INPUT.Offset;
	float edgeLitPower = (sin(gTime+localPos.x+localPos.y+offset.x+offset.y ) + sin(gTime*2.0+localPos.y+localPos.z+offset.y+offset.z))*.5f
		* .5f + .5f;
	edgeLitPower = max(0.01, edgeLitPower);
	float2 glowss = smoothstep(0.7, 0.97, abs(INPUT.UV2.xy));
	float glowIntencity = min(1, max(glowss.x,glowss.y) * 1.0f);
	float3 glow = edgeGlow * glowIntencity * edgeLitPower;	
	shadedColor += glow*aoT;

	float2 ss = smoothstep(0.97, 1.0, abs(INPUT.UV2.xy));
	float3 finalColor = lerp( shadedColor, edgeLitColor * edgeLitPower, ss.x + ss.y);

    return float4( finalColor, 1.0f );    // Yellow, with Alpha = 1
}