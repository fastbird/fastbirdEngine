//----------------------------------------------------------------------------
// File: skySphere.hlsl
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Constant Buffer
//----------------------------------------------------------------------------
#include "Constants.h"
#include "CommonFunctions.h"

//----------------------------------------------------------------------------
// Textures
//----------------------------------------------------------------------------
Texture2D  gPlanetMap : register(t0);
SamplerState gPlanetSampler : register(s0);
Texture2D  gPNoiseMap : register(t1);
SamplerState gPNoiseSampler : register(s1);
Texture2D  gColorRampMap : register(t2);
SamplerState gColorRampSampler : register(s2);

//----------------------------------------------------------------------------
struct a2v
{
	float4 Position		: POSITION;
	float2 UV			: TEXCOORD;
};

//----------------------------------------------------------------------------
struct v2p
{
	float4 Position		: SV_Position;
	float3 WorldDir		: TEXCOORD0;
};

//----------------------------------------------------------------------------
// VertexShader
//----------------------------------------------------------------------------
v2p skysphere_VertexShader(in a2v INPUT)
{
	v2p OUTPUT;
	float4 pos = INPUT.Position;
	pos.z = 0.9999;
	OUTPUT.Position = pos;
	float4 worldPos = mul(gInvViewProj, pos);
	worldPos.xyz /= worldPos.w;
	
	OUTPUT.WorldDir = worldPos.xyz - gCameraPos.xyz;

	return OUTPUT;
}

float Noise(int x)
{
	x = (x << 13) ^ x;
	return ( 1.0 - ( (x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}

float SmoothedNoise1D(float x)
{
	return Noise(x)/2.0 + (Noise(x-1) + Noise(x+1))/4.0;
}

float InterpolatedNoise1D(float x)
{
	int intX;
	float fractionalX = modf(x, intX);
	
	float v1 = SmoothedNoise1D(intX);
	float v2 = SmoothedNoise1D(intX+1);
	return lerp(v1, v2, fractionalX);	
}

float PerlinNoise1D(float x)
{
	float total = 0;
	float p = 0.5f;
	float f = 1;
	float a = 1;
	for (int i=0; i<4; i++)
	{
		total += InterpolatedNoise1D(x*f) * a;
		
		f*=2.0;
		a*=p;		
	}
	return total;
}

float Noise2D(int x, int y)
{
	int n = x + y * 57;
	n = (n<<13) ^ n;
	return ( 1.0 - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}

float SmoothedNoise2D(float x, float y)
{
	int intx = int(x);
	int inty = int(y);
	float corners = (Noise2D(intx - 1, inty-1) + Noise2D(intx+1, inty-1) + Noise2D(intx-1, inty+1) + Noise2D(intx+1, inty+1) ) / 16.0f;
	float sides = (Noise2D(intx-1, inty) + Noise2D(intx+1, inty) + Noise2D(intx, inty-1) + Noise2D(intx, inty+1) ) / 8.0f;
	float center = Noise2D(intx, inty) / 4.0f;
	return corners + sides + center;
}

float InterpolatedNoise2D(float x, float y)
{
	int intX;
	float fractionalX = modf(x, intX);
	
	int intY;
	float fractionalY = modf(y, intY);
	
	float v1 = SmoothedNoise2D(intX, intY);
	float v2 = SmoothedNoise2D(intX+1, intY);
	float v3 = SmoothedNoise2D(intX, intY+1);
	float v4 = SmoothedNoise2D(intX+1, intY+1);
	
	float i1 = lerp(v1, v2, fractionalX);
	float i2 = lerp(v3, v4, fractionalX);
	return lerp(i1, i2, fractionalY);
}

float PerlinNoise2D(float x, float y, int iteration, float p, float f)
{
	float total = 0;
	float a = 1;
	for (int i=0; i<iteration; i++)
	{
		total += InterpolatedNoise2D(x*f, y*f) * a;
		
		f*=2.0;
		a*=p;		
	}
	return total;
}


float2 SphericalCoord2(float3 dir)
{
	float2 uv;
	
	uv.x = 0.5 + atan2(dir.y, dir.x) / TWO_PI;
	
	uv.y = 0.5 - asin(dir.z)  * INV_PI;
	
	return uv;
}

float4 PlanetColor(float3 dir, float3 cameraPos)
{
	float2 uv = SphericalCoord2(dir);	
	float3 starDir = {0.0, -1.0, 0.0};	
	float3 seaColor= {0.058, 0.235, 0.490};
	float3 desertColor = {0.8470, 0.7450, 0.5607};
	float3 greenLandColor = {0.4470, 0.4862, 0.2549};
	float3 magmaColor = {0.8862, 0.1725, 0.1215};
	float3 cloudColor = {0.859, 0.851, 0.894};
	float d = pow(saturate(dot(starDir, dir)*2), 3);
	if (d<0.0001)
		return float4(0, 0, 0, 0);
	float mitigator = pow(d, 10);
	float seed = gPNoiseMap.Sample(gPNoiseSampler, uv);
	float4 weights = gColorRampMap.Sample(gColorRampSampler, float2(seed, .5f));	
	float3 sea = seaColor * weights.x;
	float3 greenland = greenLandColor * weights.y;
	float3 desert =desertColor * weights.z;	
	float3 magma = magmaColor * weights.w;
	float3 surfaceColor = (sea + desert + greenland + magma) * mitigator;
	float noise = gPNoiseMap.Sample(gPNoiseSampler, uv*6).x;	
	
	return float4(lerp(surfaceColor, cloudColor, noise) * d, d);	
}
//----------------------------------------------------------------------------
// PIXEL shader
//----------------------------------------------------------------------------
float4 skysphere_PixelShader(in v2p INPUT) : SV_Target
{
	float3 dir =normalize(INPUT.WorldDir);
	float3 nestColor = StarNest(dir, gCameraPos);
	float ringIntencity=0;
	if (dir.z>0)
	{
		ringIntencity = sin(dir.x*2)*cos(dir.y*2)*tan((dir.z)*2);
	}
	int iteration = 4;
	float persistence = 1.0;
	float frequency = 1.0;
	float noise = PerlinNoise2D(((dir.x + dir.z)*0.25+0.5)*2.0f, ((dir.y+dir.z)*0.25+0.5)*2.0f, iteration, persistence, frequency)*.5 +.5;
	float fogIntencity = noise;
	
	float3 starRing = nestColor*abs(1.0-ringIntencity) + ringIntencity*float3(0.01, 0.0, 0.001);
	float4 planet = PlanetColor(dir, gCameraPos);
	float planetAlpha = planet.a;
	float3 background = starRing*(1-planetAlpha)  + planet;
	float3 fogColor = vec3(0.1, 0.25, 0.25);
	
	fogIntencity = lerp(fogIntencity, 0, planetAlpha);
	return float4(lerp(background, fogColor, fogIntencity), 1);
	//float2 uv = SphericalCoord2(dir);
	//return gEnvMap.Sample(gEnvSampler, uv);	
}