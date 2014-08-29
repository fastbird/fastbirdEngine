#include <CommonLib/StdAfx.h>

namespace fastbird
{

	float lerp(float a, float b, float w)
	{
		return a * (1.0f - w) + b * w;
	}
	float Noise(int x)
	{
		x = (x << 13) ^ x;
		return (1.0f - ((x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
	}

	float SmoothedNoise1D(float x)
	{
		int intx = (int)x;
		return Noise(intx) / 2.0f + (Noise(intx - 1) + Noise(intx + 1)) / 4.0f;
	}

	float InterpolatedNoise1D(float x)
	{
		float intX;
		float fractionalX = modf(x, &intX);

		float v1 = SmoothedNoise1D(intX);
		float v2 = SmoothedNoise1D(intX + 1);
		return v1 * 1.0f - fractionalX + v2 * fractionalX;
	}

	float PerlinNoise1D(float x)
	{
		float total = 0;
		float p = 0.5f;
		float f = 1;
		float a = 1;
		for (int i = 0; i < 4; i++)
		{
			total += InterpolatedNoise1D(x*f) * a;

			f *= 2.0f;
			a *= p;
		}
		return total;
	}

	float Noise2D(int x, int y)
	{
		int n = x + y * 57;
		n = (n << 13) ^ n;
		return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
	}

	float SmoothedNoise2D(float x, float y)
	{
		int intx = (int)x;
		int inty = (int)y;
		float corners = (Noise2D(intx - 1, inty - 1) + Noise2D(intx + 1, inty - 1) + Noise2D(intx - 1, inty + 1) + Noise2D(intx + 1, inty + 1)) / 16.0f;
		float sides = (Noise2D(intx - 1, inty) + Noise2D(intx + 1, inty) + Noise2D(intx, inty - 1) + Noise2D(intx, inty + 1)) / 8.0f;
		float center = Noise2D(intx, inty) / 4.0f;
		return corners + sides + center;
	}

	float InterpolatedNoise2D(float x, float y)
	{
		float intX;
		float fractionalX = modf(x, &intX);

		float intY;
		float fractionalY = modf(y, &intY);

		float v1 = SmoothedNoise2D(intX, intY);
		float v2 = SmoothedNoise2D(intX + 1, intY);
		float v3 = SmoothedNoise2D(intX, intY + 1);
		float v4 = SmoothedNoise2D(intX + 1, intY + 1);

		float i1 = lerp(v1, v2, fractionalX);
		float i2 = lerp(v3, v4, fractionalX);
		return lerp(i1, i2, fractionalY);
	}

	float PerlinNoise2D(float x, float y, float persistance)
	{
		float total = 0;
		float p = persistance;
		float f = 1;
		float a = 1;
		for (int i = 0; i < 4; i++)
		{
			total += InterpolatedNoise2D(x*f, y*f) * a;

			f *= 2.0f;
			a *= p;
		}
		return total;
	}
}