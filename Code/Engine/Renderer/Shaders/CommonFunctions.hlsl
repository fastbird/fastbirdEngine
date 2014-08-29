#define INV_PI 0.31830988618379067153776752674503
#define TWO_PI 6.283185307179586476925286766559
#define PI 3.1415926535897932384626433832795
#define INV_LOG2 1.4426950408889634073599246810019
#define ENV_MAX_LOD 11.0
#define ENV_SAMPLES 16
#ifdef ENV_TEXTURE
TextureCube gEnvTexture : register(t4);
SamplerState gEnvSampler : register(s4);
#endif

Texture2D gDepthMap : register(t5);
SamplerState gDepthSampler : register(s5);

Texture2D gFogMap : register(t6);
SamplerState gFogSampler : register(s6);

Texture2D gNoiseMap : register(t7);
SamplerState gNoiseSampler : register(s7);

float2 SphericalCoord(float3 dir)
{
	float n = length(dir.xy);
	float x_xyPlane = (n > 0.000001) ? dir.x / n : 0;
	float2 uv;
	uv.x = acos(x_xyPlane)*INV_PI*.5;
	uv.x = (dir.y > 0.0) ? uv.x : 1.0-uv.x;
	uv.y = 1.0 - (atan(dir.z / n) * INV_PI+ 0.5);
	//uv.y = 1.0f - (dir.z*.5 + .5);
	
	return uv;
}

float3 fade(float3 t)
{
  return t * t * t * (t * (t * 6 - 15) + 10); // new curve
  //return t * t * (3 - 2 * t); // old curve
}

cbuffer cbImmutable
{
	static const float3 gGradient[16] =
    {
		{1, 1, 0},
	{-1, 1, 0},
	{1, -1, 0},
	{-1, -1, 0},

	{1, 0, 1},
	{-1, 0, 1},
	{1, 0, -1},
	{-1, 0, -1},

	{0, 1, 1},
	{0, -1, 1},
	{0, 1, -1},
	{0, -1, -1},
    
	{1, 1, 0},
	{0, -1, 1},
	{-1, 1, 0},
	{0, -1, -1}
	};
	
	static const float gPermutation[512] = 
	{
	151, 160, 137, 91, 90, 15,
131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180,
151, 160, 137, 91, 90, 15,
131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
	};
}

float perm(float x)
{
  return gPermutation[x];
}



float grad(float x, float3 p)
{
  return dot(gGradient[fmod(x, 16)], p);
}

float inoise(float3 p)
{	
	float3 fp = floor(p);
	float3 P_GRID = fmod(fp, 256.0);
	p -= fp;
	float3 f = fade(p);
  
  // HASH COORDINATES FOR 6 OF THE 8 CUBE CORNERS
  float A = perm(P_GRID.x) + P_GRID.y;
  float AA = perm(A) + P_GRID.z;
  float AB = perm(A + 1) + P_GRID.z;
  float B =  perm(P_GRID.x + 1) + P_GRID.y;
  float BA = perm(B) + P_GRID.z;
  float BB = perm(B + 1) + P_GRID.z;
  
  // AND ADD BLENDED RESULTS FROM 8 CORNERS OF CUBE
  float AABA = lerp(grad(perm(AA), p),                                             grad(perm(BA), p + float3(-1, 0, 0)),           f.x);
  float ABBB = lerp(grad(perm(AB), p + float3(0, -1, 0)),           grad(perm(BB), p + float3(-1, -1, 0)),         f.x);
  float AABA_ABBB = lerp(AABA, ABBB, f.y);
  //return perm(AA)/256;
  
  float AABA_1 = lerp(grad(perm(AA + 1), p + float3(0, 0, -1)),  grad(perm(BA + 1), p + float3(-1, 0, -1)),   f.x);
  float ABBB_1 = lerp(grad(perm(AB + 1), p + float3(0, -1, -1)),grad(perm(BB + 1), p + float3(-1, -1, -1)), f.x);
  float AABA_ABBB_1 = lerp(AABA_1, ABBB_1, f.y);
  
  return lerp(AABA_ABBB, AABA_ABBB_1, f.z);
}

float3 fixNormalSample(float3 v)
{
	float3 res = (v - float3(0.5,0.5,0.5))*2.0;
	return res;
}

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
// Star Nest originally developed by Pablo Roman Andrioli
// This content is under the MIT License.
static const float tile = 0.85;
/*static const int volsteps=6;
static const int iterations=15;
static const float formuparam=0.634;
static const float brightness = 0.000005;
static const float distFading=0.53;
static const float stepsize = 0.30;
static const float saturation=0.95;*/
float3 StarNest(float3 dir, float3 cameraPos)
{
	float volsteps = gMaterialParam[2].x;
	float iterations = gMaterialParam[2].y;
	float formuparam = gMaterialParam[2].z;
	float brightness = gMaterialParam[2].w;
	float distFading = gMaterialParam[3].x;
	float stepsize = gMaterialParam[3].y;
	float saturation = gMaterialParam[3].z;
	float3 finalColor = 0;

	float s = 0.0;
	float fade = 1.0;
	float3 v = {0, 0, 0};
	for (int r=0; r<volsteps; r++)
	{
		float3 p = cameraPos*0.0001 + s * dir * .5f;
		p =abs( tile - fmod(p, (tile*2)) );
		float pa=0, a=0;
		for (int i=0; i<iterations; i++)
		{
			p= abs(p) / dot(p,p) - formuparam;
			float lp = length(p);
			a += abs(lp-pa);
			pa=lp;
		}
		a*=a*a;
		v+=float3(s, s*s, s*s*s*s) * a * brightness * fade;
		fade *= distFading;
		s+=stepsize;
	}
	v=lerp(length(v),v,saturation); //color adjust
	finalColor += v;

	return finalColor;

}

float3 GetFoggedColor(float2 uv, float3 incidentColor, float3 worldPos)
{
	float depth = gDepthMap.Sample(gDepthSampler, uv);
	float4 volumeFog  = gFogMap.Sample(gFogSampler, uv);
	float start = volumeFog.x;
	float end = volumeFog.y;	
	end += volumeFog.b*volumeFog.a;
	
	float volumeFogDensity = (end - start);
	//return volumeFogDensity.xxx;
	
	//return volumeFogDensity.xxx;
	float p = volumeFogDensity*volumeFogDensity*gFogColor.w;
	float f = 1.0/(pow(2.71828, p));
	//return f.xxx;
	//float noise = inoise(worldPos*10);
	//return noise.xxx;
	float3 foggedColor = f * incidentColor + (1.0-f) * gFogColor.rgb;
	return foggedColor;
}

float2 GetScreenUV(float2 screenxy)
{
	float2 uv = screenxy / gScreenSize.xy;
	return uv;
}