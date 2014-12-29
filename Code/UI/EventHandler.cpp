#include <UI/StdAfx.h>
#include <UI/EventHandler.h>
#include <UI/WinBase.h>
#include <UI/IUIManager.h>
#include <CommonLib/LuaUtils.h>

namespace fastbird
{

size_t EventHandler::UNIQUE_ID = 0;

EventHandler::EventHandler()
: mEventEnable(true)
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

void EventHandler::RegisterEventLuaFunc(EVENT e, const char* luaFuncName)
{
	if (luaFuncName == 0)
	{
		Error("Cannot register null function!");
		return;
	}
	std::string funcName = StripBoth(luaFuncName);
	LuaObject func;
	func.FindFunction(IUIManager::GetUIManager().GetLuaState(), funcName.c_str());
	if_assert_pass(func.IsFunction())
		mLuaFuncMap[e] = func;	
}

void EventHandler::UnregisterEventLuaFunc(EVENT e)
{
	auto it = mLuaFuncMap.find(e);
	if (it != mLuaFuncMap.end())
		mLuaFuncMap.erase(it);
}

bool EventHandler::OnEvent(EVENT e)
{
	if (mDisabledEvent.find(e) != mDisabledEvent.end() || !mEventEnable)
		return false;

	bool processed = false;
	auto it = mEventFuncMap.find(e);
	if (it!=mEventFuncMap.end())
	{
		for (auto funcID : it->second)
		{
			mFuncMap[funcID](dynamic_cast<WinBase*>(this));
		}
		processed = processed || true;
	}

	// check lua
	{
		const auto& it = mLuaFuncMap.find(e);
		if (it != mLuaFuncMap.end())
		{
			assert(it->second.IsFunction());

			lua_State* L = IUIManager::GetUIManager().GetLuaState();
			it->second.PushToStack();
			WinBase* pComp = dynamic_cast<WinBase*>(this);
			lua_pushstring(L, pComp->GetName());
			LUA_PCALL_RET_FALSE(L, it->second.IsMethod() ? 2 : 1, 0);
			processed = processed || true;
		}
		
	}



	return processed;
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

void EventHandler::ClearDisabledEvents()
{
	mDisabledEvent.clear();
}

}