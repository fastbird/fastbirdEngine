#pragma once
namespace fastbird
{
	class IEventHandler
	{
	public:
		typedef size_t FunctionID;
		virtual ~IEventHandler(){}

		enum EVENT
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
			EVENT_COLORRAMP_DRAGGED,

			EVENT_NUM
		};

		typedef std::function< void(void*) > EVENT_FUNCTION;
		virtual FunctionID RegisterEventFunc(EVENT e, EVENT_FUNCTION) = 0;
		virtual void RegisterEventLuaFunc(EVENT e, const char* luaFuncName) = 0;
		virtual void UnregisterEventLuaFunc(EVENT e) = 0;
		virtual void UnregisterEventFunc(EVENT e, FunctionID id) = 0;
		virtual void DisableEvent(EVENT e) = 0;
		virtual void ClearDisabledEvents() = 0;

		// don't need to call this function if you already called IWinBase::SetEnable()
		virtual void SetEnableEvent(bool enable) = 0;
		virtual bool GetEnableEvent() const = 0;

	};

	inline IEventHandler::EVENT ConvertToEventEnum(const char* sz)
	{
		if (_stricmp(sz, "OnVisible") == 0)
		{
			return IEventHandler::EVENT_ON_VISIBLE;
		}
		else if (_stricmp(sz, "OnHide") == 0)
		{
			return IEventHandler::EVENT_ON_HIDE;
		}
		else if (_stricmp(sz, "OnMouseIn") == 0)
		{
			return IEventHandler::EVENT_MOUSE_IN;
		}
		else if (_stricmp(sz, "OnMouseHover") == 0)
		{
			return IEventHandler::EVENT_MOUSE_HOVER;
		}
		else if (_stricmp(sz, "OnMouseOut") == 0)
		{
			return IEventHandler::EVENT_MOUSE_OUT;
		}
		else if (_stricmp(sz, "OnMouseLClick") == 0)
		{
			return IEventHandler::EVENT_MOUSE_LEFT_CLICK;
		}
		else if (_stricmp(sz, "OnMouseLDClick") == 0)
		{
			return IEventHandler::EVENT_MOUSE_LEFT_DOUBLE_CLICK;
		}
		else if (_stricmp(sz, "OnMouseRClick") == 0)
		{
			return IEventHandler::EVENT_MOUSE_RIGHT_CLICK;
		}
		else if (_stricmp(sz, "OnMouseDown") == 0)
		{
			return IEventHandler::EVENT_MOUSE_DOWN;
		}
		else if (_stricmp(sz, "OnMouseDrag") == 0)
		{
			return IEventHandler::EVENT_MOUSE_DRAG;
		}
		else if (_stricmp(sz, "OnEnter") == 0)
		{
			return IEventHandler::EVENT_ENTER;
		}
		else if (_stricmp(sz, "OnFileSelectorSelected") == 0)
		{
			return IEventHandler::EVENT_FILE_SELECTOR_SELECTED;
		}
		else if (_stricmp(sz, "OnFileSelectorOk") == 0)
		{
			return IEventHandler::EVENT_FILE_SELECTOR_OK;
		}
		else if (_stricmp(sz, "OnFileSelectorCancel") == 0)
		{
			return IEventHandler::EVENT_FILE_SELECTOR_CANCEL;
		}
		else if (_stricmp(sz, "OnNumericUp") == 0)
		{
			return IEventHandler::EVENT_NUMERIC_UP;
		}
		else if (_stricmp(sz, "OnNumericDown") == 0)
		{
			return IEventHandler::EVENT_NUMERIC_DOWN;
		}
		else if (_stricmp(sz, "OnNumericSet") == 0)
		{
			return IEventHandler::EVENT_NUMERIC_SET;
		}
		else if (_stricmp(sz, "OnDropDownSelected") == 0)
		{
			return IEventHandler::EVENT_DROP_DOWN_SELECTED;
		}
		else if (_stricmp(sz, "EVENT_ON_LOADED") == 0)
		{
			return IEventHandler::EVENT_ON_LOADED;
		}
		else if (_stricmp(sz, "OnListBoxCleared") == 0)
		{
			return IEventHandler::EVENT_LISTBOX_CLEARED;
		}
		else if (_stricmp(sz, "OnColorRampDragged") == 0)
		{
			return IEventHandler::EVENT_COLORRAMP_DRAGGED;
		}
		else
		{
			assert(0);
			return IEventHandler::EVENT_NUM;
		}
	}
}