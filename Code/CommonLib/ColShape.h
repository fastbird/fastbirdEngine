#pragma once
#include <CommonLib/Math/Transformation.h>
namespace fastbird
{
	namespace ColShape
	{
		enum Enum
		{
			SPHERE,
			CUBE,

			Num
		};

		static const char* strings[] =
		{ "SPHERE", "CUBE" };

		static const char* ConvertToString(Enum e)
		{
			assert(e >= SPHERE && e < Num);
			return strings[e];
		}

		static Enum ConvertToEnum(const char* str)
		{
			if (stricmp(str, "SPHERE") == 0)
			{
				return SPHERE;
			}
			if (stricmp(str, "CUBE") == 0)
			{
				return CUBE;
			}
			assert(0);
			return Num;
		}
	}

	typedef std::pair<ColShape::Enum, Transformation> COL_SHAPE;
}