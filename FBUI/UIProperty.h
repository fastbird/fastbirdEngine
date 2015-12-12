/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#pragma once
#include "Align.h"
#include "FBCommonHeaders/Helpers.h"
namespace fb
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
			NSIZEX,
			NSIZEY,
			OFFSETX,
			OFFSETY,
			USE_NPOSX,
			USE_NPOSY,
			USE_NSIZEX,
			USE_NSIZEY,

			BACK_COLOR, // vec4
			BACK_COLOR_OVER,	// vec4
			BACK_COLOR_DOWN,
			TEXT_LEFT_GAP,
			TEXT_RIGHT_GAP,
			TEXT_GAP,
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
			IMAGE_HFLIP,
			TEXTUREATLAS,
			TEXTURE_FILE,
			KEEP_UI_RATIO,
			UI_RATIO,
			KEEP_IMAGE_RATIO,
			REGION,
			REGIONS,
			FPS,
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
			GAUGE_BORDER_COLOR,
			NO_MOUSE_EVENT,
			NO_MOUSE_EVENT_ALONE,
			VISUAL_ONLY_UI, // no mouse & keyboard and region test.
			INVALIDATE_MOUSE,
			SCROLLERH,
			SCROLLERV,
			SCROLLERV_OFFSET,
			USE_SCISSOR,
			LISTBOX_COL,
			LISTBOX_ROW_HEIGHT,
			LISTBOX_ROW_GAP,
			LISTBOX_COL_SIZES,
			LISTBOX_TEXT_SIZES,
			LISTBOX_COL_ALIGNH,
			LISTBOX_COL_HEADERS,
			LISTBOX_COL_HEADERS_TEXT_SIZE,
			LISTBOX_HIGHLIGHT_COLOR,
			LISTBOX_MULTI_SELECTION,
			LISTBOX_NO_HIGHLIGHT,
			LISTBOX_NO_SEARCH,
			LISTBOX_HAND,
			EDGE_COLOR,
			EDGE_COLOR_OVER,
			USE_WND_FRAME,
			TITLEBAR,
			CLOSE_BTN,
			USE_BORDER,
			SPECIAL_ORDER, // higher will render top
			INHERIT_VISIBLE_TRUE, // Inherites visibility from parents constainer. Only works when the setting visibility is true.
			VISIBLE,
			SHOW_ANIMATION,
			HIDE_ANIMATION,
			ENABLED,
			IMAGE_COLOR_OVERLAY,
			IMAGE_FIXED_SIZE, // match to image size if not set it will matched to ui size
			IMAGE_ROTATE,
			IMAGE_LINEAR_SAMPLER,
			ALPHA_REGION,

			NO_BUTTON,
			CHECKBOX_CHECKED,
			MODAL,
			NUMERIC_UPDOWN_MINMAX,
			NUMERIC_UPDOWN_NUMBER,
			NUMERIC_UPDOWN_SHIFT_STEP,
			NUMERIC_UPDOWN_STEP,
			REGION_FILLED, // for vertical gauges for now.
			REGION_NOT_FILLED,
			ALWAYS_ON_TOP,
			CLOSE_BY_ESC,
			DROPDOWN_INDEX,
			DROPDOWN_MAX_HEIGHT,
			COLOR_RAMP_VALUES,
			DRAGABLE,
			NAMED_PORTRAIT_IMAGE_SIZE,
			NAMED_PORTRAIT_TEXT,
			NAMED_PORTRAIT_TEXT_COLOR,
			CARD_SIZEX,
			CARD_SIZEY,
			CARD_OFFSETY,
			RADIO_GROUP,
			RADIO_CHECK,

			SYNC_WINDOW_POS,
			MSG_TRANSLATION,
			TAB_ORDER,

			TABWND_NUM_TABS,
			TABWND_TAB_NAMES,

			BUTTON_IMAGE_SIZE,

			WND_NO_FOCUS,
			WND_MOVE_TO_BOTTOM, // when focused
			MOUSE_CURSOR_HAND,

			NO_FOCUS_BY_CLICK,
			SEND_EVENT_TO_CHILDREN,
			RECEIVE_EVENT_FROM_PARENT,

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
			"NSIZEX",
			"NSIZEY",
			"OFFSETX",
			"OFFSETY",
			"USE_NPOSX",
			"USE_NPOSY",
			"USE_NSIZEX",
			"USE_NSIZEY",
			"BACK_COLOR", // vec4
			"BACK_COLOR_OVER",	// vec4
			"BACK_COLOR_DOWN", // vec4
			"TEXT_LEFT_GAP",
			"TEXT_RIGHT_GAP",
			"TEXT_GAP",
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
			"IMAGE_HFLIP",
			"TEXTUREATLAS",
			"TEXTURE_FILE",
			"KEEP_UI_RATIO",
			"UI_RATIO",
			"KEEP_IMAGE_RATIO",
			"REGION",
			"REGIONS",
			"FPS",
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
			"GAUGE_BORDER_COLOR",
			"NO_MOUSE_EVENT",
			"NO_MOUSE_EVENT_ALONE", // not inherited to children
			"VISUAL_ONLY_UI",
			"INVALIDATE_MOUSE",
			"SCROLLERH",
			"SCROLLERV",
			"SCROLLERV_OFFSET",
			"USE_SCISSOR",
			"LISTBOX_COL",
			"LISTBOX_ROW_HEIGHT",
			"LISTBOX_ROW_GAP",
			"LISTBOX_COL_SIZES",
			"LISTBOX_TEXT_SIZES",
			"LISTBOX_COL_ALIGNH",
			"LISTBOX_COL_HEADERS",
			"LISTBOX_COL_HEADERS_TEXT_SIZE",
			"LISTBOX_HIGHLIGHT_COLOR",
			"LISTBOX_MULTI_SELECTION",
			"LISTBOX_NO_HIGHLIGHT",
			"LISTBOX_NO_SEARCH",
			"LISTBOX_HAND",
			"EDGE_COLOR",
			"EDGE_COLOR_OVER",
			"USE_WND_FRAME", // with title bar
			"TITLEBAR",
			"CLOSE_BTN",
			"USE_BORDER",
			"SPECIAL_ORDER",
			"INHERIT_VISIBLE_TRUE",
			"VISIBLE",
			"SHOW_ANIMATION",
			"HIDE_ANIMATION",
			"ENABLED",
			"IMAGE_COLOR_OVERLAY",
			"IMAGE_FIXED_SIZE",
			"IMAGE_ROTATE",
			"IMAGE_LINEAR_SAMPLER",
			"ALPHA_REGION",
			"NO_BUTTON",
			"CHECKBOX_CHECKED",
			"MODAL",
			"NUMERIC_UPDOWN_MINMAX",
			"NUMERIC_UPDOWN_NUMBER",
			"NUMERIC_UPDOWN_SHIFT_STEP",
			"NUMERIC_UPDOWN_STEP",
			"REGION_FILLED",
			"REGION_NOT_FILLED",
			"ALWAYS_ON_TOP",
			"CLOSE_BY_ESC",
			"DROPDOWN_INDEX",
			"DROPDOWN_MAX_HEIGHT",
			"COLOR_RAMP_VALUES",
			"DRAGABLE",
			"NAMED_PORTRAIT_IMAGE_SIZE",
			"NAMED_PORTRAIT_TEXT",
			"NAMED_PORTRAIT_TEXT_COLOR",
			"CARD_SIZEX",
			"CARD_SIZEY",
			"CARD_OFFSETY",
			"RADIO_GROUP",
			"RADIO_CHECK",

			"SYNC_WINDOW_POS",
			"MSG_TRANSLATION",

			"TAB_ORDER",

			"TABWND_NUM_TABS",
			"TABWND_TAB_NAMES",

			"BUTTON_IMAGE_SIZE",

			"WND_NO_FOCUS",
			"WND_MOVE_TO_BOTTOM",
			"MOUSE_CURSOR_HAND",

			"NO_FOCUS_BY_CLICK",

			"SEND_EVENT_TO_CHILDREN",
			"RECEIVE_EVENT_FROM_PARENT",

			"COUNT"
		};
		static_assert(ARRAYCOUNT(strings) == COUNT+1, "Count mismatch");
		inline const char* ConvertToString(Enum e)
		{
			assert(e >= 0 && e < COUNT);
			return strings[e];
		}
		inline const char* ConvertToString(unsigned e)
		{
			return ConvertToString(Enum(e));
		}

		inline Enum ConvertToEnum(const char* sz)
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

		inline Enum IsUIProperty(const char* sz)
		{
			int i = 0;
			for (auto& strEnum : strings)
			{
				if (_stricmp(strEnum, sz) == 0)
					return Enum(i);

				++i;
			}

			return COUNT;
		}

		inline Vec4 GetDefaultValueVec4(UIProperty::Enum prop)
		{
			switch (prop){
			case BACK_COLOR:
				return Vec4(0.05f, 0.1f, 0.15f, 0.8f);
			case TEXT_COLOR:
				return Vec4(0.9f, 0.9f, 0.9f, 1.0f);
			case TEXT_COLOR_HOVER:
				return Vec4(1.f, 1.0f, 1.f, 1.f);
			case TEXT_COLOR_DOWN:
				return Vec4(1.0f, 1.0f, 1.0f, 1.f);
			case BACK_COLOR_OVER:
				return Vec4(0.09f, 0.02f, 0.03f, 0.8f);
			case BACK_COLOR_DOWN:
				return Vec4(0.3f, 0.3f, 0.f, 0.5f);
			case EDGE_COLOR:
				return Vec4(1.f, 1.f, 1.f, 0.7f);
			case EDGE_COLOR_OVER:
				return Vec4(0.9f, 0.85f, 0.0f, 0.7f);
			case IMAGE_COLOR_OVERLAY:
				return Vec4(1, 1, 1, 1);
			case GAUGE_COLOR:
				return Vec4(1, 1, 1, 1);
			case GAUGE_BLINK_COLOR:
				return Vec4(1, 1, 0, 1);
			case GAUGE_BORDER_COLOR:
				return Vec4(0.5f, 0.5f, 0.5f, 0.5f);
			case LISTBOX_HIGHLIGHT_COLOR:
				return Vec4(0.1f, 0.3f, 0.3f, 0.7f);
			}
			assert(0);
			return Vec4::ZERO;
		}

		inline Vec2I GetDefaultValueVec2I(UIProperty::Enum prop)
		{
			switch (prop){
			case TEXT_GAP:
				return Vec2I::ZERO;
			case DRAGABLE:
				return Vec2I::ZERO;
			case NUMERIC_UPDOWN_MINMAX:
				return Vec2I(0, 100);
			case BUTTON_IMAGE_SIZE:
				return Vec2I(0, 0);
			}
			assert(0);
			return Vec2I::ZERO;
		}

		inline float GetDefaultValueFloat(UIProperty::Enum prop)
		{
			switch (prop){
			case TEXT_SIZE:
				return 16.f;
			case FPS:
				return 0.f;
			case GAUGE_MAX:
				return 1.f;
			case GAUGE_CUR:
				return 0.f;
			case GAUGE_BLINK_SPEED:
				return 3.f;
			case UI_RATIO:
				return 1.f;
			}
			assert(0);
			return 0.f;
		}

		inline bool GetDefaultValueBool(UIProperty::Enum prop)
		{
			switch (prop){
			case FIXED_TEXT_SIZE:
				return true;
			case MATCH_SIZE:
				return false;
			case NO_BACKGROUND:
				return false;			
			case NO_MOUSE_EVENT:
				return false;
			case VISUAL_ONLY_UI:
				return false;
			case NO_MOUSE_EVENT_ALONE:
				return false;
			case USE_SCISSOR:
				return true;
			case USE_BORDER:
				return false;
			case INHERIT_VISIBLE_TRUE:
				return true;
			case VISIBLE:
				return true;
			case ENABLED:
				return true;
			case MODAL:
				return false;
			case CHECKBOX_CHECKED:
				return false;
			case ACTIVATED_IMAGE_ROT:
				return false;
			case BUTTON_ACTIVATED:
				return false;
			case CHANGE_IMAGE_ACTIVATE:
				return false;
			case NO_BUTTON:
				return false;
			case SCROLLERV:
				return false;
			case SCROLLERH:
				return false;
			case MATCH_HEIGHT:
				return false;
			case KEEP_IMAGE_RATIO:
				return true;
			case IMAGE_FIXED_SIZE:
				return false;
			case IMAGE_ROTATE:
				return false;
			case IMAGE_LINEAR_SAMPLER:
				return false;
			case RADIO_CHECK:
				return false;
			case TEXTBOX_MATCH_HEIGHT:
				return false;
			case IMAGE_HFLIP:
				return false;
			case USE_WND_FRAME:
				return false;
			case ALWAYS_ON_TOP:
				return false;
			case CLOSE_BY_ESC:
				return false;
			case SYNC_WINDOW_POS:
				return false;
			case KEEP_UI_RATIO:
				return false;
			case LISTBOX_MULTI_SELECTION:
				return false;
			case LISTBOX_NO_HIGHLIGHT:
				return false;
			case LISTBOX_NO_SEARCH:
				return false;
			case CLOSE_BTN:
				return false;
			case WND_NO_FOCUS:
				return false;
			case WND_MOVE_TO_BOTTOM:
				return false;
			case LISTBOX_HAND:
				return false;
			case MOUSE_CURSOR_HAND:
				return false;
			case NO_FOCUS_BY_CLICK:
				return false;
			case SEND_EVENT_TO_CHILDREN:
				return false;
			case RECEIVE_EVENT_FROM_PARENT:
				return false;
			}
			assert(0);
			return false;
		}

		inline int GetDefaultValueInt(UIProperty::Enum prop)
		{
			switch (prop){
			case TEXT_ALIGN:
				return ALIGNH::LEFT;
			case TEXT_VALIGN:
				return ALIGNV::MIDDLE;
			case TEXT_LEFT_GAP:
				return 0;
			case TEXT_RIGHT_GAP:
				return 0;
			case ALIGNH:
				return ALIGNH::LEFT;
			case ALIGNV:
				return ALIGNV::TOP;
			case SPECIAL_ORDER:
				return 0;
			case TAB_ORDER:
				return -1;
			case BUTTON_ICON_TEXT:
				return 0;
			case CARD_SIZEX:
				return 200;
			case CARD_SIZEY:
				return 100;
			case CARD_OFFSETY:
				return 2;
			case LISTBOX_ROW_HEIGHT:
				return 24;
			case LISTBOX_ROW_GAP:
				return 4;
			case NUMERIC_UPDOWN_NUMBER:
				return 0;
			case RADIO_GROUP:
				return -1;
			case TABWND_NUM_TABS:
				return 0;
			case NUMERIC_UPDOWN_SHIFT_STEP:
				return 5;
			case NUMERIC_UPDOWN_STEP:
				return 1;
			case DROPDOWN_MAX_HEIGHT:
				return 200;
			}
			assert(0);
			return 0;
		}

		inline const char* GetDefaultValueString(UIProperty::Enum prop)
		{
			switch (prop)
			{
			case LISTBOX_HIGHLIGHT_COLOR:
				return "0.1, 0.3, 0.3, 0.7";
			}
			assert(0);
			return "";
		}
	}
}