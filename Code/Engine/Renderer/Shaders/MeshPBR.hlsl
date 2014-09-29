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
	OUTPUT.Tangent = mul((float3x3)gWorld, INPUT.Tangent);
	OUTPUT.Binormal= cross(OUTPUT.Tangent, OUTPUT.Normal);
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
	float3 baseColor = gDiffuseTexture.Sample(gLinearSampler, INPUT.UV).xyz;
	float metallic = gMetallicTexture.Sample(gLinearSampler, INPUT.UV).r;	
	float roughness = gRoughnessTexture.Sample(gLinearSampler, INPUT.UV).r;
#ifdef NORMAL_TEXTURE
	float3 normalT = gNormalTexture.Sample(gPointSampler, INPUT.UV).xyz;
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
	float3 diffColor = baseColor * (1.0 - metallic*.9);
	float3 specColor = lerp(dielectricColor, baseColor, metallic);	

	float ndl = max(dot(normal, toLightDir), 1e-8);
	float3 h = (toViewDir + toLightDir) * .5f;
	float ndh = max(abs(dot(normal, h)), 1e-8);
	float vdh = max( 1e-8, abs(dot(toViewDir, h)) );
	float ndv = max( 1e-8, abs(dot( normal, toViewDir)) );
	float invShadow = GetShadow(INPUT.LightPos);
	float3 shadedColor = ndl * lightColor * (diffColor + CookTorrance(ndl, vdh, ndh, ndv, specColor, roughness));
	float3 envContrib = {0, 0, 0};
	
	// need to work further.
#ifdef ENV_TEXTURE
	vec3 ltangent = normalize(INPUT.Tangent
		- normal*dot(INPUT.Tangent, normal)); // local tangent
	vec3 lbitangent = normalize(INPUT.Binormal
		- normal*dot(INPUT.Binormal,normal)
		- ltangent*dot(INPUT.Binormal, ltangent)); // local bitangent
		
	for(int i=0; i<ENV_SAMPLES; ++i)
	{
		vec2 hamOffset = gHammersley[i];
		vec3 Sd = ImportanceSampleLambert(hamOffset,ltangent,lbitangent,normal);
		float pdfD = ProbabilityLambert(Sd, normal);
		float lodD = ComputeLOD(Sd, pdfD);
		envContrib +=gEnvTexture.SampleLevel(gAnisotropicSampler, Sd, lodD).rgb * diffColor;
		vec3 Hn = ImportanceSampleGGX(hamOffset,ltangent,lbitangent,normal,roughness);
		
		float3 Ln = normalize(-(toViewDir - 2 * Hn * dot(toViewDir, Hn)));
		Ln.yz = Ln.zy;

		float ndl = dot(normal, Ln);

		// Horizon fading trick from http://marmosetco.tumblr.com/post/81245981087
		const float horizonFade = 1.3;
		float horiz = clamp( 1.0 + horizonFade * ndl, 0.0, 1.0 );
		horiz *= horiz;
		ndl = max( 1e-8, abs(ndl) );

		float vdh = max( 1e-8, abs(dot(toViewDir, Hn)) );
		float ndh = max( 1e-8, abs(dot(normal, Hn)) );
		float lodS = roughness < 0.01 ? 0.0 : ComputeLOD(Ln,
			ProbabilityGGX(ndh, vdh, roughness));
		envContrib += gEnvTexture.SampleLevel(gAnisotropicSampler, Ln, lodS).rgb
			 * CookTorranceContrib(
				vdh, ndh, ndl, ndv,
				specColor,
				roughness) * horiz;
	}

	envContrib /= ENV_SAMPLES;
#else
	float3 reflect = normalize(-(toViewDir - 2 * normal * dot(toViewDir, normal)));
	reflect.yz = reflect.zy;
	float3 starColor = StarNest(reflect, float3(gWorld[0][3], gWorld[1][3], gWorld[2][3]));
	envContrib += starColor * 
		CookTorranceContrib(vdh, ndh, ndl, ndv, specColor, roughness);
#endif
	float2 screenUV = GetScreenUV(INPUT.Position.xy);
	float3 foggedColor = GetFoggedColor(screenUV, shadedColor + envContrib, normalize(INPUT.WorldPos) );
	return float4(foggedColor * invShadow, 1.0);
}