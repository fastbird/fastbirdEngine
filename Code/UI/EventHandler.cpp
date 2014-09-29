#include <UI/StdAfx.h>
#include <UI/EventHandler.h>
#include <UI/WinBase.h>

namespace fastbird
{

size_t EventHandler::UNIQUE_ID = 0;

EventHandler::EventHandler()
{
}
EventHandler::~EventHandler()
{
}

IEventHandler::FunctionID EventHandler::RegisterEventFunc(EVENT e, EVENT_FUNCTION func)
{
	mFuncMap[UNIQUE_ID] = func;
	mEventFuncMap[e].insert(UNIQUE_ID);

	return UNIQUE_ID++;	
}

void EventHandler::UnregisterEventFunc(EVENT e, FunctionID id)
{
	mEventFuncMap[e].erase(id);
	mFuncMap.erase(mFuncMap.find(id));
}

void EventHandler::UnregisterAllEventFunc()
{
	mFuncMap.clear();
	mEventFuncMap.clear();
}

bool EventHandler::OnEvent(EVENT e)
{
	if (mDisabledEvent.find(e) != mDisabledEvent.end() || !mEventEnable)
		return false;

	auto it = mEventFuncMap.find(e);
	if (it!=mEventFuncMap.end())
	{
		for each(auto funcID in it->second)
		{
			mFuncMap[funcID](dynamic_cast<WinBase*>(this));
		}
		return true;
	}

	return false;
}

void EventHandler::DisableEvent(EVENT e)
{
	mDisabledEvent.insert(e);
}

void EventHandler::DisableAllEvent()
{
	for (int i = 0; i < EVENT_NUM; ++i)
	{
		mDisabledEvent.insert((EVENT)i);
	}
}

}