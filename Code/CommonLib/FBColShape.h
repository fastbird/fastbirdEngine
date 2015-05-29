#pragma once
#include <CommonLib/Math/Transformation.h>
namespace fastbird
{
	namespace FBColShape
	{
		enum Enum
		{
			SPHERE,
			CUBE,
			MESH,

			Num
		};

		static const char* strings[] =
		{ "SPHERE", "CUBE", "MESH" };

		static const char* ConvertToString(Enum e)
		{
			assert(e >= SPHERE && e < Num);
			return strings[e];
		}

		static Enum ConvertToEnum(const char* str)
		{
			for (int i = 0; i < Num; ++i)
			{
				if (_stricmp(strings[i], str) == 0)
					return Enum(i);
			}
			assert(0);
			return Num;
		}
	}

	typedef std::pair<FBColShape::Enum, Transformation> COL_SHAPE;
}