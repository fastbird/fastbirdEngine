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
			EVENT_LISTBOX_SELECTION_CHANGED,
			EVENT_COLORRAMP_DRAGGED,

			EVENT_NUM
		};

		static const char* const StrEVENT[];
		static const char* ConvertToString(EVENT e);
		static EVENT ConvertToEnum(const char* str);

		typedef std::function< void(void*) > EVENT_FUNCTION;
		virtual FunctionID RegisterEventFunc(EVENT e, EVENT_FUNCTION) = 0;
		virtual bool RegisterEventLuaFunc(EVENT e, const char* luaFuncName) = 0;
		virtual void UnregisterEventLuaFunc(EVENT e) = 0;
		virtual void UnregisterEventFunc(EVENT e, FunctionID id) = 0;
		virtual void DisableEvent(EVENT e) = 0;
		virtual void ClearDisabledEvents() = 0;

		// don't need to call this function if you already called IWinBase::SetEnable()
		virtual void SetEnableEvent(bool enable) = 0;
		virtual bool GetEnableEvent() const = 0;
		virtual bool OnEvent(EVENT e) = 0;

	};
}