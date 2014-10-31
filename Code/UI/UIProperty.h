#pragma once
namespace fastbird
{
	namespace UIProperty
	{
		enum Enum
		{
			POS,
			NPOS,
			BACK_COLOR, // vec4
			BACK_COLOR_OVER,	// vec4
			BACK_COLOR_DOWN,
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
			TEXTURE_FILE,
			KEEP_IMAGE_RATIO,
			REGION,
			ALPHA,
			BACKGROUND_IMAGE,
			BACKGROUND_IMAGE_HOVER,
			BACKGROUND_IMAGE_NOATLAS,
			BACKGROUND_IMAGE_HOVER_NOATLAS,
			FRAME_IMAGE,
			TOOLTIP,
			PROGRESSBAR,
			GAUGE_COLOR,
			GAUGE_COLOR_EMPTY,
			GAUGE_BLINK_COLOR,
			GAUGE_BLINK_SPEED,
			NO_MOUSE_EVENT,
			SCROLLERH,
			SCROLLERV,
			USE_SCISSOR,
			LISTBOX_COL,
			LISTBOX_COL_SIZES,
			LISTBOX_COL_HEADERS,
			EDGE_COLOR,
			EDGE_COLOR_OVER,
			USE_WND_FRAME,
			TITLEBAR,
			USE_BORDER,
			SCISSOR_STOP_HERE,
			SPECIAL_ORDER, // higher will render top
			INHERIT_VISIBLE_TRUE, // Inherites visibility from parents constainer. Only works when the setting visibility is true.

			COUNT
		};

		static const char* strings[] = {
			"POS",
			"NPOS",
			"BACK_COLOR", // vec4
			"BACK_COLOR_OVER",	// vec4
			"BACK_COLOR_DOWN", // vec4
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
			"TEXTURE_FILE",
			"KEEP_IMAGE_RATIO",
			"REGION",
			"ALPHA",
			"BACKGROUND_IMAGE",
			"BACKGROUND_IMAGE_HOVER",
			"BACKGROUND_IMAGE_NOATLAS",
			"BACKGROUND_IMAGE_HOVER_NOATLAS",
			"FRAME_IMAGE",
			"TOOLTIP",
			"PROGRESSBAR",
			"GAUGE_COLOR",
			"GAUGE_COLOR_EMPTY",
			"GAUGE_BLINK_COLOR",
			"GAUGE_BLINK_SPEED",
			"NO_MOUSE_EVENT",
			"SCROLLERH",
			"SCROLLERV",
			"USE_SCISSOR",
			"LISTBOX_COL",
			"LISTBOX_COL_SIZES",
			"LISTBOX_COL_HEADERS",
			"EDGE_COLOR",
			"EDGE_COLOR_OVER",
			"USE_WND_FRAME", // with title bar
			"TITLEBAR",
			"USE_BORDER",
			"SCISSOR_STOP_HERE",
			"SPECIAL_ORDER",
			"INHERIT_VISIBLE_TRUE",
		};

		inline const char* ConvertToString(Enum e)
		{
			assert(e >= 0 && e < COUNT);
			return strings[e];
		}

		inline Enum ConverToEnum(const char* sz)
		{
			if (stricmp(sz, "POS") == 0)
				return POS;
			if (stricmp(sz, "NPOS") == 0)
				return NPOS;
			if (stricmp(sz, "BACK_COLOR") == 0)
				return BACK_COLOR;
			if (stricmp(sz, "BACK_COLOR_OVER") == 0)
				return BACK_COLOR_OVER;
			if (stricmp(sz, "BACK_COLOR_DOWN") == 0)
				return BACK_COLOR_DOWN;
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
			if (stricmp(sz, "TEXTURE_FILE") == 0)
				return TEXTURE_FILE;
			if (stricmp(sz, "KEEP_IMAGE_RATIO") == 0)
				return KEEP_IMAGE_RATIO;
			if (stricmp(sz, "REGION") == 0)
				return REGION;
			if (stricmp(sz, "ALPHA") == 0)
				return ALPHA;
			if (stricmp(sz, "BACKGROUND_IMAGE") == 0)
				return BACKGROUND_IMAGE;
			if (stricmp(sz, "BACKGROUND_IMAGE_HOVER") == 0)
				return BACKGROUND_IMAGE_HOVER;
			if (stricmp(sz, "BACKGROUND_IMAGE_NOATLAS") == 0)
				return BACKGROUND_IMAGE_NOATLAS;
			if (stricmp(sz, "BACKGROUND_IMAGE_HOVER_NOATLAS") == 0)
				return BACKGROUND_IMAGE_HOVER_NOATLAS;
			if (stricmp(sz, "FRAME_IMAGE") == 0)
				return FRAME_IMAGE;
			if (stricmp(sz, "TOOLTIP") == 0)
				return TOOLTIP;
			if (stricmp(sz, "PROGRESSBAR") == 0)
				return PROGRESSBAR;
			if (stricmp(sz, "GAUGE_COLOR") == 0)
				return GAUGE_COLOR;
			if (stricmp(sz, "GAUGE_COLOR_EMPTY") == 0)
				return GAUGE_COLOR_EMPTY;
			if (stricmp(sz, "GAUGE_BLINK_COLOR") == 0)
				return GAUGE_BLINK_COLOR;
			if (stricmp(sz, "GAUGE_BLINK_SPEED") == 0)
				return GAUGE_BLINK_SPEED;
			if (stricmp(sz, "NO_MOUSE_EVENT") == 0)
				return NO_MOUSE_EVENT;
			if (stricmp(sz, "SCROLLERH") == 0)
				return SCROLLERH;
			if (stricmp(sz, "SCROLLERV") == 0)
				return SCROLLERV;
			if (stricmp(sz, "USE_SCISSOR") == 0)
				return USE_SCISSOR;
			if (stricmp(sz, "LISTBOX_COL") == 0)
				return LISTBOX_COL;
			if (stricmp(sz, "LISTBOX_COL_SIZES") == 0)
				return LISTBOX_COL_SIZES;
			if (stricmp(sz, "LISTBOX_COL_HEADERS") == 0)
				return LISTBOX_COL_HEADERS;
			if (stricmp(sz, "EDGE_COLOR") == 0)
				return EDGE_COLOR;
			if (stricmp(sz, "EDGE_COLOR_OVER") == 0)
				return EDGE_COLOR_OVER;
			if (stricmp(sz, "USE_WND_FRAME") == 0)
				return USE_WND_FRAME;
			if (stricmp(sz, "TITLEBAR") == 0)
				return TITLEBAR;
			if (stricmp(sz, "USE_BORDER") == 0)
				return USE_BORDER;
			if (stricmp(sz, "SCISSOR_STOP_HERE") == 0)
				return SCISSOR_STOP_HERE;
			if (stricmp(sz, "SPECIAL_ORDER") == 0)
				return SPECIAL_ORDER;
			if (stricmp(sz, "INHERIT_VISIBLE_TRUE") == 0)
				return INHERIT_VISIBLE_TRUE;
			
			{
				assert(0);
				return COUNT;
			}
		}
	}
}