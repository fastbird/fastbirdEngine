#pragma once
namespace fastbird
{
	namespace UIProperty
	{
		enum Enum
		{
			POS,
			POSX,
			POSY,
			NPOS,
			NPOSX,
			NPOSY,
			SIZE,
			SIZEX,
			SIZEY,
			OFFSETX,
			OFFSETY,
			BACK_COLOR, // vec4
			BACK_COLOR_OVER,	// vec4
			BACK_COLOR_DOWN,
			TEXT_LEFT_GAP,
			TEXT_RIGHT_GAP,
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
			TEXTBOX_MATCH_HEIGHT,
			MATCH_HEIGHT,
			TEXTUREATLAS,
			TEXTURE_FILE,
			KEEP_IMAGE_RATIO,
			REGION,
			REGIONS,
			FPS,
			ALPHA,
			HOVER_IMAGE,
			BACKGROUND_IMAGE,
			BACKGROUND_IMAGE_HOVER,
			BACKGROUND_IMAGE_DISABLED,
			BACKGROUND_IMAGE_NOATLAS,
			BACKGROUND_IMAGE_HOVER_NOATLAS,
			FRAME_IMAGE,
			FRAME_IMAGE_DISABLED,
			CHANGE_IMAGE_ACTIVATE,
			ACTIVATED_IMAGE,
			ACTIVATED_IMAGE_ROT,
			DEACTIVATED_IMAGE,
			BUTTON_ACTIVATED,
			BUTTON_ICON_TEXT,
			TOOLTIP,
			PROGRESSBAR,
			GAUGE_COLOR,
			GAUGE_COLOR_EMPTY,
			GAUGE_BLINK_COLOR,
			GAUGE_BLINK_SPEED,
			GAUGE_MAX,
			GAUGE_CUR,
			NO_MOUSE_EVENT,
			INVALIDATE_MOUSE,
			SCROLLERH,
			SCROLLERV,
			USE_SCISSOR,
			LISTBOX_COL,
			LISTBOX_ROW_HEIGHT,
			LISTBOX_ROW_GAP,
			LISTBOX_COL_SIZES,
			LISTBOX_TEXT_SIZES,
			LISTBOX_COL_ALIGNH,
			LISTBOX_COL_HEADERS,
			LISTBOX_COL_HEADERS_TEXT_SIZE,
			EDGE_COLOR,
			EDGE_COLOR_OVER,
			USE_WND_FRAME,
			TITLEBAR,
			USE_BORDER,
			SPECIAL_ORDER, // higher will render top
			INHERIT_VISIBLE_TRUE, // Inherites visibility from parents constainer. Only works when the setting visibility is true.
			VISIBLE,
			SHOW_ANIMATION,
			HIDE_ANIMATION,
			ENABLED,
			IMAGE_COLOR_OVERLAY,
			IMAGE_FIXED_SIZE,
			NO_BUTTON,
			CHECKBOX_CHECKED,
			MODAL,

			COUNT
		};

		static const char* strings[] = {
			"POS",
			"POSX",
			"POSY",
			"NPOS",
			"NPOSX",
			"NPOSY",
			"SIZE",
			"SIZEX",
			"SIZEY",
			"OFFSETX",
			"OFFSETY",
			"BACK_COLOR", // vec4
			"BACK_COLOR_OVER",	// vec4
			"BACK_COLOR_DOWN", // vec4
			"TEXT_LEFT_GAP",
			"TEXT_RIGHT_GAP",
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
			"TEXTBOX_MATCH_HEIGHT",
			"MATCH_HEIGHT",
			"TEXTUREATLAS",
			"TEXTURE_FILE",
			"KEEP_IMAGE_RATIO",
			"REGION",
			"REGIONS",
			"FPS",
			"ALPHA",
			"HOVER_IMAGE",
			"BACKGROUND_IMAGE",
			"BACKGROUND_IMAGE_HOVER",
			"BACKGROUND_IMAGE_DISABLED",
			"BACKGROUND_IMAGE_NOATLAS",
			"BACKGROUND_IMAGE_HOVER_NOATLAS",
			"FRAME_IMAGE",
			"FRAME_IMAGE_DISABLED",
			"CHANGE_IMAGE_ACTIVATE",
			"ACTIVATED_IMAGE",
			"ACTIVATED_IMAGE_ROT",
			"DEACTIVATED_IMAGE",
			"BUTTON_ACTIVATED",
			"BUTTON_ICON_TEXT",
			"TOOLTIP",
			"PROGRESSBAR",
			"GAUGE_COLOR",
			"GAUGE_COLOR_EMPTY",
			"GAUGE_BLINK_COLOR",
			"GAUGE_BLINK_SPEED",
			"GAUGE_MAX",
			"GAUGE_CUR",
			"NO_MOUSE_EVENT",
			"INVALIDATE_MOUSE",
			"SCROLLERH",
			"SCROLLERV",
			"USE_SCISSOR",
			"LISTBOX_COL",
			"LISTBOX_ROW_HEIGHT",
			"LISTBOX_ROW_GAP",
			"LISTBOX_COL_SIZES",
			"LISTBOX_TEXT_SIZES",
			"LISTBOX_COL_ALIGNH",
			"LISTBOX_COL_HEADERS",
			"LISTBOX_COL_HEADERS_TEXT_SIZE",
			"EDGE_COLOR",
			"EDGE_COLOR_OVER",
			"USE_WND_FRAME", // with title bar
			"TITLEBAR",
			"USE_BORDER",
			"SPECIAL_ORDER",
			"INHERIT_VISIBLE_TRUE",
			"VISIBLE",
			"SHOW_ANIMATION",
			"HIDE_ANIMATION",
			"ENABLED",
			"IMAGE_COLOR_OVERLAY",
			"IMAGE_FIXED_SIZE",
			"NO_BUTTON",
			"CHECKBOX_CHECKED",
			"MODAL",

			"COUNT"
		};
		static_assert(ARRAYCOUNT(strings) == COUNT+1, "Count mismatch");
		inline const char* ConvertToString(Enum e)
		{
			assert(e >= 0 && e < COUNT);
			return strings[e];
		}

		inline Enum ConverToEnum(const char* sz)
		{
			int i = 0;
			for (auto& strEnum : strings)
			{
				if (_stricmp(strEnum, sz) == 0)
					return Enum(i);

				++i;
			}
			
			assert(0);
			return COUNT;
		}
	}
}