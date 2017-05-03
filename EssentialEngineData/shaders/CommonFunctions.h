/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/
#ifndef __COMMON_FUNCTIONS_H_
#define __COMMON_FUNCTIONS_H_

#include "EssentialEngineData/shaders/CommonDefines.h"
#include "EssentialEngineData/shaders/Constants.h"

#ifdef ENV_TEXTURE
TextureCube gEnvTexture : register(t4);
//Texture2D gEnvTexture : register(t4);
#endif

Texture2D gDepthMap : register(t5);

Texture2D gFogMap : register(t6);

Texture2D gNoiseMap : register(t7);

Texture1D<unsigned int> gPermutationMap : register(t10);
Texture1D gGradiantsMap : register(t11);
Texture1D<float> gValueNoiseMap : register(t12);


float turbulence(float3 p, int octaves, float lacunarity = 2.0, float gain = 0.5);

float2 SphericalCoord(float3 dir)
{
	float radius = length(dir);
	float theta = 1 - (asin(dir.z / radius) + HALF_PI) * INV_PI;
	float phi = 0.f;
	phi = atan2(dir.y, dir.x)*INV_PI * .5f + .5f;	
	
	return float2(phi, theta);
}

float2 SphericalCoord2(float3 dir)
{	
	float u, v;
	if (dir.x == 0 && dir.y == 0)
		u = 0;
	else
		u = atan2(dir.y, dir.x) / (2*PI) + 0.5;
	
	float len = length(dir.xy);
	if (dir.z == 0 && len == 0)
		v = 0;
	else
		v = atan2(dir.z, len) / PI + 0.5;	

	return float2(u, v);
}

float3 GeodeticToSpherical(float lat, float lon){
	float cosLat = cos(lat);
	
	return float3(cos(lon) * cosLat, sin(lon) * cosLat, sin(lat));
}

#ifdef ENV_TEXTURE
float3 SampleEnvironmentMap(float3 dir, float lod)
{
	float3 color = gEnvTexture.SampleLevel(gAnisotropicSampler, dir, lod).rgb *2.0;
	return color;
}
#endif

float3 fixNormalSample(float3 v)
{
	float3 res;
	res	= (v - 0.5f) * 2.f;	
	res.z = sqrt(1.0f - min(1, (res.x*res.x + res.y*res.y)));	
	//res.y = -res.y;
	return res;
}

float mod(float x, float y)
{
  return x - y * floor(x/y);
}

float3 mod(float3 x, float3 y)
{
  return x - y * floor(x/y);
}

float mod2(float a, float b) {
	int n = (int)(a / b);
	a -= n*b;
	if (a < 0)
		a += b;
	return a;
}

#define CR00 -0.5
#define CR01 1.5
#define CR02 -1.5
#define CR03 0.5
#define CR10 1.0
#define CR11 -2.5
#define CR12 2.0
#define CR13 -0.5
#define CR20 -0.5
#define CR21 0.0
#define CR22 0.5
#define CR23 0.0
#define CR30 0.0
#define CR31 1.0
#define CR32 0.0
#define CR33 0.0

float spline(float x, float4 knot){
	x = clamp(x, 0, 1);	
	float4 c = mul(float4x4(CR00, CR01, CR02, CR03,
							CR10, CR11, CR12, CR13,
							CR20, CR21, CR22, CR23,
							CR30, CR31, CR32, CR33), knot);
	return ((c.x * x + c.y)*x + c.z)*x + c.w;
	
}
float3 GetFoggedColor(float2 uv, float3 incidentColor, float3 worldPos)
{	
  return incidentColor;
	// float4 volumeFog  = gFogMap.Sample(gPointSampler, uv);
	// float start = volumeFog.x;
	// float end = volumeFog.y;	
	// end += volumeFog.b*volumeFog.a;
	
	// float volumeFogDensity = (end - start);  
	// volumeFogDensity = volumeFogDensity * gFogColor.w * turbulence(worldPos * 0.35f + float3(gTime*0.02f, 0, 0), 2);	
	
	// float p = volumeFogDensity*volumeFogDensity;
	// float f = 1.0/(pow(2.71828, p));	
	// float3 foggedColor = f * incidentColor + (1.0-f) * gFogColor.rgb;
	// return foggedColor;
}

float2 GetScreenUV(float2 screenxy)
{
	float2 uv = screenxy / gScreenSize.xy;
	return uv;
}

float3 srgb_to_linear(float3 c)
{
    return pow(c,float3(2.2,2.2,2.2));
}

float4 srgb_to_linear(float4 c)
{
    return float4(pow(c.rgb,float3(2.2,2.2,2.2)), c.a);
}

float SymetricCurve(float x)
{
	static const float ContrastPower = 4;
	return 0.5f * pow(saturate(2*((x>0.5) ? 1-x : x)), ContrastPower);
}

