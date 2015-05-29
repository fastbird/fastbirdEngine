#include <UI/StdAfx.h>
#include <UI/EventHandler.h>
#include <UI/WinBase.h>
#include <UI/IUIManager.h>
#include <CommonLib/LuaUtils.h>

namespace fastbird
{

size_t EventHandler::UNIQUE_ID = 0;
unsigned EventHandler::sLastEventProcess = 0;
EventHandler::EventHandler()
: mEventEnable(true)
{
}
EventHandler::~EventHandler()
{
}

IEventHandler::FunctionID EventHandler::RegisterEventFunc(UIEvents::Enum e, EVENT_FUNCTION func)
{
	mFuncMap[UNIQUE_ID] = func;
	mEventFuncMap[e].insert(UNIQUE_ID);

	return UNIQUE_ID++;	
}

void EventHandler::UnregisterEventFunc(UIEvents::Enum e, FunctionID id)
{
	mEventFuncMap[e].erase(id);
	mFuncMap.erase(mFuncMap.find(id));
}

void EventHandler::UnregisterAllEventFunc()
{
	mFuncMap.clear();
	mEventFuncMap.clear();
}

bool EventHandler::RegisterEventLuaFunc(UIEvents::Enum e, const char* luaFuncName)
{
	if (luaFuncName == 0)
	{
		Error("Cannot register null function!");
		return false;
	}
	std::string funcName = StripBoth(luaFuncName);
	LuaObject func;
	func.FindFunction(gFBEnv->pUIManager->GetLuaState(), funcName.c_str());
	if_assert_pass(func.IsFunction()){
		mLuaFuncMap[e] = func;
		return true;
	}
	return false;
}

void EventHandler::UnregisterEventLuaFunc(UIEvents::Enum e)
{
	auto it = mLuaFuncMap.find(e);
	if (it != mLuaFuncMap.end())
		mLuaFuncMap.erase(it);
}

bool EventHandler::OnEvent(UIEvents::Enum e)
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

			lua_State* L = gFBEnv->pUIManager->GetLuaState();
			LUA_STACK_CLIPPER lsc(L);
			it->second.PushToStack();
			WinBase* pComp = dynamic_cast<WinBase*>(this);
			lua_pushstring(L, pComp->GetName());
			LUA_PCALL_RET_FALSE(L, it->second.IsMethod() ? 2 : 1, 0);
			processed = processed || true;
		}
		
	}
	if (processed)
		sLastEventProcess = gpTimer->GetFrame();

	return processed;
}

void EventHandler::DisableEvent(UIEvents::Enum e)
{
	mDisabledEvent.insert(e);
}

void EventHandler::DisableAllEvent()
{
	for (int i = 0; i < UIEvents::EVENT_NUM; ++i)
	{
		mDisabledEvent.insert((UIEvents::Enum)i);
	}
}

void EventHandler::ClearDisabledEvents()
{
	mDisabledEvent.clear();
}

}