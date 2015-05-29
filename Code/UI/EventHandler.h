#pragma once
#include <UI/IEventHandler.h>
#include <CommonLib/LuaObject.h>

namespace fastbird
{
	class EventHandler : public IEventHandler
	{
	public:
		static size_t UNIQUE_ID;
		static unsigned sLastEventProcess;
		EventHandler();
		virtual ~EventHandler();
		virtual FunctionID RegisterEventFunc(UIEvents::Enum e, EVENT_FUNCTION);
		virtual void UnregisterEventFunc(UIEvents::Enum e, FunctionID slot);
		virtual bool RegisterEventLuaFunc(UIEvents::Enum e, const char* luaFuncName);
		virtual void UnregisterEventLuaFunc(UIEvents::Enum e);
		virtual void UnregisterAllEventFunc();
		virtual void DisableEvent(UIEvents::Enum e);
		virtual void DisableAllEvent();
		virtual void ClearDisabledEvents();

		virtual void SetEnableEvent(bool enable){ mEventEnable = enable; }
		virtual bool GetEnableEvent() const { return mEventEnable; }
		virtual bool OnEvent(UIEvents::Enum e);

	protected:
		friend class UIManager;
		

	protected:
		typedef std::map<FunctionID, EVENT_FUNCTION> FUNC_MAP;
		typedef std::map<UIEvents::Enum, std::set<FunctionID> > EVENT_FUNC_MAP;
		FUNC_MAP mFuncMap;
		EVENT_FUNC_MAP mEventFuncMap;

		typedef std::map<UIEvents::Enum, fastbird::LuaObject> LUA_EVENT_FUNC_MAP;
		LUA_EVENT_FUNC_MAP mLuaFuncMap;

		std::set<UIEvents::Enum> mDisabledEvent;
		bool mEventEnable;
	};
}