static const float3 GradientBuffer[] = {
	{ 1, 1, 0}, { -1, 1, 0}, { 1, -1, 0}, { -1, -1, 0},
	{ 1, 0, 1}, { -1, 0, 1}, { 1, 0, -1},{ -1, 0, -1},
	{ 0, 1, 1}, { 0, -1, 1}, { 0, 1, -1}, { 0, -1, -1},	
	{ -1, 1, 0}, { 0, -1, 1}, { 1, 1, 0}, { 0, -1, -1},	
};

static const unsigned int PermutationBuffer[] = {
	170,173,207,213,176,202,39,255,216,157,
	20,77,244,121,39,89,66,124,156,193,
	248,205,140,135,32,35,100,184,166,135,
	149,76,176,90,141,78,45,153,231,61,
	177,96,222,177,128,173,8,65,233,103,
	65,165,213,159,228,24,159,21,66,0,
	38,254,76,209,33,4,147,47,179,143,
	115,83,64,67,138,175,126,202,111,213,
	207,211,161,149,206,90,190,101,39,42,
	246,7,173,161,190,101,166,180,201,192,
	105,50,52,9,44,77,1,143,23,86,
	198,219,157,200,166,216,11,136,129,56,
	97,107,104,18,98,249,84,208,231,113,
	23,72,120,13,146,41,29,134,41,153,
	114,219,116,28,250,79,55,184,181,176,
	149,87,245,223,128,108,109,141,116,217,
	139,67,101,17,8,165,246,121,189,247,
	235,21,184,224,225,96,143,110,60,123,
	244,91,98,138,138,143,39,92,247,229,
	135,74,59,50,155,97,64,132,198,195,
	177,167,48,74,16,238,117,111,3,47,
	158,106,239,16,80,155,200,129,67,41,
	40,138,246,233,158,71,161,129,72,49,
	108,205,164,158,222,129,163,140,152,16,
	255,154,67,205,207,87,199,80,89,191,
	189,28,39,3,40,127,
};

float3 fade(float3 t)
{
  return t * t * t * (t * (t * 6 - 15) + 10); // new curve
  //return t * t * (3 - 2 * t); // old curve
}
	
float PERM(int x)
{
	return gPermutationMap.Load(int2(x&PERM_MASK, 0));
	//return PermutationBuffer[x];
}

float ValueNoiseTable(int x){	
	return gValueNoiseMap[x];
}

float grad(float x, float3 p)
{
  return dot(GradientBuffer[fmod(x, 16)].xyz, p);
  //return dot(gGradiantsMap.Load(int2(x, 0)), p);
}

float inoise(float3 p)
{	
	float3 fp = floor(p);
	float3 P_GRID = fmod(fp, 256);
	p -= fp;
	float3 f = fade(p);  
	// HASH COORDINATES FOR 6 OF THE 8 CUBE CORNERS
	float A = PERM(P_GRID.x) + P_GRID.y;
	float AA = PERM(A) + P_GRID.z;
	float AB = PERM(A + 1) + P_GRID.z;
	float B =  PERM(P_GRID.x + 1) + P_GRID.y;
	float BA = PERM(B) + P_GRID.z;
	float BB = PERM(B + 1) + P_GRID.z;

	// AND ADD BLENDED RESULTS FROM 8 CORNERS OF CUBE
	float AABA = lerp(grad(PERM(AA), p), grad(PERM(BA), p + float3(-1, 0, 0)), f.x);
	float ABBB = lerp(grad(PERM(AB), p + float3(0, -1, 0)), grad(PERM(BB), p + float3(-1, -1, 0)), f.x);
	float AABA_ABBB = lerp(AABA, ABBB, f.y);  

	float AABA_1 = lerp(grad(PERM(AA + 1), p + float3(0, 0, -1)), grad(PERM(BA + 1), p + float3(-1, 0, -1)), f.x);
	float ABBB_1 = lerp(grad(PERM(AB + 1), p + float3(0, -1, -1)), grad(PERM(BB + 1), p + float3(-1, -1, -1)), f.x);
	float AABA_ABBB_1 = lerp(AABA_1, ABBB_1, f.y);

	return lerp(AABA_ABBB, AABA_ABBB_1, f.z);
}
float noise3(float3 p){	
	int b00, b10, b01, b11;
	
	float3 t = p * 10000;
	int3 b0 = ((int3)t) & TABMASK;
	int3 b1 = (b0+1) & TABMASK;
	float3 r0 = t - (int3)t;
	float3 r1 = r0 - 1.;
}

float hash( float n ) { return frac(sin(n)*753.5453123); }
float hnoise( float3 x )
{
    float3 p = floor(x);
    float3 f = frac(x);
    f = f*f*(3.0-2.0*f);
	
    float n = p.x + p.y*157.0 + 113.0*p.z;
    return lerp(lerp(lerp( hash(n+  0.0), hash(n+  1.0),f.x),
                   lerp( hash(n+157.0), hash(n+158.0),f.x),f.y),
               lerp(lerp( hash(n+113.0), hash(n+114.0),f.x),
                   lerp( hash(n+270.0), hash(n+271.0),f.x),f.y),f.z);
}

