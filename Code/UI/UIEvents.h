#pragma once

namespace fastbird{
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
			EVENT_LISTBOX_CLEARED,
			EVENT_LISTBOX_SELECTION_CHANGED,
			EVENT_COLORRAMP_DRAGGED,

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
			"OnListBoxCleared",
			"OnListBoxSelectionChanged",
			"OnColorRampDragged",

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