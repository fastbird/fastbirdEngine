#pragma once
namespace fastbird
{
	namespace UIProperty
	{
		enum Enum
		{
			BACK_COLOR, // vec4
			BACK_COLOR_OVER,	// vec4
			TEXT_ALIGN,		// left, center, right
			TEXT_VALIGN,	// top, middle, bottom
			TEXT_SIZE,			// be sure set fixed text size also if you need.
			TEXT_COLOR,		// vec4
			TEXT_COLOR_HOVER,	// vec4
			TEXT_COLOR_DOWN,	// vec4
			FIXED_TEXT_SIZE,
			MATCH_SIZE,		// true, false
			NO_BACKGROUND,		// true, false
			ALIGNH,
			ALIGNV,
			TEXT,
			TEXTUREATLAS,
			REGION,
			ALPHA,

			COUNT
		};

		static const char* strings[] = {
			"BACK_COLOR", // vec4
			"BACK_COLOR_OVER",	// vec4
			"TEXT_ALIGN",		// left, center, right
			"TEXT_VALIGN", 
			"TEXT_SIZE",			// be sure set fixed text size also if you need.
			"TEXT_COLOR",		// vec4
			"TEXT_COLOR_HOVER",	// vec4
			"TEXT_COLOR_DOWN",	// vec4
			"FIXED_TEXT_SIZE",
			"MATCH_SIZE",		// true, false
			"NO_BACKGROUND",		// true, false
			"ALIGNH",
			"ALIGNV",
			"TEXT",
			"TEXTUREATLAS",
			"REGION",
			"ALPHA",
		};

		inline const char* ConvertToString(Enum e)
		{
			assert(e >= BACK_COLOR && e < COUNT);
			return strings[e];
		}

		inline Enum ConverToEnum(const char* sz)
		{
			if (stricmp(sz, "BACK_COLOR") == 0)
				return BACK_COLOR;
			if (stricmp(sz, "BACK_COLOR_OVER") == 0)
				return BACK_COLOR_OVER;
			if (stricmp(sz, "TEXT_ALIGN") == 0)
				return TEXT_ALIGN;
			if (stricmp(sz, "TEXT_VALIGN") == 0)
				return TEXT_VALIGN;
			if (stricmp(sz, "TEXT_SIZE") == 0)
				return TEXT_SIZE;
			if (stricmp(sz, "TEXT_COLOR") == 0)
				return TEXT_COLOR;
			if (stricmp(sz, "TEXT_COLOR_HOVER") == 0)
				return TEXT_COLOR_HOVER;
			if (stricmp(sz, "TEXT_COLOR_DOWN") == 0)
				return TEXT_COLOR_DOWN;
			if (stricmp(sz, "FIXED_TEXT_SIZE") == 0)
				return FIXED_TEXT_SIZE;
			if (stricmp(sz, "MATCH_SIZE") == 0)
				return MATCH_SIZE;
			if (stricmp(sz, "NO_BACKGROUND") == 0)
				return NO_BACKGROUND;
			if (stricmp(sz, "ALIGNH") == 0)
				return ALIGNH;
			if (stricmp(sz, "ALIGNV") == 0)
				return ALIGNV;
			if (stricmp(sz, "TEXT") == 0)
				return TEXT;
			if (stricmp(sz, "TEXTUREATLAS") == 0)
				return TEXTUREATLAS;
			if (stricmp(sz, "REGION") == 0)
				return REGION;
			if (stricmp(sz, "ALPHA") == 0)
				return ALPHA;
			else
			{
				assert(0);
				return COUNT;
			}
		}
	}
}