/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#include "StdAfx.h"
#include "EventHandler.h"
#include "WinBase.h"
#include "UIManager.h"

namespace fb
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

EventHandler::FunctionID EventHandler::RegisterEventFunc(UIEvents::Enum e, EVENT_FUNCTION func)
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
	func.FindFunction(UIManager::GetInstance().GetLuaState(), funcName.c_str());
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

			lua_State* L = UIManager::GetInstance().GetLuaState();
			LUA_STACK_CLIPPER lsc(L);
			it->second.PushToStack();
			WinBase* pComp = dynamic_cast<WinBase*>(this);
			LuaUtils::pushstring(pComp->GetName());			
			it->second.CallWithManualArgs(1, 0);
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