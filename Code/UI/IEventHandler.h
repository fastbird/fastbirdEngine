#pragma once
namespace fastbird
{
	class CLASS_DECLSPEC_UI IEventHandler
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
			EVENT_DROP_DOWN_SELECTED,

			EVENT_NUM
		};

		typedef std::function< void(void*) > EVENT_FUNCTION;
		virtual FunctionID RegisterEventFunc(EVENT e, EVENT_FUNCTION) = 0;
		virtual void UnregisterEventFunc(EVENT e, FunctionID id) = 0;
		virtual void DisableEvent(EVENT e) = 0;

		// don't need to call this function if you already called IWinBase::SetEnable()
		virtual void SetEnableEvent(bool enable) = 0;
		virtual bool GetEnableEvent() const = 0;

	};
}