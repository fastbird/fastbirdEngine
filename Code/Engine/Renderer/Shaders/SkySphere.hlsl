//----------------------------------------------------------------------------
// File: skySphere.hlsl
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Constant Buffer
//----------------------------------------------------------------------------
#include "Constants.h"
#include "CommonFunctions.hlsl"

//----------------------------------------------------------------------------
// Textures
//----------------------------------------------------------------------------
//Texture2D  gEnvMap : register(t0);
//SamplerState gEnvSampler : register(s0);

//----------------------------------------------------------------------------
struct a2v
{
	float3 Position		: POSITION;
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
	float4 pos = float4(INPUT.Position, 1.0);
	pos.z = 0.9999;
	OUTPUT.Position = pos;
	float4 worldPos = mul(gInvViewProj, pos);
	worldPos.xyz /= worldPos.w;
	
	OUTPUT.WorldDir = worldPos.xyz - gCameraPos.xyz;

	return OUTPUT;
}
/*
float2 SphericalCoord2(float3 dir)
{
	float n = length(dir.xy);
	float x_xyPlane = (n > 0.000001) ? dir.x / n : 0;
	float2 uv;
	uv.x = acos(x_xyPlane)*INV_PI*.5;
	uv.x = (dir.y > 0.0) ? uv.x : 1.0-uv.x;
	uv.y = 1.0 - (atan(dir.z / n) * INV_PI+ 0.5);
	//uv.y = 1.0f - (dir.z*.5 + .5);
	
	return uv;
}*/

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

float PerlinNoise2D(float x, float y)
{
	float total = 0;
	float p = 1.0;
	float f = 1;
	float a = 1;
	for (int i=0; i<4; i++)
	{
		total += InterpolatedNoise2D(x*f, y*f) * a;
		
		f*=2.0;
		a*=p;		
	}
	return total;
}
//----------------------------------------------------------------------------
// PIXEL shader
//----------------------------------------------------------------------------
float4 skysphere_PixelShader(in v2p INPUT) : SV_Target
{
	float3 dir =normalize(INPUT.WorldDir);
	float3 starColor = StarNest(dir, gCameraPos);
	float3 star = gDirectionalLightDir_Intensity.xyz;
	float3 color= gDirectionalLightDiffuse.xyz * gDirectionalLightDir_Intensity.w;
	float d = pow(saturate(dot(star, dir)-0.6), 5) * 2;
	float3 starColor += color * d;
	
	float ringIntencity=0;
	if (dir.z>0)
	{
		ringIntencity = sin(dir.x*2)*cos(dir.y*2)*tan((dir.z)*2);
	}
	float noise = PerlinNoise2D(((dir.x + dir.z)*0.25+0.5)*2.0f, ((dir.y+dir.z)*0.25+0.5)*2.0f)*.5 +.5; //abs(PerlinNoise1D(abs(dir.x*2) + abs(dir.y*2) + abs(dir.z)));
	float fogIntencity = noise;
	
	float3 starRing = starColor*abs(1.0-ringIntencity) + ringIntencity*float3(0.01, 0.0, 0.001);
	
	return float4(lerp(starRing, vec3(0.1, 0.25, 0.25), fogIntencity), 1);
	//float2 uv = SphericalCoord2(dir);
	//return gEnvMap.Sample(gEnvSampler, uv);	
}