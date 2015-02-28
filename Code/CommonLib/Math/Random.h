#pragma once
#include <CommonLib/Math/Vec3.h>
#include <CommonLib/Color.h>

namespace fastbird
{
	//-------------------------------------------------------------------------
	// max is included
	inline float Random()
	{
		return (float)std::rand() / (float)RAND_MAX;
	}
	template<class T>
	inline  T Random(T min, T max)
	{
		T k = max - min;
		T offset = (T)(((float)std::rand() / (float)RAND_MAX) * (float)k);
		T result = min + offset;
		return result;
	}

	inline Vec3 Random(const Vec3& min, const Vec3& max) 
	{
		return Vec3(Random(min.x, max.x),
			Random(min.y, max.y),
			Random(min.z, max.z));
	};
	inline Vec3 RandomDirection()
	{
		return Random(Vec3(-1, -1, -1), Vec3(1, 1, 1)).NormalizeCopy();
	}

	inline Vec2 Random(const Vec2& min, const Vec2& max) 
	{
		return Vec2(Random(min.x, max.x),
			Random(min.y, max.y));
	};

	inline Color Random(const Color& min, const Color& max)
	{
		return Color(Random(min.r(), max.r()),
			Random(min.g(), max.g()),
			Random(min.b(), max.b()));
	}

	//-------------------------------------------------------------------------
	template<class T>
	class Randomizer
	{
	public:
		Randomizer(T min, T max)
		{
			mMin = min;
			mMax = max;
		}
		T Get(T value)
		{
			T factor = Random(mMin, mMax);
			return value + factor;

		}

	private:
		T mMin;
		T mMax;
	};
}