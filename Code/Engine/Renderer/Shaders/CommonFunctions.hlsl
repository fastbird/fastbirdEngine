#define INV_PI 0.31830988618379067153776752674503
#define TWO_PI 6.283185307179586476925286766559
#define PI 3.1415926535897932384626433832795
#define INV_LOG2 1.4426950408889634073599246810019

#define vec2 float2
#define vec3 float3
#define vec4 float4

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

float3 fixNormalSample(float3 v)
{
  float3 result = v - float3(0.5,0.5,0.5);
  return result;
}

void BlinnPhongShading(float3 lightColor, float3 normal, float3 toLight,
    float3 toView, float3 specularT, out float3 diffuse, out float3 specular)
{
    float3 h = normalize(toView + toLight);
    float ldn= saturate(dot(toLight, normal));
	diffuse = lightColor * ldn;
	
    float ndh = saturate(dot(normal,h));
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
	return Fresnel(vdh,Ks) * ( NormalDistrib(ndh,roughness) * Visibility(ndl,ndv,roughness) / 4.0 );
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

// Star Nest originally developed by Pablo Roman Andrioli
// This content is under the MIT License.
static const float tile = 0.85;
static const int volsteps=5;
static const int iterations=17;
static const float formuparam=0.53;
static const float darkmatter = 0.300;
static const float brightness = 0.000015;
static const float distFading=0.730;
static const float stepsize = 0.1;
static const float saturation=0.850;
float3 Star(float3 dir, float3 cameraPos)
{
	float3 star = {1.0, -1.0, 1.0};

	float3 color= {0.9411, 0.7059, 0.2745};
	float d = pow(saturate(dot(star, dir)-0.6), 5) * 2;
	float3 finalColor = color * d;

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
		float dm = max(0, darkmatter -  a);
		a*=a*a;
		//fade *=1.0-dm;
		//v+=float3(dm, dm*.5, 0.);
		//v+=fade;
		v+=float3(s, s*s, s*s*s*s) * a * brightness * fade;
		fade *= distFading;
		s+=stepsize;
	}
	v=lerp(length(v),v,saturation); //color adjust
	finalColor += v;

	return finalColor;

}