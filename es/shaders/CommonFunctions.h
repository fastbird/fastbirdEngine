#include "CommonDefines.h"

#ifdef ENV_TEXTURE
TextureCube gEnvTexture : register(t4);
//Texture2D gEnvTexture : register(t4);
#endif

Texture2D gDepthMap : register(t5);

Texture2D gFogMap : register(t6);

Texture2D gNoiseMap : register(t7);


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

#ifdef ENV_TEXTURE
float3 SampleEnvironmentMap(float3 dir, float lod)
{
	float3 color = gEnvTexture.SampleLevel(gAnisotropicSampler, dir, lod).rgb *2.0;
	return color;
}
#endif

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

float turbulence(float3 p, int octaves, float lacunarity = 2.0, float gain = 0.5)
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

float3 GetFoggedColor(float2 uv, float3 incidentColor, float3 worldPos)
{
	float depth = gDepthMap.Sample(gPointSampler, uv);
	float4 volumeFog  = gFogMap.Sample(gPointSampler, uv);
	float start = volumeFog.x;
	float end = volumeFog.y;	
	end += volumeFog.b*volumeFog.a;
	
	float volumeFogDensity = (end - start);
	volumeFogDensity = volumeFogDensity * gFogColor.w * turbulence(worldPos * 0.35f + float3(gTime*0.02f, 0, 0), 2);
	
	//return volumeFogDensity.xxx;
	float p = volumeFogDensity*volumeFogDensity;
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