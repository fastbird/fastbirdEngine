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

#pragma once
#include "UIEvents.h"

namespace fb
{
	class FB_DLL_UI EventHandler
	{
	public:
		typedef size_t FunctionID;
		typedef std::function< void(void*) > EVENT_FUNCTION;
		static size_t UNIQUE_ID;
		static unsigned sLastEventProcess;
		EventHandler();
		virtual ~EventHandler();
		FunctionID RegisterEventFunc(UIEvents::Enum e, EVENT_FUNCTION);
		void UnregisterEventFunc(UIEvents::Enum e, FunctionID slot);
		bool RegisterEventLuaFunc(UIEvents::Enum e, const char* luaFuncName);
		void UnregisterEventLuaFunc(UIEvents::Enum e);
		void UnregisterAllEventFunc();
		void DisableEvent(UIEvents::Enum e);
		void DisableAllEvent();
		void ClearDisabledEvents();

		void SetEnableEvent(bool enable){ mEventEnable = enable; }
		bool GetEnableEvent() const { return mEventEnable; }
		bool OnEvent(UIEvents::Enum e);

	protected:
		friend class UIManager;
		

	protected:
		typedef std::map<FunctionID, EVENT_FUNCTION> FUNC_MAP;
		typedef std::map<UIEvents::Enum, std::set<FunctionID> > EVENT_FUNC_MAP;
		FUNC_MAP mFuncMap;
		EVENT_FUNC_MAP mEventFuncMap;

		typedef std::map<UIEvents::Enum, fb::LuaObject> LUA_EVENT_FUNC_MAP;
		LUA_EVENT_FUNC_MAP mLuaFuncMap;

		std::set<UIEvents::Enum> mDisabledEvent;
		bool mEventEnable;
	};
}