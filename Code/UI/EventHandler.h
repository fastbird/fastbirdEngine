#pragma once
#include <UI/IEventHandler.h>

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

	protected:
		void OnEvent(EVENT e);

	protected:
		typedef std::map<FunctionID, EVENT_FUNCTION> FUNC_MAP;
		typedef std::map<EVENT, std::set<FunctionID> > EVENT_FUNC_MAP;
		FUNC_MAP mFuncMap;
		EVENT_FUNC_MAP mEventFuncMap;
	};
}