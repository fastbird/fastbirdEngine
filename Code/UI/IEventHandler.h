#pragma once
#include <UI/UIEvents.h>

namespace fastbird
{
	class IEventHandler
	{
	public:
		typedef size_t FunctionID;
		virtual ~IEventHandler(){}

		typedef std::function< void(void*) > EVENT_FUNCTION;
		virtual FunctionID RegisterEventFunc(UIEvents::Enum e, EVENT_FUNCTION) = 0;
		virtual bool RegisterEventLuaFunc(UIEvents::Enum e, const char* luaFuncName) = 0;
		virtual void UnregisterEventLuaFunc(UIEvents::Enum e) = 0;
		virtual void UnregisterEventFunc(UIEvents::Enum e, FunctionID id) = 0;
		virtual void DisableEvent(UIEvents::Enum e) = 0;
		virtual void ClearDisabledEvents() = 0;

		// don't need to call this function if you already called IWinBase::SetEnable()
		virtual void SetEnableEvent(bool enable) = 0;
		virtual bool GetEnableEvent() const = 0;
		virtual bool OnEvent(UIEvents::Enum e) = 0;

	};
}