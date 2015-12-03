#pragma once
namespace fb{
	namespace ColisionShapeType
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

		static const char* ConvertToString(Enum e);
		static Enum ConvertToEnum(const char* str);
	}
}