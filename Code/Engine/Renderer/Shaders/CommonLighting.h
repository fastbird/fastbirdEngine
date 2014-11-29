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
	float sphg = pow(2.0, (-5.55473*vdh - 6.98316) * vdh);
	return F0 + (float3(1.0, 1.0, 1.0) - F0) * sphg;
}

// Normal Distrubution function (NDF) = Specular D
// Disney's GGX/Trowbridge-Reitz
float NormalDistrib(float ndh, float roughness)
{
	float alpha = roughness * roughness;
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

float Visibility(float ndl, float ndv, float roughness)
{
	// visibility is a Cook-Torrance geometry function divided by (n.l)*(n.v)
	float k = roughness * roughness * 0.5;
	return G1(ndl,k)*G1(ndv,k);
}

// Cook-Torrance microfacet specular shading model
float3 CookTorrance(float ndl, float vdh, float ndh, float ndv, float3 Ks, float roughness)
{	
// reference : http://www.filmicworlds.com/2014/04/21/optimizing-ggx-shaders-with-dotlh/

          // F : Frenel
	return Fresnel(vdh,Ks)
	   
	   // D : GGX Distribution
	* ( NormalDistrib(ndh,roughness)
		
		// V : Schlick approximation of Smith solved with GGX
	* Visibility(ndl,ndv,roughness) / 4.0 );
}

float3 CookTorranceContrib(
	float vdh,
	float ndh,
	float ndl,
	float ndv,
	float3 Ks,
	float roughness)
{
// This is the contribution when using importance sampling with the GGX based
// sample distribution. This means ct_contrib = ct_brdf / ggx_probability
	return Fresnel(vdh,Ks) * (Visibility(ndl,ndv,roughness) * vdh * ndl / ndh );
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
	
	for (int y=-1; y<=1; ++y)
	{
		for (int x = -1; x<=1; ++x)
		{
			c += gShadowMap.SampleCmp(gShadowSampler, uv.xy, lightPos.z-0.0005, float2(x, y));
		}
	}
	return 0.5 + c * 0.0555556;
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