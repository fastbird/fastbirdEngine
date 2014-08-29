#pragma once
namespace fastbird
{
	namespace ALIGNH
	{
		enum Enum
		{
			LEFT,
			CENTER,
			RIGHT,
		};

		static const char* strings[] =
		{
			"LEFT",
			"CENTER",
			"RIGHT",
		};

		inline const char* ConvertToString(Enum e)
		{
			assert(e >= LEFT && e <= RIGHT);
			return strings[e];
		}

		inline Enum ConvertToEnum(const char* sz)
		{
			assert(sz);
			if (stricmp(sz, "LEFT") == 0)
				return LEFT;
			if (stricmp(sz, "CENTER") == 0)
				return CENTER;
			if (stricmp(sz, "RIGHT") == 0)
				return RIGHT;
			else
			{
				assert(0);
				return LEFT;
			}
		}
	}

	namespace ALIGNV
	{
		enum Enum
		{
			TOP,
			MIDDLE,
			BOTTOM,
		};
		static const char* strings[] = {
			"TOP",
			"MIDDLE",
			"BOTTOM",
		};

		inline const char* ConvertToString(Enum e)
		{
			assert(e >= TOP && e <= BOTTOM);
			return strings[e];
		}

		inline Enum ConvertToEnum(const char* sz)
		{
			assert(sz);
			if (stricmp(sz, "TOP") == 0)
				return TOP;
			if (stricmp(sz, "MIDDLE") == 0)
				return MIDDLE;
			if (stricmp(sz, "BOTTOM") == 0)
				return BOTTOM;
			else
			{
				assert(0);
				return TOP;
			}
		}
	}
}