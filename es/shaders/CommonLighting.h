#include "CommonDefines.h"

// using same slot : 5
Texture2D gShadowMap : register(t8);

void BlinnPhongShading(float3 lightColor, float3 normal, float3 toLight,
    float3 toView, float3 specularT, out float3 diffuse, out float3 specular)
{
    float3 h = normalize(toView + toLight);
    float ldn= max(dot(toLight, normal), 1e-8);
	diffuse = lightColor * ldn;
	
    float ndh = max(dot(normal,h), 1e-8);
    float specularPow = exp2(specularT*11.0 + 2.0);
    float specularNorm = (specularPow+8.0) / 8.0;    
    specular = lightColor * (specularNorm * pow(ndh, specularPow) * 0.04 * ldn);
}


// Specular F
// referenct :  http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
float3 Fresnel(	float vdh, float3 F0)
{
	//float sphg = pow(2.0, (-5.55473*vdh - 6.98316) * vdh);
	float sphg = pow(1.0f - vdh, 5);
	return F0 + (float3(1.0, 1.0, 1.0) - F0) * sphg;
}

// Normal Distrubution function (NDF) = Specular D
// Disney's GGX/Trowbridge-Reitz
float NormalDistrib(float ndh, float alpha)
{
	float tmp = alpha / (ndh*ndh*(alpha*alpha-1.0)+1.0);
	return tmp * tmp * INV_PI;
}

// Specular G
// specular geometric attenuation term based on Schlick model with different k.
// w is either toLight or toView
float G1(float ndw, float k)
{	
	return 1.0 / ( ndw*(1.0-k) + k ); // k > 0
}

// float Visibility(float ndl, float ndv, float alpha)
// {
	// // visibility is a Cook-Torrance geometry function divided by (n.l)*(n.v)
	// float k = alpha * 0.5;
	// return G1(ndl,k)*G1(ndv,k);
// }

// reference : http://www.filmicworlds.com/2014/04/21/optimizing-ggx-shaders-with-dotlh/
// ldh == vdh
float NewVisibility(float ldh, float alpha){
	float k = alpha * 0.5;
	float k2 = k*k;
	float invK2 = 1.0f - k2;
	return rcp(ldh*ldh*invK2 + k2);
}

// Cook-Torrance microfacet specular shading model
float3 CookTorrance(float vdh, float ndh, float3 Ks, float roughness)
{	
// reference : http://www.filmicworlds.com/2014/04/21/optimizing-ggx-shaders-with-dotlh/

	float alpha = roughness * roughness;
          // F : Frenel
	return Fresnel(vdh,Ks)
	   
	   // D : GGX Distribution
	* ( NormalDistrib(ndh,alpha)
		
		// V : Schlick approximation of Smith solved with GGX
	* NewVisibility(vdh,alpha) * 0.25);
}

float3 CookTorranceContrib(
	float vdh,
	float ndh,
	float ndl,
	float3 Ks,
	float roughness)
{
// This is the contribution when using importance sampling with the GGX based
// sample distribution. This means ct_contrib = ct_brdf / ggx_probability
	return Fresnel(vdh,Ks) * (NewVisibility(vdh,roughness) * vdh * ndl / ndh );
}

vec3 ImportanceSampleLambert(vec2 hamOffset, vec3 tan, vec3 bi, vec3 normal)
{
	float cosT = sqrt(hamOffset.y);
	float sinT = sqrt(1.0 - hamOffset.y);
	float phi = 2.0*PI*hamOffset.x;
	return (sinT*cos(phi)) * tan + (sinT*sin(phi)) * bi + cosT * normal;
}

float ProbabilityLambert(vec3 Ln, vec3 Nn)
{
	return max(0.0, dot(Nn, Ln) * INV_PI);
}

float Distortion(vec3 Wn)
{
	// Computes the inverse of the solid angle of the (differential) pixel in
	// the environment map pointed at by Wn
	float sinT = max(0.0000001, sqrt(1.0 - Wn.y*Wn.y));
	return 1.0 / sinT;
}

float ComputeLOD(vec3 Ln, float p)
{
	return max(0.0, (ENV_MAX_LOD - 1.5) - 0.5 * (log(float(ENV_SAMPLES)) + log(p * Distortion(Ln))) * INV_LOG2);
}

vec3 ImportanceSampleGGX(vec2 hamOffset, vec3 tan, vec3 bi, vec3 normal, float roughness)
{
	float a = roughness*roughness;
	float cosT = sqrt((1.0 - hamOffset.y) / (1.0 + (a*a - 1.0)*hamOffset.y));
	float sinT = sqrt(1.0 - cosT*cosT);
	float phi = 2.0*PI*hamOffset.x;
	return (sinT*cos(phi)) * tan + (sinT*sin(phi)) * bi + cosT * normal;
}

float ProbabilityGGX(float ndh, float vdh, float Roughness)
{
	return NormalDistrib(ndh, Roughness) * ndh / (4.0*vdh);
}


