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
#include "UIManager.h"
#include "StaticText.h"
#include "Button.h"
#include "ListBox.h"
#include "ListItem.h"
#include "ListItemDataType.h"
#include "ListBoxData.h"
#include "PropertyList.h"
#include "TextBox.h"
#include "CheckBox.h"
#include "NumericUpDown.h"
#include "DropDown.h"
#include "ColorRampComp.h"
#include "CardScroller.h"
#include "HorizontalGauge.h"
#include "RadioBox.h"
#include "UIAnimation.h"
#include "ImageBox.h"
#include "TabWindow.h"
#include "FBLua/LuaUtils.h"
//--------------------------------------------------------------------------------

namespace fb
{
	int GetUIPosSize(lua_State* L) {
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compoName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compoName);
		if (!comp) {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("no comp(%s, %s) found.", uiname, compoName).c_str());
			return 0;
		}
		auto pos = comp->GetPos();
		auto size = comp->GetSize();
		LuaUtils::pushinteger(L, pos.x);
		LuaUtils::pushinteger(L, pos.y);
		LuaUtils::pushinteger(L, size.x);
		LuaUtils::pushinteger(L, size.y);
		return 4;
	}
	
	int LoadLuaUI(lua_State* L)
	{
		const char* uiFile = LuaUtils::checkstring(L, 1);
		HWindowId id = INVALID_HWND_ID;
		if (LuaUtils::gettop(L) == 2)
		{
			id = LuaUtils::checkunsigned(L, 2);
		}
		else
		{
			id = Renderer::GetInstance().GetMainWindowHandleId();
		}
		std::vector<WinBasePtr> temp;
		std::string name;
		UIManager::GetInstance().ParseUI(uiFile, temp, name, id, true);
		UIManager::GetInstance().SetVisible(name.c_str(), false);
		return 0;
	}

	int IsLoadedUI(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		bool loaded = UIManager::GetInstance().IsLoadedUI(uiname);
		LuaUtils::pushboolean(L, loaded);
		return 1;
	}

	int AddLuaUI(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		LuaObject ui(L, 2);
		HWindowId hwndId = INVALID_HWND_ID;
		if (LuaUtils::gettop(L) == 3)
		{
			hwndId = LuaUtils::checkunsigned(L, 2);
		}
		else
		{
			hwndId = Renderer::GetInstance().GetMainWindowHandleId();
		}
		UIManager::GetInstance().AddLuaUI(uiname, ui, hwndId);
		UIManager::GetInstance().SetVisible(uiname, false);
		return 0;
	}

	int DeleteLuaUI(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		bool pending = false;
		if (LuaUtils::gettop(L) == 2){
			pending = LuaUtils::toboolean(L, 2);
		}
		UIManager::GetInstance().DeleteLuaUI(uiname, pending);
		return 0;
	}

	int SetStaticText(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compoName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compoName);
		auto staticText = std::dynamic_pointer_cast<StaticText>(comp);
		if (staticText)
		{
			staticText->SetText(AnsiToWide(LuaUtils::checkstring(L, 3)));
			LuaUtils::pushboolean(L, 1);
		}
		else
		{
			LuaUtils::pushboolean(L, 0);
		}
		return 1;
	}

	int SetVisibleLuaUI(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);		
		LuaUtils::checktype(L, 2, LUA_TBOOLEAN);
		bool visible = LuaUtils::toboolean(L, 2);		
		UIManager::GetInstance().SetVisible(uiname, visible);
		return 0;
	}

	int ToggleVisibleLuaUI(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		UIManager::GetInstance().ToggleVisibleLuaUI(uiname);
		return 0;
	}

	int CloseAllLuaUI(lua_State* L)
	{
		UIManager::GetInstance().CloseAllLuaUI();
		return 0;
	}

	int GetVisibleLuaUI(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);		
		bool visible = UIManager::GetInstance().GetVisible(uiname);
		LuaUtils::pushboolean(L, visible);
		return 1;
	}

	int SetVisibleComponent(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		LuaUtils::checktype(L, 3, LUA_TBOOLEAN);
		bool visible = LuaUtils::toboolean(L, 3);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		if (comp)
		{
			comp->SetVisible(visible);
			return 0;
		}

		return 0;
	}

	int SetVisibleChildrenComponent(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		LuaUtils::checktype(L, 3, LUA_TBOOLEAN);
		bool visible = LuaUtils::toboolean(L, 3);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		if (comp)
		{
			comp->SetVisibleChildren(visible);
			return 0;
		}

		return 0;
	}

	int RemoveAllChildrenOf(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		bool immedi = false;
		if (LuaUtils::gettop(L) == 3){
			immedi = LuaUtils::toboolean(L, 3) != 0;
		}

		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		if (comp)
		{
			comp->RemoveAllChildren(immedi);
			LuaUtils::pushboolean(L, 1);
		}
		else
		{
			LuaUtils::pushboolean(L, 0);
		}
		return 1;
	}

	int RemoveComponent(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);		
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		if (comp)
		{
			auto parent = comp->GetParent();
			if (parent)
			{
				parent->RemoveChild(comp);
			}
			else
			{
				Logger::Log(FB_ERROR_LOG_ARG, "Cannot remove root component.");
			}
		}
		return 0;
	}

	int AddComponent(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		LuaObject compTable(L, 3);
		if (comp)
		{
			//UIManager::GetInstance().SetEnablePosSizeEvent(false);
			auto winbase = comp->AddChild(compTable);
			//UIManager::GetInstance().SetEnablePosSizeEvent(true);
			if (winbase)
			{
				//winbase->OnSizeChanged();
				LuaUtils::pushboolean(L, 1);
				winbase->SetVisible(true);
				return 1;
			}
		}

		LuaUtils::pushboolean(L, 0);
		return 1;
	}

	int AddComponentTabWindow(lua_State* L) {
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		if (!comp) {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString(
				"Cannot find a comp(%s, %s)", uiname, compName).c_str());
			return 0;
		}
		auto tabWindow = std::dynamic_pointer_cast<TabWindow>(comp);
		if (!tabWindow) {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString(
				"%s is not a TabWindow", compName).c_str());
			return 0;
		}

		unsigned index = LuaUtils::checkunsigned(L, 3);
		LuaObject data(L, 4);
		if (!data.IsValid()) {
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid ui data.");
			return 0;
		}

		tabWindow->AddTabChild(index, data);
		return 0;
	}

	int CreateNewCard(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* cardScrollerName = LuaUtils::checkstring(L, 2);
		unsigned key = LuaUtils::checkunsigned(L, 3);
		LuaObject card(L, 4);
		auto comp = UIManager::GetInstance().FindComp(uiname, cardScrollerName);
		if (!comp)
		{
			Logger::Log(FB_ERROR_LOG_ARG, "No card scroller found!");
			return 0;
		}

		CardScroller* cardScroller = (CardScroller*)comp.get();
		cardScroller->AddCard(key, card);
		return 0;
	}

	int DeleteCard(lua_State* L){
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* cardScrollerName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, cardScrollerName);
		if (!comp)
		{
			Logger::Log(FB_ERROR_LOG_ARG, "No card scroller found!");
			return 0;
		}

		CardScroller* cardScroller = (CardScroller*)comp.get();
		unsigned key = LuaUtils::checkunsigned(L, 3);
		cardScroller->DeleteCard(key);
		return 0;
	}

	int DeleteAllCard(lua_State* L){
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* cardScrollerName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, cardScrollerName);
		if (!comp)
		{
			Logger::Log(FB_ERROR_LOG_ARG, "No card scroller found!");
			return 0;
		}

		CardScroller* cardScroller = (CardScroller*)comp.get();
		cardScroller->DeleteAllCard();
		return 0;
	}

	int IsExistingCard(lua_State* L){
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* cardScrollerName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, cardScrollerName);
		if (!comp)
		{
			Logger::Log(FB_ERROR_LOG_ARG, "No card scroller found!");
			return 0;
		}

		CardScroller* cardScroller = (CardScroller*)comp.get();
		unsigned key = LuaUtils::checkunsigned(L, 3);
		bool exist = cardScroller->IsExisting(key);
		LuaUtils::pushboolean(L, exist);
		return 1;

	}

	int BlinkButton(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		LuaUtils::checktype(L, 3, LUA_TBOOLEAN);
		bool blink = LuaUtils::toboolean(L, 3);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		if (comp)
		{
			auto button = dynamic_cast<Button*>(comp.get());
			if (button)
			{
				if (LuaUtils::gettop(L) == 4){
					float time = (float)LuaUtils::checknumber(L, 4);
					button->Blink(blink, time);
				}
				else{
					button->Blink(blink);
				}
				
				return 0;
			}
			else
			{
				auto bar = dynamic_cast<HorizontalGauge*>(comp.get());
				if (bar){
					if (LuaUtils::gettop(L) == 4){
						float time = (float)LuaUtils::checknumber(L, 4);
						bar->Blink(blink, time);
					}
					else{
						bar->Blink(blink);
					}

					return 0;
				}
			}
		}
		assert(0);
		return 0;
	}

	int UpdateButtonProgressBar(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		float percent = (float)LuaUtils::checknumber(L, 3);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		if (comp)
		{
			auto button = dynamic_cast<Button*>(comp.get());
			if (button)
			{
				button->SetPercentage(percent);
				return 0;
			}
		}
		assert(0);
		return 0;
	}

	int StartButtonProgressBar(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		if (comp)
		{
			auto button = dynamic_cast<Button*>(comp.get());
			if (button)
			{
				button->StartProgress();
				return 0;
			}
		}
		assert(0);
		return 0;
	}

	int EndButtonProgressBar(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		if (comp)
		{
			auto button = dynamic_cast<Button*>(comp.get());
			if (button)
			{
				button->EndProgress();
				return 0;
			}
		}
		assert(0);
		return 0;
	}

	int SetTextBoxText(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		const char* text = LuaUtils::checkstring(L, 3);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		if (comp)
		{
			auto textBox = dynamic_cast<TextBox*>(comp.get());
			if (textBox)
			{
				textBox->SetText(AnsiToWide(text));
				return 0;
			}
		}
		assert(0);
		return 0;
	}

	int SetUIBackground(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		const char* image = LuaUtils::checkstring(L, 3);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		if (comp)
		{
			comp->SetProperty(UIProperty::BACKGROUND_IMAGE_NOATLAS, image);
			return 0;
		}
		assert(0);
		return 0;
	}

	int CacheUIComponent(lua_State* L) {
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto success = UIManager::GetInstance().CacheUIComponent(uiname, compName);
		LuaUtils::pushboolean(L, success);
		return 1;
	}

	int SetUIPropertyCached(lua_State* L) {
		const char* prop = LuaUtils::checkstring(L, 1);
		const char* val = LuaUtils::checkstring(L, 2);
		bool updatePosSize = false;
		if (LuaUtils::gettop(L) == 3) {
			updatePosSize = LuaUtils::toboolean(L, 3);
		}
		UIManager::GetInstance().SetUIPropertyCached(prop, val, updatePosSize);
		return 0;
	}

	int SetUIProperty(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		const char* prop = LuaUtils::checkstring(L, 3);
		const char* val = LuaUtils::checkstring(L, 4);		
		bool updatePosSize = false;
		if (LuaUtils::gettop(L) == 5){
			updatePosSize = LuaUtils::toboolean(L, 5);			
		}
		UIManager::GetInstance().SetUIProperty(uiname, compName, prop, val, updatePosSize);
		
		return 0;

	}

	int GetUIProperty(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		const char* prop = LuaUtils::checkstring(L, 3);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		if (comp)
		{
			char buf[UIManager::PROPERTY_BUF_SIZE];

			bool result = comp->GetProperty(UIProperty::ConvertToEnum(prop), 
				buf, UIManager::PROPERTY_BUF_SIZE, false);
			if (result)
			{
				LuaUtils::pushstring(L, buf);
				return 1;
			}
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString(
				"Component(%s) in the ui(%s) is not found.", compName, uiname).c_str());
		}
		return 0;
	}

	int SetCardUIProperty(lua_State* L){
		int li = 1;
		const char* uiname = LuaUtils::checkstring(L, li++);
		const char* scrollerName = LuaUtils::checkstring(L, li++);
		unsigned key = LuaUtils::checkunsigned(L, li++);
		const char* compName = LuaUtils::checkstring(L, li++);
		const char* prop = LuaUtils::checkstring(L, li++);
		const char* val = LuaUtils::checkstring(L, li++);
		auto card = (CardScroller*)UIManager::GetInstance().FindComp(uiname, scrollerName).get();
		if (card){
			card->SetItemProperty(key, compName, prop, val);
		}

		return 0;
	}

	int RemoveUIEventhandler(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		const char* eventName = LuaUtils::checkstring(L, 3);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		if (comp)
		{
			auto eventHandler = dynamic_cast<EventHandler*>(comp.get());
			if (eventHandler)
			{
				eventHandler->UnregisterEventLuaFunc(UIEvents::ConvertToEnum(eventName));
				return 0;
			}
		}
		assert(0);
		return 0;

	}

	int GetMousePos(lua_State* L)
	{
		long x, y;
		auto injector = InputManager::GetInstance().GetInputInjector();
		injector->GetMousePos(x, y);		

		LuaUtils::pushnumber(L, x);
		LuaUtils::pushnumber(L, y);
		return 2;
	}

	int GetComponentWidth(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		if (comp)
		{
			LuaUtils::pushinteger(L, comp->GetSize().x);
			return 1;
		}
		assert(0);
		Logger::Log(FB_ERROR_LOG_ARG, FormatString(
			"UI component not found! (%s, %s)", uiname, compName).c_str());
		LuaUtils::pushinteger(L, 0);
		return 1;
	}

	WinBase* gRememberedComp = 0;
	int FindAndRememberComponent(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		gRememberedComp = UIManager::GetInstance().FindComp(uiname, compName).get();
		LuaUtils::pushboolean(L, gRememberedComp != 0);
		return 1;
	}

	int SetActivationUIAnim(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		if (!comp)
		{
			Logger::Log(FB_ERROR_LOG_ARG, "no comp found");
			return 0;
		}
		const char* animName = LuaUtils::checkstring(L, 3);
		bool active = LuaUtils::toboolean(L, 4);

		auto anim = comp->GetOrCreateUIAnimation(animName);
		anim->SetActivated(active);
		return 0;
	}

	int SetFocusUI(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		UIManager::GetInstance().SetFocusUI(uiname);
		return 0;
	}

	int CloneLuaUI(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* newUIname = LuaUtils::checkstring(L, 2);
		UIManager::GetInstance().CloneUI(uiname, newUIname);
		return 0;
	}

	int IsButtonActivated(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		Button* btn = dynamic_cast<Button*>(comp.get());
		if (btn)
		{
			bool activated = btn->IsActivated();
			LuaUtils::pushboolean(L, activated);
			return 1;
		}

		return 0;
	}

	int StartUIAnimation(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		const char* animName = LuaUtils::checkstring(L, 3);

		if (comp)
		{
			if (comp->GetNoBackground())
			{
				comp->SetProperty(UIProperty::NO_BACKGROUND, "false");
				comp->SetProperty(UIProperty::BACK_COLOR, "0, 0, 0, 0");
			}
			auto uiAnimation = comp->GetUIAnimation(animName);
			if (!uiAnimation)
			{
				auto ganim = UIManager::GetInstance().GetGlobalAnimation(animName);
				uiAnimation = ganim->Clone();
				uiAnimation->SetGlobalAnim(true);
				comp->SetUIAnimation(uiAnimation);
			}
			if (uiAnimation)
			{
				uiAnimation->SetActivated(true);
			}
		}
		return 0;
	}

	int StopUIAnimation(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		const char* animName = LuaUtils::checkstring(L, 3);

		if (comp)
		{
			auto uiAnimation = comp->GetUIAnimation(animName);
			if (uiAnimation)
			{
				uiAnimation->SetActivated(false);
			}
		}
		return 0;
	}

	int MatchUIHeight(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		bool checkName = false;
		if (!LuaUtils::isnil(L, 3))
			checkName = LuaUtils::toboolean(L, 3)!=0;

		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		if (comp)
		{
			Container* con = dynamic_cast<Container*>(comp.get());
			if (con)
			{
				con->MatchHeight(checkName);
			}
		}
		return 0;
	}

	int GetCheckedFromCheckBox(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto comp = dynamic_cast<CheckBox*>(UIManager::GetInstance().FindComp(uiname, compName).get());
		if (comp)
		{
			LuaUtils::pushboolean(L, comp->GetCheck());
			return 1;
		}
		return 0;
	}

	int GetNumericUpDownValue(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto comp = dynamic_cast<NumericUpDown*>(UIManager::GetInstance().FindComp(uiname, compName).get());
		if (comp)
		{
			LuaUtils::pushinteger(L, comp->GetValue());
			return 1;
		}
		return 0;
	}

	int SetNumericUpDownValue(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto comp = dynamic_cast<NumericUpDown*>(UIManager::GetInstance().FindComp(uiname, compName).get());
		if (comp)
		{
			int val = LuaUtils::checkint(L, 3);
			comp->SetNumber(val);
			return 0;
		}
		return 0;
	}

	int MoveUIToBottom(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		UIManager::GetInstance().MoveToBottom(uiname);
		return 0;
	}

	int MoveUIToTop(lua_State* L) 
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		UIManager::GetInstance().MoveToTop(uiname);
		return 0;
	}

	int SetDropDownIndex(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		unsigned index = LuaUtils::checkunsigned(L, 3);
		auto dropDown = dynamic_cast<DropDown*>(UIManager::GetInstance().FindComp(uiname, compName).get());
		if (dropDown)
		{
			dropDown->SetSelectedIndex(index);
		}
		return 0;
	}

	int GetDropDownIndex(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto dropDown = dynamic_cast<DropDown*>(UIManager::GetInstance().FindComp(uiname, compName).get());
		if (dropDown)
		{
			LuaUtils::pushunsigned(L, dropDown->GetSelectedIndex());
			return 1;
		}
		return 0;
	}

	int GetDropDownString(lua_State* L){
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto dropDown = dynamic_cast<DropDown*>(UIManager::GetInstance().FindComp(uiname, compName).get());
		if (dropDown)
		{
			unsigned idx = LuaUtils::checkunsigned(L, 3);
			auto str = dropDown->GetItemString(idx);
			if (wcslen(str) == 0){
				return 0;
			}
			std::string cstr = WideToAnsi(str);
			LuaUtils::pushstring(L, cstr.c_str());
			return 1;
		}
		return 0;
	}

	int ClearDropDownItem(lua_State* L) {
		auto uiname = LuaUtils::checkstring(L, 1);
		auto compname = LuaUtils::checkstring(L, 2);
		auto dropdown = std::dynamic_pointer_cast<DropDown>(UIManager::GetInstance().FindComp(uiname, compname));
		if (!dropdown) {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Dropdown(%s, %s) component is not found.", uiname, compname).c_str());
			return 0;
		}
		dropdown->ClearDropDownItems();
		return 0;
	}

	int AddDropDownItem(lua_State* L) {
		auto uiname = LuaUtils::checkstring(L, 1);
		auto compname = LuaUtils::checkstring(L, 2);
		auto dropdown = std::dynamic_pointer_cast<DropDown>(UIManager::GetInstance().FindComp(uiname, compname));
		if (!dropdown) {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Dropdown(%s, %s) component is not found.", uiname, compname).c_str());
			return 0;
		}

		if (LuaUtils::isstring(L, 3)) {
			size_t rowIndex = -1;
			auto szItem = LuaUtils::checkstring(L, 3);
			if (LuaUtils::isnumber(L, 4)) {
				auto key = LuaUtils::checkunsigned(L, 4);
				rowIndex = dropdown->AddDropDownItem(key, AnsiToWide(szItem));
			}
			else {
				rowIndex = dropdown->AddDropDownItem(AnsiToWide(szItem));
			}
			LuaUtils::pushunsigned(L, rowIndex);
			return 1;
		}
		else {
			Logger::Log(FB_ERROR_LOG_ARG, "Only string is supported for now.");
		}
		return 0;
	}

	int GetDropDownKey(lua_State* L) {
		auto uiname = LuaUtils::checkstring(L, 1);
		auto compname = LuaUtils::checkstring(L, 2);
		auto dropdown = std::dynamic_pointer_cast<DropDown>(UIManager::GetInstance().FindComp(uiname, compname));
		if (!dropdown) {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Dropdown(%s, %s) component is not found.", uiname, compname).c_str());
			return 0;
		}
		unsigned index = LuaUtils::checkunsigned(L, 3);
		auto key = dropdown->GetKey(index);
		LuaUtils::pushunsigned(L, key);
		return 1;
	}

	int SetVisibleLuaUIWithoutFocusing(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		LuaUtils::checktype(L, 2, LUA_TBOOLEAN);
		bool visible = LuaUtils::toboolean(L, 2);
		UIManager::GetInstance().LockFocus(true);
		UIManager::GetInstance().SetVisible(uiname, visible);
		UIManager::GetInstance().LockFocus(false);
		return 0;
	}

	int GetColorRampUIValues(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto colorRamp = dynamic_cast<ColorRampComp*>(UIManager::GetInstance().FindComp(uiname, compName).get());
		if (colorRamp)
		{
			std::vector<float> values;
			colorRamp->GetColorRampValuesFloats(values);
			if (values.empty())
				return 0;
			LuaObject table;
			table.NewTable(L);
			int n = 1;
			for (auto v : values)
			{
				table.SetSeq(n++, v);
			}
			table.PushToStack();
			return 1;
		}
		return 0;
	}

	int SetColorRampUIValues(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto colorRamp = dynamic_cast<ColorRampComp*>(UIManager::GetInstance().FindComp(uiname, compName).get());
		if (colorRamp)
		{
			std::vector<float> values;
			LuaObject table(L, 3);
			auto it = table.GetSequenceIterator();
			LuaObject elem;
			while (it.GetNext(elem))
			{
				values.push_back(elem.GetFloat());
			}
			colorRamp->SetColorRampValuesFloats(values);
		}
		return 0;
	}

	int HideUIsExcept(lua_State* L)
	{
		LuaObject table(L, 1);
		std::vector<std::string> excepts;
		auto it = table.GetSequenceIterator();
		LuaObject data;
		while (it.GetNext(data))
		{
			excepts.push_back(data.GetString());
		}
		UIManager::GetInstance().HideUIsExcept(excepts);
		return 0;
	}

	int StartHighlightUI(lua_State* L)
	{
		auto uiname = LuaUtils::checkstring(L, 1);
		UIManager::GetInstance().HighlightUI(uiname);
		return 0;
	}

	int StopHighlightUI(lua_State* L)
	{
		auto uiname = LuaUtils::checkstring(L, 1);
		UIManager::GetInstance().StopHighlightUI(uiname);
		return 0;
	}

	int SetHighlightUIComp(lua_State* L){
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		if (!LuaUtils::isboolean(L, 3))
		{
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid param.");
			return 0;
		}
		bool enable = LuaUtils::toboolean(L, 3) != 0;
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		if (comp)
		{
			auto type = comp->GetType();
			switch (type){
			case ComponentType::Button:
			{
				auto button = std::dynamic_pointer_cast<Button>(comp);
				button->Highlight(enable);
				break;
			}
			case ComponentType::ImageBox:{
				auto imageBox = std::dynamic_pointer_cast<ImageBox>(comp);
				imageBox->Highlight(enable);
				break;
			}
			}
		}
		else {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot find a ui comp(%s)", compName).c_str());
		}
		return 0;
	}

	int SetEnableComponent(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		if (!LuaUtils::isboolean(L, 3))
		{
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid param.");
			return 0;
		}
		bool enable = LuaUtils::toboolean(L, 3)!=0;
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		if (comp)
		{
			comp->SetEnable(enable);
		}
		return 0;
	}

	int GetUIPath(lua_State* L)
	{
		auto uiname = LuaUtils::checkstring(L, 1);
		const char* path = UIManager::GetInstance().GetUIPath(uiname);
		if (path)
		{
			LuaUtils::pushstring(L, path);
			return 1;
		}
		return 0;
	}

	int GetUIScriptPath(lua_State* L)
	{
		auto uiname = LuaUtils::checkstring(L, 1);
		const char* path = UIManager::GetInstance().GetUIScriptPath(uiname);
		if (path)
		{
			LuaUtils::pushstring(L, path);
			return 1;
		}
		return 0;
	}

	int GetNumUIProperties(lua_State* L)
	{
		LuaUtils::pushinteger(L, UIProperty::COUNT);
		return 1;
	}

	int GetUIPropertyName(lua_State* L)
	{
		unsigned i = LuaUtils::checkunsigned(L, 1);
		assert(i < UIProperty::COUNT);
		LuaUtils::pushstring(L, UIProperty::ConvertToString(i));
		return 1;
	}

	int GetPropertyCurKeyValue(lua_State* L)
	{
		auto uiname = LuaUtils::checkstring(L, 1);
		auto compName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName).get();
		if (comp && comp->GetType() == ComponentType::PropertyList)
		{
			auto propertyList = (PropertyList*)comp;
			std::string key, value;
			propertyList->GetCurKeyValue(key, value);
			LuaUtils::pushstring(L, key.c_str());
			LuaUtils::pushstring(L, value.c_str());
			return 2;
		}
		return 0;

	}

	int GetNumUIEvents(lua_State* L){
		LuaUtils::pushinteger(L, UIEvents::EVENT_NUM);
		return 1;
	}

	int GetUIEventName(lua_State* L){
		UIEvents::Enum e = (UIEvents::Enum)LuaUtils::checkunsigned(L, 1);
		LuaUtils::pushstring(L, UIEvents::ConvertToString(e));
		return 1;
	}

	int ModifyDropDownItem(lua_State* L){
		auto uiname = LuaUtils::checkstring(L, 1);
		auto compname = LuaUtils::checkstring(L, 2);
		unsigned index = LuaUtils::checkunsigned(L, 3) - 1;
		auto prop = UIProperty::ConvertToEnum(LuaUtils::checkstring(L, 4));
		auto str = LuaUtils::checkstring(L, 5);
		auto comp = UIManager::GetInstance().FindComp(uiname, compname).get();
		if (comp && comp->GetType() == ComponentType::DropDown){
			auto dropDown = (DropDown*)comp;
			if (dropDown){
				dropDown->ModifyItem(index, prop, str);
			}
		}
		return 0;
	}


	//-----------------------------------------------------------------------
	// Listbox
	//-----------------------------------------------------------------------
	int ClearListBox(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compoName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compoName);
		auto listBox = dynamic_cast<ListBox*>(comp.get());
		if (listBox)
		{
			listBox->Clear();
			LuaUtils::pushboolean(L, 1);
		}
		else
		{
			LuaUtils::pushboolean(L, 0);
		}
		return 1;
	}

	//-----------------------------------------------------------------------
	int ClearPropertyItems(lua_State* L)
	{
		auto uiname = LuaUtils::checkstring(L, 1);
		auto compName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName).get();
		if (comp && comp->GetType() == ComponentType::PropertyList)
		{
			auto propertyList = (PropertyList*)comp;
			propertyList->Clear();
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int SortPropertyList(lua_State* L)
	{
		auto uiname = LuaUtils::checkstring(L, 1);
		auto compName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName).get();
		if (comp && comp->GetType() == ComponentType::PropertyList)
		{
			auto propertyList = (PropertyList*)comp;
			propertyList->Sort();
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int CacheListBox(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		bool found = UIManager::GetInstance().CacheListBox(uiname, compName);
		LuaUtils::pushboolean(L, found);
		return 1;
	}

	//-----------------------------------------------------------------------
	int InsertListBoxItem(lua_State* L) // string or numeric key
	{
		auto listbox = UIManager::GetInstance().GetCachedListBox();
		if (!listbox)
			return 0;

		if (LuaUtils::isnumber(L, 1))
		{
			unsigned key = LuaUtils::checkunsigned(L, 1);
			unsigned row = listbox->InsertItem(key);
			LuaUtils::pushunsigned(L, row);
			return 1;
		}
		else if (LuaUtils::isstring(L, 1))
		{
			const char* key = LuaUtils::checkstring(L, 1);
			unsigned row = listbox->InsertItem(AnsiToWide(key));
			LuaUtils::pushunsigned(L, row);
			return 1;
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int SetListBoxItem(lua_State* L)
	{
		auto listbox = UIManager::GetInstance().GetCachedListBox();
		if (!listbox)
			return 0;
		unsigned rowIndex = LuaUtils::checkunsigned(L, 1);
		unsigned colIndex = LuaUtils::checkunsigned(L, 2);
		ListItemDataType::Enum dataType = (ListItemDataType::Enum)LuaUtils::checkunsigned(L, 3);
		
		switch (dataType){
		case ListItemDataType::String:
		case ListItemDataType::TextField:
		case ListItemDataType::TexturePath:
		case ListItemDataType::TextureRegion:
		{
			auto str = LuaUtils::checkstring(L, 4);			
			listbox->SetItem(Vec2I(rowIndex, colIndex), AnsiToWide(str), dataType);
			break;
		}
		case ListItemDataType::CheckBox:
		{
			listbox->SetItem(Vec2I(rowIndex, colIndex), LuaUtils::toboolean(L, 4)!=0);
			break;
		}
		case ListItemDataType::NumericUpDown:
		{
			listbox->SetItem(Vec2I(rowIndex, colIndex), (int)LuaUtils::tonumber(L, 4));
			break;
		}
		case ListItemDataType::HorizontalGauge:
		{
			listbox->SetItem(Vec2I(rowIndex, colIndex), (float)LuaUtils::tonumber(L, 4));
			break;
		}
		default:
			assert(0);
		}

		return 0;
	}

	//-----------------------------------------------------------------------
	int AddPropertyItem(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* listCompName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, listCompName);
		PropertyList* propertyList = dynamic_cast<PropertyList*>(comp.get());
		if (propertyList)
		{
			const char* key = LuaUtils::checkstring(L, 3);
			const char* value = LuaUtils::checkstring(L, 4);
			std::wstring wKey = AnsiToWide(key);
			std::wstring wValue = AnsiToWide(value);
			propertyList->InsertItem(wKey.c_str(), wValue.c_str());
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int GetListBoxSelectedRows(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		ListBox* listbox = dynamic_cast<ListBox*>(comp.get());
		if (listbox)
		{
			auto rows = listbox->GetSelectedRows();
			if (rows.empty())
			{
				LuaUtils::pushnil(L);
				return 1;
			}
			LuaObject luaRows;
			luaRows.NewTable(L);
			int i = 1;
			for (auto& row : rows)
			{
				luaRows.SetSeq(i++, row);
			}
			luaRows.PushToStack();
			return 1;
		}

		assert(0);
		return 0;
	}

	//-----------------------------------------------------------------------
	int GetListBoxSelectedStringKeys(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		ListBox* listbox = dynamic_cast<ListBox*>(comp.get());
		if (listbox)
		{
			std::vector<std::string> keys;
			listbox->GetSelectedUniqueIdsString(keys);

			if (keys.empty())
			{
				LuaUtils::pushnil(L);
				return 1;
			}
			LuaObject luaRows;
			luaRows.NewTable(L);
			int i = 1;
			for (auto& key : keys)
			{
				luaRows.SetSeq(i++, key.c_str());
			}
			luaRows.PushToStack();
			return 1;
		}

		assert(0);
		return 0;
	}

	//-----------------------------------------------------------------------
	int GetListBoxSelectedNumericKeys(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		ListBox* listbox = dynamic_cast<ListBox*>(comp.get());
		if (listbox)
		{
			std::vector<unsigned> keys;
			listbox->GetSelectedUniqueIdsUnsigned(keys);
			
			LuaObject luaRows;
			luaRows.NewTable(L);
			int i = 1;
			for (auto& key : keys)
			{
				luaRows.SetSeq(i++, key);
			}
			luaRows.PushToStack();
			return 1;
		}

		assert(0);
		return 0;
	}

	//-----------------------------------------------------------------------
	int GetListBoxStringKey(lua_State* L){
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		ListBox* listBox = dynamic_cast<ListBox*>(comp.get());
		if (listBox){
			unsigned row = LuaUtils::checkunsigned(L, 3);
			auto key = listBox->GetStringKey(row);
			if (key){
				LuaUtils::pushstring(L, WideToAnsi(key));
				return 1;
			}
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int GetListBoxNumericKey(lua_State* L){
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName);
		ListBox* listBox = dynamic_cast<ListBox*>(comp.get());
		if (listBox){
			unsigned row = LuaUtils::checkunsigned(L, 3);
			auto key = listBox->GetUnsignedKey(row);
			LuaUtils::pushnumber(L, key);
			return 1;
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int GetListBoxStringKeyCached(lua_State* L){
		auto listBox = UIManager::GetInstance().GetCachedListBox();
		if (listBox){
			unsigned row = LuaUtils::checkunsigned(L, 1);
			auto key = listBox->GetStringKey(row);
			if (key){
				LuaUtils::pushstring(L, WideToAnsi(key));
				return 1;
			}
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int GetListBoxNumericKeyCached(lua_State* L){
		auto listBox = UIManager::GetInstance().GetCachedListBox();
		if (listBox){
			unsigned row = LuaUtils::checkunsigned(L, 1);
			auto key = listBox->GetUnsignedKey(row);
			LuaUtils::pushnumber(L, key);
			return 1;
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int GetListBoxRowIndex(lua_State* L) {
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto listBox =std::dynamic_pointer_cast<ListBox>(UIManager::GetInstance().FindComp(uiname, compName));			
		if (listBox)
		{
			if (LuaUtils::isnumber(L, 3)) {
				LuaUtils::pushnumber(L, listBox->FindRowIndex(
					LuaUtils::checkunsigned(L, 3)));
				return 1;
			}
			else {

				LuaUtils::pushnumber(L, listBox->FindRowIndex(
					AnsiToWide(LuaUtils::checkstring(L, 3))
					));
				return 1;
			}
		}
		else {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot find the comp(%s)", compName).c_str());
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int GetListBoxRowIndexCached(lua_State* L){
		auto listBox = UIManager::GetInstance().GetCachedListBox();
		if (listBox)
		{
			if (LuaUtils::isnumber(L, 1)){
				LuaUtils::pushnumber(L, listBox->FindRowIndex(
					LuaUtils::checkunsigned(L, 1)));
				return 1;
			}
			else{

				LuaUtils::pushnumber(L, listBox->FindRowIndex(
					AnsiToWide(LuaUtils::checkstring(L, 1))
					));
				return 1;
			}
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int SelectListBoxItem(lua_State* L)
	{
		const char* wnd = LuaUtils::checkstring(L, 1);
		const char* comp = LuaUtils::checkstring(L, 2);
		if (LuaUtils::isnil(L, 3))
		{
			return 0;
		}
		unsigned row = LuaUtils::checkunsigned(L, 3);
		auto winbase = UIManager::GetInstance().FindComp(wnd, comp).get();
		ListBox* listbox = dynamic_cast<ListBox*>(winbase);
		if (listbox)
		{
			if (row == -1)
			{
				listbox->ClearSelection();
			}
			else
			{
				listbox->SelectRow(row);
			}

		}

		return 0;
	}

	//-----------------------------------------------------------------------
	int SelectListBoxItems(lua_State* L)
	{
		const char* wnd = LuaUtils::checkstring(L, 1);
		const char* comp = LuaUtils::checkstring(L, 2);
		if (LuaUtils::isnil(L, 3))
		{
			return 0;
		}
		LuaObject rows(L, 3);
		if (!rows.IsTable())
		{
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid Param.");
			return 0;
		}
		auto winbase = UIManager::GetInstance().FindComp(wnd, comp).get();
		ListBox* listbox = dynamic_cast<ListBox*>(winbase);
		if (listbox)
		{
			listbox->ClearSelection();
			unsigned len = rows.GetLen();
			for (unsigned i = 1; i <= len; ++i)
			{
				unsigned row = rows.GetUnsignedAt(i);
				listbox->SelectRow(row);
			}
		}

		return 0;
	}

	int SelectListBoxItemsById(lua_State* L) {
		const char* wnd = LuaUtils::checkstring(L, 1);
		const char* comp = LuaUtils::checkstring(L, 2);
		if (LuaUtils::isnil(L, 3))
		{
			return 0;
		}
		LuaObject ids(L, 3);
		if (!ids.IsTable())
		{
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid Param.");
			return 0;
		}
		auto winbase = UIManager::GetInstance().FindComp(wnd, comp).get();
		ListBox* listbox = dynamic_cast<ListBox*>(winbase);
		if (listbox)
		{
			listbox->ClearSelection();
			unsigned len = ids.GetLen();
			for (unsigned i = 1; i <= len; ++i)
			{
				unsigned id = ids.GetUnsignedAt(i);
				listbox->SelectId(id);
			}
		}

		return 0;
	}

	//-----------------------------------------------------------------------
	int GetListItemDataImpl(lua_State* L, ListBox* listbox, unsigned row, unsigned col){
		if (listbox)
		{
			auto itemdata = listbox->GetData(row, col);
			if (itemdata->IsTextData()){
				LuaUtils::pushstring(L, WideToAnsi(itemdata->GetText()));
				return 1;
			}
			else if (itemdata->GetDataType() == ListItemDataType::CheckBox){
				LuaUtils::pushboolean(L, itemdata->GetChecked());
				return 1;
			}
			else if (itemdata->GetDataType() == ListItemDataType::NumericUpDown){
				LuaUtils::pushnumber(L, itemdata->GetInt());
				return 1;
			}
			else if (itemdata->GetDataType() == ListItemDataType::HorizontalGauge){
				LuaUtils::pushnumber(L, itemdata->GetFloat());
				return 1;
			}
			else
			{
				assert(0);
			}
		}
		return 0;

	}

	//-----------------------------------------------------------------------
	int GetListItemData(lua_State* L){
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto listbox = dynamic_cast<ListBox*>(UIManager::GetInstance().FindComp(uiname, compName).get());
		unsigned row = LuaUtils::checkunsigned(L, 3);
		unsigned col = LuaUtils::checkunsigned(L, 4);
		return GetListItemDataImpl(L, listbox, row, col);
	}

	//-----------------------------------------------------------------------
	int GetListItemDataCachedListBox(lua_State* L){
		auto listbox = UIManager::GetInstance().GetCachedListBox().get();
		unsigned row = LuaUtils::checkunsigned(L, 1);
		unsigned col = LuaUtils::checkunsigned(L, 2);
		return GetListItemDataImpl(L, listbox, row, col);		
	}

	//-----------------------------------------------------------------------
	int SetListItemProperty(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* listCompName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, listCompName);
		ListBox* listbox = dynamic_cast<ListBox*>(comp.get());
		if (listbox)
		{
			unsigned rowIndex = LuaUtils::checkunsigned(L, 3);
			unsigned colIndex = LuaUtils::checkunsigned(L, 4);
			auto item = listbox->GetItem(Vec2I(rowIndex, colIndex));
			auto prop = LuaUtils::checkstring(L, 5);
			auto val = LuaUtils::checkstring(L, 6);
			item->SetProperty(UIProperty::ConvertToEnum(prop), val);
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int StartUIAnimationForListItem(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName).get();
		if (!comp || comp->GetType() != ComponentType::ListBox)
			return 0;
		unsigned index = LuaUtils::checkunsigned(L, 3);
		const char* animName = LuaUtils::checkstring(L, 4);

		ListBox* listbox = (ListBox*)comp;
		unsigned numCols = listbox->GetNumCols();
		for (unsigned c = 0; c < numCols; ++c)
		{
			auto item = listbox->GetItem(Vec2I((int)index, (int)c));
			if (item)
			{
				if (item->GetNoBackground())
				{
					item->SetProperty(UIProperty::NO_BACKGROUND, "false");
					item->SetProperty(UIProperty::BACK_COLOR, "0, 0, 0, 0");
				}
				auto uiAnimation = item->GetUIAnimation(animName);
				if (!uiAnimation)
				{
					auto ganim = UIManager::GetInstance().GetGlobalAnimation(animName);
					uiAnimation = ganim->Clone();
					item->SetUIAnimation(uiAnimation);
				}
				if (uiAnimation)
				{
					uiAnimation->SetActivated(true);
				}
			}
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int StopUIAnimationForListItem(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compName).get();
		if (!comp || comp->GetType() != ComponentType::ListBox)
			return 0;
		unsigned index = LuaUtils::checkunsigned(L, 3);
		const char* animName = LuaUtils::checkstring(L, 4);

		ListBox* listbox = (ListBox*)comp;
		unsigned numCols = listbox->GetNumCols();
		for (unsigned c = 0; c < numCols; ++c)
		{
			auto item = listbox->GetItem(Vec2I((int)index, (int)c));
			auto uiAnimation = item->GetUIAnimation(animName);
			if (uiAnimation)
			{
				uiAnimation->SetActivated(false);
			}
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int GetNumListBoxData(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto listBox = dynamic_cast<ListBox*>(UIManager::GetInstance().FindComp(uiname, compName).get());
		if (listBox)
		{
			LuaUtils::pushunsigned(L, listBox->GetNumData());
			return 1;
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int SwapListBoxItem(lua_State* L)
	{
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto listBox = dynamic_cast<ListBox*>(UIManager::GetInstance().FindComp(uiname, compName).get());
		if (listBox)
		{
			listBox->SwapItems(LuaUtils::checkunsigned(L, 3), LuaUtils::checkunsigned(L, 4));
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int GetLastChangedRow(lua_State* L){
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto listBox = dynamic_cast<ListBox*>(UIManager::GetInstance().FindComp(uiname, compName).get());
		if (listBox)
		{
			unsigned row = listBox->GetLastChangedRow();
			if (row != -1){
				LuaUtils::pushunsigned(L, row);
				return 1;
			}
		}
		return 0;
	}

	int SetListBoxItemProperty(lua_State* L){
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto listBox = dynamic_cast<ListBox*>(UIManager::GetInstance().FindComp(uiname, compName).get());
		if (listBox)
		{
			if (LuaUtils::type(L, 3) == LUA_TNUMBER){
				unsigned key = LuaUtils::checkunsigned(L, 3);
				listBox->SetItemProperty(key, UIProperty::ConvertToEnum(LuaUtils::checkstring(L, 4)), LuaUtils::checkstring(L, 5));
			}
			else if (LuaUtils::type(L, 4) == LUA_TSTRING){
				listBox->SetItemProperty(AnsiToWide(LuaUtils::checkstring(L, 3)), UIProperty::ConvertToEnum(LuaUtils::checkstring(L, 4)), LuaUtils::checkstring(L, 5));
			}
			else
				assert(0);
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int DisableListBoxItemEvent(lua_State* L) {
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto listBox = dynamic_cast<ListBox*>(UIManager::GetInstance().FindComp(uiname, compName).get());
		if (listBox) {
			listBox->DisableItemEvent(LuaUtils::checkunsigned(L, 3));
		}
		return 0;
	}

	int EnableListBoxItemEvent(lua_State* L){
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compName = LuaUtils::checkstring(L, 2);
		auto listBox = dynamic_cast<ListBox*>(UIManager::GetInstance().FindComp(uiname, compName).get());
		if (listBox) {
			listBox->EnableItemEvent(LuaUtils::checkunsigned(L, 3));
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int SetTooltipString(lua_State* L){
		const char* s = LuaUtils::checkstring(L, 1);
		UIManager::GetInstance().SetTooltipString(AnsiToWide(s));		
		auto injector = InputManager::GetInstance().GetInputInjector();
		UIManager::GetInstance().SetTooltipPos(injector->GetMouseNPos());
		return 0;
	}

	int ChangeUISizeX(lua_State* L){
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compoName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compoName);
		if (!comp) return 0;
		auto sizeX = LuaUtils::checkint(L, 3);
		comp->ChangeSizeX(sizeX);
		return 0;
	}
	int ChangeUISizeY(lua_State* L){
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compoName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compoName);
		if (!comp) return 0;
		auto sizeY = LuaUtils::checkint(L, 3);
		comp->ChangeSizeY(sizeY);
		return 0;
	}
	int ChangeUISize(lua_State* L){
		const char* uiname = LuaUtils::checkstring(L, 1);
		const char* compoName = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compoName);
		if (!comp) return 0;
		Vec2I size = LuaUtils::checkVec2I(L, 3);
		comp->ChangeSize(size);
		return 0;	
	}

	int SetEnableUIInput(lua_State* L){
		if (LuaUtils::isboolean(L, 1)){
			auto enable = LuaUtils::toboolean(L, 1);
			UIManager::GetInstance().EnableInputListener(enable);
		}
		return 0;
		
	}

	int SetCheckRadioBox(lua_State* L){
		auto uiname = LuaUtils::checkstring(L, 1);
		auto compname = LuaUtils::checkstring(L, 2);
		auto comp = std::dynamic_pointer_cast<RadioBox>(UIManager::GetInstance().FindComp(uiname, compname));
		assert(comp);
		comp->OnClickRadio(comp.get());
		comp->SetCheck(true);
		return 0;
	}

	int HasComponent(lua_State* L){
		auto uiname = LuaUtils::checkstring(L, 1);
		auto compname = LuaUtils::checkstring(L, 2);
		auto comp = UIManager::GetInstance().FindComp(uiname, compname);
		LuaUtils::pushboolean(L, comp != 0);
		return 1;
	}

	int GetScrollbarOffset(lua_State* L){
		auto uiname = LuaUtils::checkstring(L, 1);
		auto compname = LuaUtils::checkstring(L, 2);
		auto comp = std::dynamic_pointer_cast<Container>(UIManager::GetInstance().FindComp(uiname, compname));
		if (comp){
			auto offset = comp->GetScrollOffset();
			LuaUtils::pushVec2(L, offset);
			return 1;
		}
		return 0;
	}

	int SetScrollbarOffset(lua_State* L){
		auto uiname = LuaUtils::checkstring(L, 1);
		auto compname = LuaUtils::checkstring(L, 2);
		auto comp = std::dynamic_pointer_cast<Container>(UIManager::GetInstance().FindComp(uiname, compname));
		if (comp){
			Vec2 offset = LuaUtils::checkVec2(L, 3);
			comp->SetScrollOffset(offset);
		}
		return 0;
	}

	int SetScrollbarEnd(lua_State* L) {
		auto uiname = LuaUtils::checkstring(L, 1);
		auto compname = LuaUtils::checkstring(L, 2);
		auto comp = std::dynamic_pointer_cast<Container>(UIManager::GetInstance().FindComp(uiname, compname));
		if (comp) {			
			comp->SetScrollOffset(Vec2(0, -FLT_MAX));
		}
		return 0;
	}

	int GetContentEndY(lua_State* L) {
		auto uiname = LuaUtils::checkstring(L, 1);
		auto compname = LuaUtils::checkstring(L, 2);
		auto comp = std::dynamic_pointer_cast<Container>(UIManager::GetInstance().FindComp(uiname, compname));
		if (comp) {
			auto end = comp->GetChildrenContentEndInPixel();
			LuaUtils::pushinteger(L, end);
			return 1;
		}
		LuaUtils::pushinteger(L, 0);
		return 1;
	}

	int RemoveGapChildren(lua_State* L) {
		auto uiname = LuaUtils::checkstring(L, 1);
		auto compname = LuaUtils::checkstring(L, 2);
		auto comp = std::dynamic_pointer_cast<Container>(UIManager::GetInstance().FindComp(uiname, compname));
		if (comp) {
			comp->RemoveGapChildren();
		}
		return 0;
	}

	int GetNumUIChildren(lua_State* L) {
		auto uiname = LuaUtils::checkstring(L, 1);
		auto compname = LuaUtils::checkstring(L, 2);
		auto container = std::dynamic_pointer_cast<Container>(UIManager::GetInstance().FindComp(uiname, compname));
		if (container) {
			LuaUtils::pushinteger(L, container->GetNumChildren());			
		}
		else {
			LuaUtils::pushinteger(L, 0);
		}
		return 1;
	}

	int GetChildrenNames(lua_State* L) {
		auto uiname = LuaUtils::checkstring(L, 1);
		auto compname = LuaUtils::checkstring(L, 2);
		auto container = std::dynamic_pointer_cast<Container>(UIManager::GetInstance().FindComp(uiname, compname));
		LuaObject t;
		t.NewTable(L);
		if (container) {
			container->GetChildrenNames(t);
			
		}
		t.PushToStack();
		return 1;
	}

	int MoveUpListBoxItems(lua_State* L) {
		auto uiname = LuaUtils::checkstring(L, 1);
		auto comp = LuaUtils::checkstring(L, 2);
		std::vector<unsigned> ids;
		{
			LuaObject t(L, 3);
			LuaObject data;
			auto it = t.GetSequenceIterator();
			while (it.GetNext(data)) {
				auto id = data.GetUnsigned();
				ids.push_back(id);
			}
			auto listbox = std::dynamic_pointer_cast<ListBox>(UIManager::GetInstance().FindComp(uiname, comp));
			if (listbox) {
				listbox->MoveUpListBoxItems(ids);
			}
			return 0;
		}		
	}

	int MoveDownListBoxItems(lua_State* L) {
		auto uiname = LuaUtils::checkstring(L, 1);
		auto comp = LuaUtils::checkstring(L, 2);
		std::vector<unsigned> ids;
		{
			LuaObject t(L, 3);
			LuaObject data;
			auto it = t.GetSequenceIterator();
			while (it.GetNext(data)) {
				auto id = data.GetUnsigned();
				ids.push_back(id);
			}
			auto listbox = std::dynamic_pointer_cast<ListBox>(UIManager::GetInstance().FindComp(uiname, comp));
			if (listbox) {
				listbox->MoveDownListBoxItems(ids);
			}
			return 0;
		}
	}

	int RemoveListBoxItems(lua_State* L) {
		auto ui = LuaUtils::checkstring(L, 1);
		auto comp = LuaUtils::checkstring(L, 2);
		auto listbox = std::dynamic_pointer_cast<ListBox>(UIManager::GetInstance().FindComp(ui, comp));
		if (!listbox) {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString(
				"Listbox(%s:%s) is not found", ui, comp).c_str());
			return 0;
		}
		std::vector<unsigned> ids;
		{
			LuaObject t(L, 3);
			LuaObject data;
			auto it = t.GetSequenceIterator();
			while(it.GetNext(data)) {
				ids.push_back(data.GetUnsigned(-1));
			}
		}
		listbox->RemoveDataWithKeys(ids);
		return 0;
	}

	int RemoveListBoxItemWithRowIndex(lua_State* L) {
		auto ui = LuaUtils::checkstring(L, 1);
		auto comp = LuaUtils::checkstring(L, 2);
		auto listbox = std::dynamic_pointer_cast<ListBox>(UIManager::GetInstance().FindComp(ui, comp));
		if (!listbox) {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString(
				"Listbox(%s:%s) is not found", ui, comp).c_str());
			return 0;
		}
		auto rowIndex = LuaUtils::checkunsigned(L, 3);
		listbox->RemoveDataWithRowIndex(rowIndex);
		return 0;
	}

	int GetComponentFinalPos(lua_State* L) {
		auto ui = LuaUtils::checkstring(L, 1);
		auto comp = LuaUtils::checkstring(L, 2);
		auto winbase = UIManager::GetInstance().FindComp(ui, comp);
		if (winbase) {
			luaU_push<Vec2ITuple>(L, winbase->GetFinalPos().ToTuple());
			return 1;
		}
		return 0;
	}

	int SetEnableUIRenderTarget(lua_State* L) {
		auto ui = LuaUtils::checkstring(L, 1);
		auto comp = LuaUtils::checkstring(L, 2);
		auto img = std::dynamic_pointer_cast<ImageBox>(UIManager::GetInstance().FindComp(ui, comp));
		if (img) {
			bool enable = LuaUtils::toboolean(L, 3);
			img->SetEnableRenderTarget(enable);				
		}		
		return 0;
	}

	int SetLockUIVisibility(lua_State* L) {
		auto ui = LuaUtils::checkstring(L, 1);
		bool b = LuaUtils::toboolean(L, 2);
		UIManager::GetInstance().SetLockUIVisibility(ui, b);
		return 0;
	}

	void RegisterLuaEnums(lua_State* mL) {
		ListItemDataType::RegisterToLua(mL);
	}

	int DisplayCursor(lua_State* L) {
		auto cursorType = LuaUtils::checkstring(L, 1);
		UIManager::GetInstance().DisplayCursor(CursorType::ConvertToEnum(cursorType));
		return 0;
	}

	//--------------------------------------------------------------------------------
	void RegisterLuaFuncs(lua_State* mL)
	{
		LUA_SETCFUNCTION(mL, DisplayCursor);
		LUA_SETCFUNCTION(mL, SetLockUIVisibility);
		LUA_SETCFUNCTION(mL, SetEnableUIRenderTarget);
		LUA_SETCFUNCTION(mL, GetComponentFinalPos);
		LUA_SETCFUNCTION(mL, RemoveListBoxItemWithRowIndex);
		LUA_SETCFUNCTION(mL, RemoveListBoxItems);
		LUA_SETCFUNCTION(mL, MoveDownListBoxItems);
		LUA_SETCFUNCTION(mL, MoveUpListBoxItems);
		LUA_SETCFUNCTION(mL, SetCheckRadioBox);
		LUA_SETCFUNCTION(mL, SetEnableUIInput);

		LUA_SETCFUNCTION(mL, GetNumUIChildren);
		LUA_SETCFUNCTION(mL, GetChildrenNames);
		LUA_SETCFUNCTION(mL, RemoveGapChildren);
		LUA_SETCFUNCTION(mL, GetContentEndY);
		LUA_SETCFUNCTION(mL, SetScrollbarEnd);
		LUA_SETCFUNCTION(mL, SetScrollbarOffset);
		LUA_SETCFUNCTION(mL, GetScrollbarOffset);
		LUA_SETCFUNCTION(mL, HasComponent);
		LUA_SETCFUNCTION(mL, ChangeUISizeX);
		LUA_SETCFUNCTION(mL, ChangeUISizeY);
		LUA_SETCFUNCTION(mL, ChangeUISize);

		LUA_SETCFUNCTION(mL, SetTooltipString);

		//----------------------------------------------------------------------------
		// Listbox
		//--------------------------------------------------------------------------------
		LUA_SETCFUNCTION(mL, DisableListBoxItemEvent);
		LUA_SETCFUNCTION(mL, SetListBoxItemProperty);
		LUA_SETCFUNCTION(mL, GetLastChangedRow);
		LUA_SETCFUNCTION(mL, SwapListBoxItem);
		LUA_SETCFUNCTION(mL, GetNumListBoxData);
		LUA_SETCFUNCTION(mL, StopUIAnimationForListItem);
		LUA_SETCFUNCTION(mL, StartUIAnimationForListItem);
		LUA_SETCFUNCTION(mL, SetListItemProperty);
		LUA_SETCFUNCTION(mL, GetListItemDataCachedListBox);
		LUA_SETCFUNCTION(mL, GetListItemData);
		LUA_SETCFUNCTION(mL, SelectListBoxItemsById);
		LUA_SETCFUNCTION(mL, SelectListBoxItems);
		LUA_SETCFUNCTION(mL, SelectListBoxItem);
		LUA_SETCFUNCTION(mL, GetListBoxRowIndex);
		LUA_SETCFUNCTION(mL, GetListBoxRowIndexCached);
		LUA_SETCFUNCTION(mL, GetListBoxNumericKey);
		LUA_SETCFUNCTION(mL, GetListBoxStringKey);
		LUA_SETCFUNCTION(mL, GetListBoxNumericKeyCached);
		LUA_SETCFUNCTION(mL, GetListBoxStringKeyCached);
		LUA_SETCFUNCTION(mL, GetListBoxSelectedNumericKeys);
		LUA_SETCFUNCTION(mL, GetListBoxSelectedStringKeys);
		LUA_SETCFUNCTION(mL, GetListBoxSelectedRows);
		LUA_SETCFUNCTION(mL, AddPropertyItem);
		LUA_SETCFUNCTION(mL, SetListBoxItem);
		LUA_SETCFUNCTION(mL, InsertListBoxItem);
		LUA_SETCFUNCTION(mL, CacheListBox);
		LUA_SETCFUNCTION(mL, SortPropertyList);
		LUA_SETCFUNCTION(mL, ClearPropertyItems);
		LUA_SETCFUNCTION(mL, ClearListBox);


		LUA_SETCFUNCTION(mL, ModifyDropDownItem);
		LUA_SETCFUNCTION(mL, GetUIEventName);
		LUA_SETCFUNCTION(mL, GetNumUIEvents);
		LUA_SETCFUNCTION(mL, GetPropertyCurKeyValue);
		LUA_SETCFUNCTION(mL, GetUIPropertyName);
		LUA_SETCFUNCTION(mL, GetNumUIProperties);
		LUA_SETCFUNCTION(mL, GetUIPath);
		LUA_SETCFUNCTION(mL, GetUIScriptPath);
		LUA_SETCFUNCTION(mL, SetEnableComponent);
		LUA_SETCFUNCTION(mL, StartHighlightUI);
		LUA_SETCFUNCTION(mL, StopHighlightUI);
		LUA_SETCFUNCTION(mL, SetHighlightUIComp);
		LUA_SETCFUNCTION(mL, HideUIsExcept);
		LUA_SETCFUNCTION(mL, SetColorRampUIValues);
		LUA_SETCFUNCTION(mL, GetColorRampUIValues);
		LUA_SETCFUNCTION(mL, SetVisibleLuaUIWithoutFocusing);
		LUA_SETCFUNCTION(mL, GetDropDownIndex);
		LUA_SETCFUNCTION(mL, GetDropDownString);
		LUA_SETCFUNCTION(mL, ClearDropDownItem);
		LUA_SETCFUNCTION(mL, AddDropDownItem);
		LUA_SETCFUNCTION(mL, GetDropDownKey);
		LUA_SETCFUNCTION(mL, SetDropDownIndex);
		LUA_SETCFUNCTION(mL, MoveUIToBottom);
		LUA_SETCFUNCTION(mL, MoveUIToTop);
		LUA_SETCFUNCTION(mL, SetNumericUpDownValue);
		LUA_SETCFUNCTION(mL, GetNumericUpDownValue);
		LUA_SETCFUNCTION(mL, GetCheckedFromCheckBox);
		LUA_SETCFUNCTION(mL, MatchUIHeight);
		LUA_SETCFUNCTION(mL, SetVisibleLuaUI);
		LUA_SETCFUNCTION(mL, ToggleVisibleLuaUI);
		LUA_SETCFUNCTION(mL, CloseAllLuaUI);
		LUA_SETCFUNCTION(mL, GetVisibleLuaUI);
		LUA_SETCFUNCTION(mL, SetVisibleComponent);
		LUA_SETCFUNCTION(mL, SetVisibleChildrenComponent);
		LUA_SETCFUNCTION(mL, LoadLuaUI);
		LUA_SETCFUNCTION(mL, IsLoadedUI);
		LUA_SETCFUNCTION(mL, CloneLuaUI);
		LUA_SETCFUNCTION(mL, AddLuaUI);
		LUA_SETCFUNCTION(mL, DeleteLuaUI);
		LUA_SETCFUNCTION(mL, SetStaticText);
		LUA_SETCFUNCTION(mL, RemoveAllChildrenOf);
		LUA_SETCFUNCTION(mL, RemoveComponent);
		LUA_SETCFUNCTION(mL, AddComponent);
		LUA_SETCFUNCTION(mL, AddComponentTabWindow);
		LUA_SETCFUNCTION(mL, CreateNewCard);
		LUA_SETCFUNCTION(mL, DeleteCard);
		LUA_SETCFUNCTION(mL, DeleteAllCard);
		LUA_SETCFUNCTION(mL, IsExistingCard);
		LUA_SETCFUNCTION(mL, BlinkButton);
		LUA_SETCFUNCTION(mL, UpdateButtonProgressBar);
		LUA_SETCFUNCTION(mL, StartButtonProgressBar);
		LUA_SETCFUNCTION(mL, EndButtonProgressBar);
		LUA_SETCFUNCTION(mL, SetTextBoxText);
		LUA_SETCFUNCTION(mL, SetUIBackground);
		LUA_SETCFUNCTION(mL, SetUIPropertyCached);
		LUA_SETCFUNCTION(mL, CacheUIComponent);
		LUA_SETCFUNCTION(mL, SetUIProperty);
		LUA_SETCFUNCTION(mL, GetUIProperty);
		LUA_SETCFUNCTION(mL, SetCardUIProperty);
		LUA_SETCFUNCTION(mL, RemoveUIEventhandler);
		LUA_SETCFUNCTION(mL, GetMousePos);
		LUA_SETCFUNCTION(mL, GetComponentWidth);
		LUA_SETCFUNCTION(mL, FindAndRememberComponent);
		LUA_SETCFUNCTION(mL, SetActivationUIAnim);
		LUA_SETCFUNCTION(mL, SetFocusUI);
		LUA_SETCFUNCTION(mL, IsButtonActivated);
		LUA_SETCFUNCTION(mL, StartUIAnimation);
		LUA_SETCFUNCTION(mL, StopUIAnimation);
		LUA_SETCFUNCTION(mL, GetUIPosSize);

		ListItemDataType::RegisterToLua(mL);

	}
}