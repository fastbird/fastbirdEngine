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
			EVENT_MOUSE_CLICK,
			EVENT_MOUSE_DOUBLE_CLICK,
			EVENT_MOUSE_DOWN,
			EVENT_FILE_SELECTOR_SELECTED,
			EVENT_FILE_SELECTOR_OK,
			EVENT_FILE_SELECTOR_CANCEL,
		};

		typedef std::function< void(void*) > EVENT_FUNCTION;
		virtual FunctionID RegisterEventFunc(EVENT e, EVENT_FUNCTION) = 0;
		virtual void UnregisterEventFunc(EVENT e, FunctionID id) = 0;

	};
}