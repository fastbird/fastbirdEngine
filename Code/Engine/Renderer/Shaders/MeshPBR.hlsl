//--------------------------------------------------------------------------------------
// File: MeshPBR.hlsl
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "Constants.h"
#include "CommonFunctions.h"
#include "CommonLighting.h"

Texture2D  gDiffuseTexture : register(t0);
#ifdef NORMAL_TEXTURE
Texture2D  gNormalTexture : register(t1);
#endif //NORMAL_TEXTURE
Texture2D  gMetallicTexture : register(t2);  // rgb : specular color, a : roughness
Texture2D	gRoughnessTexture : register(t3);

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
	float4 LightPos : TEXCOORD5;
};

v2p meshpbr_VertexShader( in a2v INPUT )
{
    v2p OUTPUT;

	OUTPUT.Position = mul( gWorldViewProj, INPUT.Position );
	OUTPUT.Normal = normalize(mul((float3x3)gWorld, INPUT.Normal));
	OUTPUT.UV = INPUT.UV;
	OUTPUT.Tangent = normalize(mul((float3x3)gWorld, INPUT.Tangent));
	OUTPUT.Binormal= normalize(cross(OUTPUT.Tangent, OUTPUT.Normal));
	float3 worldPos = mul(gWorld, INPUT.Position).xyz;
	OUTPUT.WorldPos = worldPos;
	OUTPUT.LightPos = mul(gLightViewProj, float4(worldPos, 1.0));

	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 meshpbr_PixelShader( in v2p INPUT ) : SV_Target
{
	INPUT.UV.y = 1.0 - INPUT.UV.y;
	// process normal map.
	float3 baseColor = srgb_to_linear(gDiffuseTexture.Sample(gLinearSampler, INPUT.UV).xyz);
	float metallic = gMetallicTexture.Sample(gLinearSampler, INPUT.UV).r;	
	float roughness = gRoughnessTexture.Sample(gLinearSampler, INPUT.UV).r;
#ifdef NORMAL_TEXTURE
	float3 normalT = gNormalTexture.Sample(gPointSampler, INPUT.UV).xyz;
#endif //NORMAL_TEXTURE
	
	float3 normal = INPUT.Normal;
#ifdef NORMAL_TEXTURE
	if(length(normalT)>0.0001)
	{
		normalT = fixNormalSample(normalT);
		float3 normalMapWS = normalize(normalT.x*INPUT.Tangent + normalT.y*INPUT.Binormal + normalT.z*INPUT.Normal);
		normal = normalMapWS;
		
	}
	//return float4(abs(INPUT.Binormal), 1);
#endif //NORMAL_TEXTURE
		
	// shading	
	float3 lightColor = gDirectionalLightDiffuse[0].xyz * gDirectionalLightDir_Intensity[0].w;
	float3 lightColor2 = gDirectionalLightDiffuse[1].xyz * gDirectionalLightDir_Intensity[1].w;
	float3 toLightDir = gDirectionalLightDir_Intensity[0].xyz;
	float3 toViewDir = normalize(gCameraPos - INPUT.WorldPos);	

	const float3 dielectricColor = float3(0.04, 0.04, 0.04);
	const float minRoughness = 1e-4;
	roughness = max(minRoughness, roughness);
	float3 diffColor = baseColor * (1.0 - metallic);
	float3 specColor = lerp(dielectricColor, baseColor, metallic);	
	specColor *= gDirectionalLightSpecular[0];

	float dotNL = dot(normal, toLightDir);
	float ndl = max(dotNL, 0);
	float ndl2 = max(-dotNL, 0);
	float3 h = normalize(toViewDir + toLightDir);
	float ndh = max(abs(dot(normal, h)), 0);
	float vdh = max( abs(dot(toViewDir, h)), 0 );
	float ndv = max( abs(dot( normal, toViewDir)), 0 );
	float invShadow = GetShadow(INPUT.LightPos);
	float3 shadedColor = ndl * lightColor * (diffColor + CookTorrance(ndl, vdh, ndh, ndv, specColor, roughness));
	shadedColor +=  ndl2 * lightColor2 * baseColor * 0.3f;
	float3 irrad = GetIrrad(float4(normal, 1.0f));
	shadedColor+=irrad;	
	
	float3 envContrib = {0, 0, 0};
	
	// need to work further.
#ifdef ENV_TEXTURE
	envContrib = CalcEnvContrib(normal, INPUT.Tangent, INPUT.Binormal, roughness, toViewDir, ndv, diffColor, specColor);
#endif
	float2 screenUV = GetScreenUV(INPUT.Position.xy);
	float3 foggedColor = GetFoggedColor(screenUV, shadedColor+envContrib, normalize(INPUT.WorldPos) );
	return float4(foggedColor * invShadow, 1.0);
}