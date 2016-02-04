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

namespace fb{
	namespace UIEvents{
		enum Enum
		{
			EVENT_ENTER,
			EVENT_MOUSE_IN,
			EVENT_MOUSE_HOVER,
			EVENT_MOUSE_OUT,
			EVENT_MOUSE_LEFT_CLICK,
			EVENT_MOUSE_LEFT_DOUBLE_CLICK,
			EVENT_MOUSE_RIGHT_CLICK,
			EVENT_MOUSE_DOWN,
			EVENT_MOUSE_DRAG,
			EVENT_FILE_SELECTOR_SELECTED,
			EVENT_FILE_SELECTOR_OK,
			EVENT_FILE_SELECTOR_CANCEL,
			EVENT_NUMERIC_UP,
			EVENT_NUMERIC_DOWN,
			EVENT_NUMERIC_SET,
			EVENT_DROP_DOWN_SELECTED,
			EVENT_ON_VISIBLE,
			EVENT_ON_HIDE,
			EVENT_ON_LOADED,
			EVENT_ON_UNLOADED,
			EVENT_LISTBOX_CLEARED,
			EVENT_LISTBOX_SELECTION_CHANGED,
			EVENT_COLORRAMP_DRAGGED,
			EVENT_TAB_WINDOW_CHANGED,

			EVENT_NUM
		};

		static const char* StrEVENT[] = {
			"OnEnter",
			"OnMouseIn",
			"OnMouseHover",
			"OnMouseOut",
			"OnMouseLClick",
			"OnMouseLDClick",
			"OnMouseRClick",
			"OnMouseDown",
			"OnMouseDrag",
			"OnFileSelectorSelected",
			"OnFileSelectorOk",
			"OnFileSelectorCancel",
			"OnNumericUp",
			"OnNumericDown",
			"OnNumericSet",
			"OnDropDownSelected",
			"OnVisible",
			"OnHide",
			"EVENT_ON_LOADED",
			"EVENT_ON_UNLOADED",
			"OnListBoxCleared",
			"OnListBoxSelectionChanged",
			"OnColorRampDragged",
			"OnTabWindowChanged",

			"EVENT_NUM"
		};

		inline const char* ConvertToString(Enum e)
		{
			assert(e >= 0 && e < EVENT_NUM);
			return StrEVENT[e];
		}
		inline Enum ConvertToEnum(const char* str)
		{
			for (int i = 0; i < EVENT_NUM; ++i)
			{
				if (_stricmp(str, StrEVENT[i]) == 0)
					return Enum(i);
			}
			return EVENT_NUM;
		}

		inline Enum IsUIEvents(const char* str)
		{
			for (int i = 0; i < EVENT_NUM; ++i)
			{
				if (_stricmp(str, StrEVENT[i]) == 0)
					return Enum(i);
			}
			return EVENT_NUM;
		}
	}
}