float GetShadow(float4 lightPos)
{
	
	lightPos.xyz /= lightPos.w;
	float2 uv;
	uv = lightPos * float2(0.5, -0.5) + 0.5;
	
	float c = 0.0;
	if (lightPos.z>=1.0f)
		return 1.0f;
	
	for (int y=-2; y<=2; ++y)
	{
		for (int x = -2; x<=2; ++x)
		{
			c += gShadowMap.SampleCmp(gShadowSampler, uv.xy, lightPos.z-0.001, float2(x, y));
		}
	}
	return min(0.5f + c * 0.02f, 1.0f);
}

float3 GetIrrad(float4 vNormal)
{
	//Ramamoorthi; Hanrahan, An Efficient Representation for Irradiance Environment Maps, 2001
	float c1 = 0.429043f, c2 = 0.511664f, c3 = 0.743125f,
		c4 = 0.886227f, c5 = 0.247708f;
		
	float4 ret = c1 * gIrradConstsnts[8] * (vNormal.x*vNormal.x - vNormal.y*vNormal.y) + c3 * gIrradConstsnts[6] * vNormal.z*vNormal.z + 
				c4 * gIrradConstsnts[0] - c5 * gIrradConstsnts[6] 
				+ 2.0 * c1 * (gIrradConstsnts[4]*(vNormal.x*vNormal.y) + gIrradConstsnts[7] * vNormal.x * vNormal.z + gIrradConstsnts[5] * vNormal.y * vNormal.z)
				+ 2.0 * c2 * (gIrradConstsnts[3] * vNormal.x + gIrradConstsnts[1] * vNormal.y + gIrradConstsnts[2] * vNormal.z);
				
	return ret.rgb;
				
	// Sloan, Stupid Spherical Harmonics(SH) Tricks, 2008
	// float3 x1, x2, x3;
	// // Linear + constant polynomial terms
	// x1.r = dot(gIrradConstsnts[0],vNormal);
	// x1.g = dot(gIrradConstsnts[1],vNormal);
	// x1.b = dot(gIrradConstsnts[2],vNormal);
	
	// // 4 of the quadratic polynomials
	// float4 vB = vNormal.xyzz * vNormal.yzzx;
	// x2.r = dot(gIrradConstsnts[3],vB);
	// x2.g = dot(gIrradConstsnts[4],vB);
	// x2.b = dot(gIrradConstsnts[5],vB);
	
	// // Final quadratic polynomial
	// float vC = vNormal.x*vNormal.x - vNormal.y*vNormal.y;
	// x3 = gIrradConstsnts[6].rgb * vC;
	
	// return x1+x2+x3;
}

float3 CalcEnvContrib(float3 normal, float3 tangent, float3 binormal, float roughness, float3 toViewDir, float3 diffColor, float3 specColor)
{
	float3 envContrib = {0, 0, 0};
#ifdef ENV_TEXTURE
	vec3 Tp = normalize(tangent	- normal*dot(tangent, normal)); // local tangent
	vec3 Bp = normalize(cross(tangent, normal));
		
	static float2 hammersley[16] = (float2[16])gHammersley;
	for(int i=0; i<ENV_SAMPLES; ++i)
	{
		vec2 Xi = hammersley[i];
		vec3 Sd = ImportanceSampleLambert(Xi,Tp,Bp,normal);
		float pdfD = ProbabilityLambert(Sd, normal);
		float lodD = ComputeLOD(Sd, pdfD);	
		
		envContrib +=	SampleEnvironmentMap(Sd, lodD) * diffColor;
		vec3 Hn = ImportanceSampleGGX(Xi,Tp,Bp,normal,roughness);
		
		float3 Ln = -reflect(toViewDir, Hn);
		Ln.yz = Ln.zy;

		float ndl = dot(normal, Ln);

		// Horizon fading trick from http://marmosetco.tumblr.com/post/81245981087
		const float horizonFade = 1.3;
		float horiz = clamp( 1.0 + horizonFade * ndl, 0.0, 1.0 );
		horiz *= horiz;
		ndl = max( 0.05, abs(ndl) );

		float vdh = max( 1e-8, dot(toViewDir, Hn) );
		float ndh = max( 1e-8, dot(normal, Hn) );
		float lodS = roughness < 0.01 ? 0.0 : ComputeLOD(Ln,
			ProbabilityGGX(ndh, vdh, roughness));
		envContrib += SampleEnvironmentMap(Ln, lodS)
			  * CookTorranceContrib(vdh, ndh, ndl, specColor, roughness) * horiz;
	}

	envContrib /= ENV_SAMPLES;
	return envContrib;
#else
	return envContrib;
#endif
}

float3 CalcPointLights(float3 worldPos, float3 normal)
{
	float3 result={0, 0, 0};
	int count = (int)gPointLightColor[0].w;
	count = min(3, count);
	for(int i=0; i<count; i++)
	{
		
		float lightReach = gPointLightPos[i].w;
		float3 toLight = gPointLightPos[i].xyz - worldPos;
		float dist = length(toLight);
		toLight = toLight / dist;
		float atten = clamp(1.0 - dist/lightReach, 0, 1);
		atten *= atten;
		float d = max(0, dot(toLight, normal));
		result += (atten * d) * gPointLightColor[i].xyz;
	}
	return result;
}