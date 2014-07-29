//--------------------------------------------------------------------------------------
// File: MeshPBR.hlsl
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "Constants.h"
#include "CommonFunctions.hlsl"

Texture2D  gDiffuseTexture : register(t0);
SamplerState gDiffuseSampler : register(s0);

#ifdef NORMAL_TEXTURE
Texture2D  gNormalTexture : register(t1);
SamplerState gNormalSampler : register(s1);
#endif //NORMAL_TEXTURE

Texture2D  gMetallicTexture : register(t2);  // rgb : specular color, a : roughness
SamplerState gMetallicSampler : register(s2);

Texture2D	gRoughnessTexture : register(t3);
SamplerState gRoughnessSampler : register(s3);

TextureCube gEnvTexture : register(t4);
SamplerState gEnvSampler : register(s4);

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
struct a2v
{
	float4 Position : POSITION;
	float3 Normal : NORMAL;
	float2 UV		: TEXCOORD;
	float3 Tangent : TANGENT;
};

struct v2p 
{
	float4 Position   : SV_Position;
	float3 Normal : TEXCOORD0;
	float3 Tangent : TEXCOORD1;
	float3 Binormal : TEXCOORD2;
	float2 UV		: TEXCOORD3;
	float3 WorldPos : TEXCOORD4;
};

v2p meshpbr_VertexShader( in a2v INPUT )
{
    v2p OUTPUT;

	OUTPUT.Position = mul( gWorldViewProj, INPUT.Position );
	OUTPUT.Normal = normalize(mul((float3x3)gWorld, INPUT.Normal));
	OUTPUT.UV = INPUT.UV;
	OUTPUT.Tangent = mul((float3x3)gWorld, INPUT.Tangent);
	OUTPUT.Binormal= cross(OUTPUT.Tangent, OUTPUT.Normal);
	OUTPUT.WorldPos = mul(gWorld, INPUT.Position).xyz;

	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 meshpbr_PixelShader( in v2p INPUT ) : SV_Target
{	
	// process normal map.
	float3 baseColor = gDiffuseTexture.Sample(gDiffuseSampler, INPUT.UV).xyz;
	float metallic = gMetallicTexture.Sample(gMetallicSampler, INPUT.UV).r;	
	float roughness = gRoughnessTexture.Sample(gRoughnessSampler, INPUT.UV).r;
#ifdef NORMAL_TEXTURE
	float3 normalT = gNormalTexture.Sample(gNormalSampler, INPUT.UV).xyz;
#endif //NORMAL_TEXTURE
	
	float3 normal = INPUT.Normal;
#ifdef NORMAL_TEXTURE
	if (length(normalT)>0.0001)
	{
		normalT = fixNormalSample(normalT);
	    float3 normalMapWS = normalT.x*INPUT.Tangent + normalT.y*INPUT.Binormal;
		normal = normalize(normal + normalMapWS);
	}  
#endif //NORMAL_TEXTURE

	// shading	
	float3 lightColor = gDirectionalLightDiffuse.xyz * gDirectionalLightDir_Intensity.w;
	float3 toLightDir = gDirectionalLightDir_Intensity.xyz;
	float3 toViewDir = normalize(gCameraPos - INPUT.WorldPos);

	const float3 dielectricColor = float3(0.04, 0.04, 0.04);
	const float minRoughness = 1e-4;
	roughness = max(minRoughness, roughness);
	float3 diffColor = baseColor * (1.0 - metallic);
	float3 specColor = lerp(dielectricColor, baseColor, metallic);	

	float ndl = saturate(dot(normal, toLightDir));
	float3 h = (toViewDir + toLightDir) * .5f;
	float ndh = saturate(dot(normal, h));
	float vdh = max( 1e-8, abs(dot(toViewDir, h)) );
	float ndv = max( 1e-8, abs(dot( normal, toViewDir)) );
	float3 shadedColor = ndl * lightColor * (diffColor + CookTorrance(ndl, vdh, ndh, ndv, specColor, roughness));
	float3 envContrib = {0, 0, 0};
	
	// need to work further.
#ifdef ENV_TEXTURE
	float3 reflect = normalize(-(toViewDir - 2 * normal * dot(toViewDir, normal)));
	reflect.yz = -reflect.yz;
	envContrib += gEnvTexture.SampleLevel(gEnvSampler, reflect, 1) * 
		CookTorranceContrib(vdh, ndh, ndl, ndv, specColor, roughness);
#else
	float3 reflect = normalize(-(toViewDir - 2 * normal * dot(toViewDir, normal)));
	float3 starColor = Star(reflect, float3(gWorld[0][3], gWorld[1][3], gWorld[2][3]));
	envContrib += starColor * 
		CookTorranceContrib(vdh, ndh, ndl, ndv, specColor, roughness);
#endif
	


	return float4(shadedColor+envContrib, 1.0);
}