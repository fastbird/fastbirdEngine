#pragma once
#include <UI/IEventHandler.h>
#include <CommonLib/LuaObject.h>

namespace fastbird
{
	class EventHandler : public IEventHandler
	{
	public:
		static size_t UNIQUE_ID;
		EventHandler();
		virtual ~EventHandler();
		virtual FunctionID RegisterEventFunc(EVENT e, EVENT_FUNCTION);
		virtual void UnregisterEventFunc(EVENT e, FunctionID slot);
		virtual void RegisterEventLuaFunc(EVENT e, const char* luaFuncName);
		virtual void UnregisterEventLuaFunc(EVENT e);
		virtual void UnregisterAllEventFunc();
		virtual void DisableEvent(EVENT e);
		virtual void DisableAllEvent();
		virtual void ClearDisabledEvents();

		virtual void SetEnableEvent(bool enable){ mEventEnable = enable; }
		virtual bool GetEnableEvent() const { return mEventEnable; }

	protected:
		friend class UIManager;
		bool OnEvent(EVENT e);

	protected:
		typedef std::map<FunctionID, EVENT_FUNCTION> FUNC_MAP;
		typedef std::map<EVENT, std::set<FunctionID> > EVENT_FUNC_MAP;
		FUNC_MAP mFuncMap;
		EVENT_FUNC_MAP mEventFuncMap;

		typedef std::map<EVENT, fastbird::LuaObject> LUA_EVENT_FUNC_MAP;
		LUA_EVENT_FUNC_MAP mLuaFuncMap;

		std::set<EVENT> mDisabledEvent;
		bool mEventEnable;
	};
}