#define INDEX(p) PERM((p.x) + PERM((p.y) + PERM(p.z)))
float vlattice(int3 p){
	return ValueNoiseTable(INDEX(p));
}

float vnoise(float3 p){
	int3 ip;	
	float3 premain;
	// x, y, z knots
	float4 xknots;
	float4 yknots;
	float4 zknots;	
	ip = floor(p);
	premain = p - ip;
	for(int k = -1; k<=2; ++k){
		for (int j=-1; j<=2; ++j){
			for (int i=-1; i<=2; ++i){
				xknots[i+1] = vlattice(ip + int3(i, j, k));
			}
			yknots[j+1] = spline(premain.x, xknots);
		}
		zknots[k+1] = spline(premain.y, yknots);
	}
	return spline(premain.z, zknots);
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

float4 SmoothCurve( float4 x ) 
{
	return x * x *( 3.0 - 2.0 * x );
}  

float4 TriangleWave( float4 x )
{
	return abs( frac( x + 0.5 ) * 2.0 - 1.0 ); 
}  

float4 SmoothTriangleWave( float4 x )
{
	return SmoothCurve( TriangleWave( x ) );
}  


float SmoothCurve( float x ) 
{
	return x * x *( 3.0 - 2.0 * x );
}  

float TriangleWave( float x )
{
	return abs( frac( x + 0.5 ) * 2.0 - 1.0 ); 
}  

float SmoothTriangleWave( float x )
{
	return SmoothCurve( TriangleWave( x ) );
}

float turbulence(float3 p, int octaves, float lacunarity, float gain)
{
    float sum = 0;
    float freq = 1.0, amp = 1.0;
    for (int i = 0; i<octaves; i++) {
        sum += abs(inoise(p*freq))*amp;
        freq *= lacunarity;
        amp *= gain;
    }
    return sum;
}

// float fBm(float3 p, int octaves, float lacunarity = 2.0, float gain = 0.5)
// {
    // float sum = 0;
    // float freq = 1.0, amp = 1.0;
    // for (int i = 0; i<octaves; i++) {
        // sum += inoise(p*freq)*amp;
        // freq *= lacunarity;
        // amp *= gain;
    // }
    // return sum;
// }

/// Fractional Brownian motion
/// \param H the fractal increment parameter
/// \param lacunarity the gap between successive frequencies
/// \octaves the number of frequencies in the fBm

float fBm(float3 p, float octaves, float lacunarity=2, float H=0.5)
{
	float value = 0;	
	for (int i = 0; i<octaves; ++i){
		value += inoise(p) * pow(lacunarity, -H*i);
		p *= lacunarity;
	}	
	// float remainder = octaves - (int)octaves;
	// if (remainder)
		// value += remainder * inoise(p) * pow(lacunarity, -H*i);	
	return value;	
}

float multifractal(float3 p, int octaves, float lacunarity=2, float H=0.1, float offset=0.9){
	float value = 1.0;
	for (int i = 0; i<octaves; ++i){
		value *= (inoise(p) + offset) * pow(lacunarity, -H*i);
		p *= lacunarity;
	}
	return value;
}

float3 VecNoise(float3 p){
	float3 result;
	result.x = inoise(p);
	result.y = inoise(p+3.33);
	result.z = inoise(p+7.77);
	return result;
}
float DistNoise(float3 p, float distortion)
{
	return inoise(p + VecNoise(p  + 0.5) * distortion);
}

float4 Interpolate2D(Texture2D tex, float2 uv, int2 textureSize){
	const float x = uv.x;
	const float y = uv.y;	
	float fX = x*(textureSize.x-1);
	float fY = y*(textureSize.y-1);
	int nX = min(textureSize.x-2, max(0, (int)fX));
	int nY = min(textureSize.y-2, max(0, (int)fY));
	float fRatioX = fX - nX;
	float fRatioY = fY - nY;	
	return	tex.Load(int3(nX, nY, 0)) * (1-fRatioX) * (1-fRatioY) +
				tex.Load(int3(nX+1, nY, 0)) * (fRatioX) * (1-fRatioY) +
				tex.Load(int3(nX, nY+1, 0)) * (1-fRatioX) * (fRatioY) +
				tex.Load(int3(nX+1, nY+1, 0)) * (fRatioX) * (fRatioY);
}

float4 Interpolate1D(Texture1D tex, float u, int textureSize){
	// textureSize should be power of two	
	int mask = textureSize - 1;
	const float x = u;	
	float fX = x*mask;	
	int nX = (int)fX;
	float fRatioX = fX - nX;
	return	lerp(tex.Load(int2(nX&mask, 0)), 
				tex.Load(int2((nX+1) & mask, 0)), 
				fRatioX);
}

#endif
