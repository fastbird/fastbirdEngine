#include <UI/StdAfx.h>
#include <UI/UIManager.h>
#include <UI/StaticText.h>
#include <UI/Button.h>
#include <UI/ListBox.h>
#include <UI/ListItem.h>
#include <UI/ListItemDataType.h>
#include <UI/ListBoxData.h>
#include <UI/PropertyList.h>
#include <UI/TextBox.h>
#include <UI/CheckBox.h>
#include <UI/NumericUpDown.h>
#include <UI/DropDown.h>
#include <UI/ColorRampComp.h>
#include <UI/CardScroller.h>
#include <UI/HorizontalGauge.h>
//--------------------------------------------------------------------------------

namespace fastbird
{
	int LoadLuaUI(lua_State* L);
	int IsLoadedUI(lua_State* L);
	int CloneLuaUI(lua_State* L);
	int AddLuaUI(lua_State* L);
	int DeleteLuaUI(lua_State* L);
	int SetStaticText(lua_State* L);
	int SetVisibleLuaUI(lua_State* L);
	int ToggleVisibleLuaUI(lua_State* L);
	int CloseAllLuaUI(lua_State* L);
	int SetVisibleComponent(lua_State* L);
	int SetVisibleChildrenComponent(lua_State* L);
	int GetVisibleLuaUI(lua_State* L);
	int RemoveAllChildrenOf(lua_State* L);
	int RemoveComponent(lua_State* L);
	int AddComponent(lua_State* L);
	int CreateNewCard(lua_State* L);
	int GetCardUIId(lua_State* L);
	int BlinkButton(lua_State* L);
	int UpdateButtonProgressBar(lua_State* L);
	int StartButtonProgressBar(lua_State* L);
	int EndButtonProgressBar(lua_State* L);
	int SetTextBoxText(lua_State* L);
	int SetUIBackground(lua_State* L);
	int SetUIProperty(lua_State* L);
	int GetUIProperty(lua_State* L);
	int RemoveUIEventhandler(lua_State* L);
	int GetMousePos(lua_State* L);
	int GetComponentWidth(lua_State* L);
	int FindAndRememberComponent(lua_State* L);
	int SetActivationUIAnim(lua_State* L);
	int SetFocusUI(lua_State* L);
	int IsButtonActivated(lua_State* L);
	int StartUIAnimation(lua_State* L);
	int StopUIAnimation(lua_State* L);
	int MatchUIHeight(lua_State* L);
	int GetCheckedFromCheckBox(lua_State* L);
	int GetNumericUpDownValue(lua_State* L);
	int SetNumericUpDownValue(lua_State* L);
	int MoveUIToBottom(lua_State* L);
	int SetDropDownIndex(lua_State* L);
	int GetDropDownIndex(lua_State* L);
	int SetVisibleLuaUIWithoutFocusing(lua_State* L);
	int GetColorRampUIValues(lua_State* L);
	int SetColorRampUIValues(lua_State* L);
	int HideUIsExcept(lua_State* L);
	int StartHighlightUI(lua_State* L);
	int StopHighlightUI(lua_State* L);
	int SetEnableComponent(lua_State* L);
	int GetUIPath(lua_State* L);
	int GetUIScriptPath(lua_State* L);
	int GetNumUIProperties(lua_State* L);
	int GetUIPropertyName(lua_State* L);
	int GetPropertyCurKeyValue(lua_State* L);	
	int GetNumUIEvents(lua_State* L);
	int GetUIEventName(lua_State* L);

	// listbox
	int ClearListBox(lua_State* L);
	int ClearPropertyItems(lua_State* L);
	int SortPropertyList(lua_State* L);
	int CacheListBox(lua_State* L);
	int InsertListBoxItem(lua_State* L); // string or numeric key
	int SetListBoxItem(lua_State* L);
	int AddPropertyItem(lua_State* L);
	int GetListBoxSelectedRows(lua_State* L);
	int GetListBoxSelectedStringKeys(lua_State* L);
	int GetListBoxSelectedNumericKeys(lua_State* L);
	int GetListBoxStringKeyCached(lua_State* L);
	int GetListBoxNumericKeyCached(lua_State* L);
	int GetListBoxRowIndexCached(lua_State* L);
	int SelectListBoxItem(lua_State* L);
	int SelectListBoxItems(lua_State* L);	
	int GetListItemData(lua_State* L);	
	int GetListItemDataCachedListBox(lua_State* L);
	int SetListItemProperty(lua_State* L);
	int StartUIAnimationForListItem(lua_State* L);
	int StopUIAnimationForListItem(lua_State* L);	
	int GetNumListBoxData(lua_State* L);
	int SwapListBoxItem(lua_State* L);

	// etc
	int SetTooltipString(lua_State* L);

	void RegisterLuaEnums(lua_State* mL){
		ListItemDataType::RegisterToLua(mL);
	}
	//--------------------------------------------------------------------------------
	void RegisterLuaFuncs(lua_State* mL)
	{
		LUA_SETCFUNCTION(mL, SetTooltipString);

		//----------------------------------------------------------------------------
		// Listbox
		//--------------------------------------------------------------------------------
		LUA_SETCFUNCTION(mL, SwapListBoxItem);
		LUA_SETCFUNCTION(mL, GetNumListBoxData);
		LUA_SETCFUNCTION(mL, StopUIAnimationForListItem);
		LUA_SETCFUNCTION(mL, StartUIAnimationForListItem);		
		LUA_SETCFUNCTION(mL, SetListItemProperty);
		LUA_SETCFUNCTION(mL, GetListItemDataCachedListBox);
		LUA_SETCFUNCTION(mL, GetListItemData);
		LUA_SETCFUNCTION(mL, SelectListBoxItems);
		LUA_SETCFUNCTION(mL, SelectListBoxItem);		
		LUA_SETCFUNCTION(mL, GetListBoxRowIndexCached);
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
		LUA_SETCFUNCTION(mL, HideUIsExcept);
		LUA_SETCFUNCTION(mL, SetColorRampUIValues);
		LUA_SETCFUNCTION(mL, GetColorRampUIValues);
		LUA_SETCFUNCTION(mL, SetVisibleLuaUIWithoutFocusing);
		LUA_SETCFUNCTION(mL, GetDropDownIndex);
		LUA_SETCFUNCTION(mL, SetDropDownIndex);
		LUA_SETCFUNCTION(mL, MoveUIToBottom);
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
		LUA_SETCFUNCTION(mL, CreateNewCard);
		LUA_SETCFUNCTION(mL, GetCardUIId);
		LUA_SETCFUNCTION(mL, BlinkButton);
		LUA_SETCFUNCTION(mL, UpdateButtonProgressBar);
		LUA_SETCFUNCTION(mL, StartButtonProgressBar);
		LUA_SETCFUNCTION(mL, EndButtonProgressBar);
		LUA_SETCFUNCTION(mL, SetTextBoxText);
		LUA_SETCFUNCTION(mL, SetUIBackground);
		LUA_SETCFUNCTION(mL, SetUIProperty);
		LUA_SETCFUNCTION(mL, GetUIProperty);
		LUA_SETCFUNCTION(mL, RemoveUIEventhandler);
		LUA_SETCFUNCTION(mL, GetMousePos);
		LUA_SETCFUNCTION(mL, GetComponentWidth);
		LUA_SETCFUNCTION(mL, FindAndRememberComponent);
		LUA_SETCFUNCTION(mL, SetActivationUIAnim);
		LUA_SETCFUNCTION(mL, SetFocusUI);		
		LUA_SETCFUNCTION(mL, IsButtonActivated);
		LUA_SETCFUNCTION(mL, StartUIAnimation);
		LUA_SETCFUNCTION(mL, StopUIAnimation);

		ListItemDataType::RegisterToLua(mL);

	}

	int LoadLuaUI(lua_State* L)
	{
		const char* uiFile = luaL_checkstring(L, 1);
		HWND_ID id = INVALID_HWND_ID;
		if (lua_gettop(L) == 2)
		{
			id = luaL_checkunsigned(L, 2);
		}
		else
		{
			id = gFBEnv->pEngine->GetMainWndHandleId();
		}
		std::vector<IWinBase*> temp;
		std::string name;
		UIManager::GetUIManagerStatic()->ParseUI(uiFile, temp, name, id, true);
		UIManager::GetUIManagerStatic()->SetVisible(name.c_str(), false);
		return 0;
	}

	int IsLoadedUI(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		bool loaded = UIManager::GetUIManagerStatic()->IsLoadedUI(uiname);
		lua_pushboolean(L, loaded);
		return 1;
	}

	int AddLuaUI(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		LuaObject ui(L, 2);
		HWND_ID hwndId = INVALID_HWND_ID;
		if (lua_gettop(L) == 3)
		{
			hwndId = luaL_checkunsigned(L, 2);
		}
		else
		{
			hwndId = gFBEnv->pEngine->GetMainWndHandleId();
		}
		UIManager::GetUIManagerStatic()->AddLuaUI(uiname, ui, hwndId);
		UIManager::GetUIManagerStatic()->SetVisible(uiname, false);
		return 0;
	}

	int DeleteLuaUI(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		UIManager::GetUIManagerStatic()->DeleteLuaUI(uiname);
		return 0;
	}

	int SetStaticText(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compoName = luaL_checkstring(L, 2);
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, compoName);
		auto staticText = dynamic_cast<StaticText*>(comp);
		if (staticText)
		{
			staticText->SetText(AnsiToWide(luaL_checkstring(L, 3)));
			lua_pushboolean(L, 1);
		}
		else
		{
			lua_pushboolean(L, 0);
		}
		return 1;
	}

	int SetVisibleLuaUI(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		luaL_checktype(L, 2, LUA_TBOOLEAN);
		bool visible = lua_toboolean(L, 2) != 0;
		UIManager::GetUIManagerStatic()->SetVisible(uiname, visible);
		return 0;
	}

	int ToggleVisibleLuaUI(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		UIManager::GetUIManagerStatic()->ToggleVisibleLuaUI(uiname);
		return 0;
	}

	int CloseAllLuaUI(lua_State* L)
	{
		UIManager::GetUIManagerStatic()->CloseAllLuaUI();
		return 0;
	}

	int GetVisibleLuaUI(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		bool visible = UIManager::GetUIManagerStatic()->GetVisible(uiname);
		lua_pushboolean(L, visible);
		return 1;
	}

	int SetVisibleComponent(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		luaL_checktype(L, 3, LUA_TBOOLEAN);
		bool visible = lua_toboolean(L, 3) != 0;
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, compName);
		if (comp)
		{
			comp->SetVisible(visible);
			return 0;
		}

		return 0;
	}

	int SetVisibleChildrenComponent(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		luaL_checktype(L, 3, LUA_TBOOLEAN);
		bool visible = lua_toboolean(L, 3) != 0;
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, compName);
		if (comp)
		{
			comp->SetVisibleChildren(visible);
			return 0;
		}

		return 0;
	}

	int RemoveAllChildrenOf(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, compName);
		if (comp)
		{
			comp->RemoveAllChild(false);
			lua_pushboolean(L, 1);
		}
		else
		{
			lua_pushboolean(L, 0);
		}
		return 1;
	}

	int RemoveComponent(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, compName);
		if (comp)
		{
			auto parent = comp->GetParent();
			if (parent)
			{
				parent->RemoveChild(comp);
			}
			else
			{
				Error(DEFAULT_DEBUG_ARG, "Cannot remove root component.");
			}
		}
		return 0;
	}

	int AddComponent(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, compName);
		LuaObject compTable(L, 3);
		if (comp)
		{
			//UIManager::GetUIManagerStatic()->SetEnablePosSizeEvent(false);
			auto winbase = comp->AddChild(compTable);
			//UIManager::GetUIManagerStatic()->SetEnablePosSizeEvent(true);
			if (winbase)
			{
				//winbase->OnSizeChanged();
				lua_pushboolean(L, 1);
				winbase->SetVisible(true);
				return 1;
			}
		}

		lua_pushboolean(L, 0);
		return 1;
	}

	int CreateNewCard(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* cardScrollerName = luaL_checkstring(L, 2);
		LuaObject card(L, 3);
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, cardScrollerName);
		if (!comp)
		{
			Error(DEFAULT_DEBUG_ARG, "No card scroller found!");
			return 0;
		}

		CardScroller* cardScroller = (CardScroller*)comp;
		cardScroller->AddCard(card);
		return 0;
	}

	int GetCardUIId(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* cardScrollerName = luaL_checkstring(L, 2);
		const char* cardName = luaL_checkstring(L, 3);
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, cardScrollerName);
		if (comp)
		{
			auto cardComp = comp->GetChild(cardName);
			if (cardComp)
			{
				CardItem* card = (CardItem*)cardComp;
				lua_pushunsigned(L, card->GetCardId());
				return 1;
			}
		}		

		lua_pushunsigned(L, -1);
		return 1;
	}

	int BlinkButton(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		luaL_checktype(L, 3, LUA_TBOOLEAN);
		bool blink = lua_toboolean(L, 3) != 0;
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, compName);
		if (comp)
		{
			auto button = dynamic_cast<Button*>(comp);
			if (button)
			{
				if (lua_gettop(L) == 4){
					float time = (float)luaL_checknumber(L, 4);
					button->Blink(blink, time);
				}
				else{
					button->Blink(blink);
				}
				
				return 0;
			}
			else
			{
				auto bar = dynamic_cast<HorizontalGauge*>(comp);
				if (bar){
					if (lua_gettop(L) == 4){
						float time = (float)luaL_checknumber(L, 4);
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
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		float percent = (float)luaL_checknumber(L, 3);
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, compName);
		if (comp)
		{
			auto button = dynamic_cast<Button*>(comp);
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
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, compName);
		if (comp)
		{
			auto button = dynamic_cast<Button*>(comp);
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
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, compName);
		if (comp)
		{
			auto button = dynamic_cast<Button*>(comp);
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
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		const char* text = luaL_checkstring(L, 3);
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, compName);
		if (comp)
		{
			auto textBox = dynamic_cast<TextBox*>(comp);
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
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		const char* image = luaL_checkstring(L, 3);
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, compName);
		if (comp)
		{
			comp->SetProperty(UIProperty::BACKGROUND_IMAGE_NOATLAS, image);
			return 0;
		}
		assert(0);
		return 0;
	}

	int SetUIProperty(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		const char* prop = luaL_checkstring(L, 3);
		const char* val = luaL_checkstring(L, 4);
		UIManager::GetUIManagerStatic()->SetUIProperty(uiname, compName, prop, val);
		return 0;

	}

	int GetUIProperty(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		const char* prop = luaL_checkstring(L, 3);
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, compName);
		if (comp)
		{
			char buf[256];

			bool result = comp->GetProperty(UIProperty::ConvertToEnum(prop), buf, 256, false);
			if (result)
			{
				lua_pushstring(L, buf);
				return 1;
			}
		}
		return 0;
	}

	int RemoveUIEventhandler(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		const char* eventName = luaL_checkstring(L, 3);
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, compName);
		if (comp)
		{
			auto eventHandler = dynamic_cast<EventHandler*>(comp);
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
		gFBEnv->pEngine->GetMousePos(x, y);

		lua_pushnumber(L, x);
		lua_pushnumber(L, y);
		return 2;
	}

	int GetComponentWidth(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, compName);
		if (comp)
		{
			lua_pushinteger(L, comp->GetSize().x);
			return 1;
		}
		assert(0);
		Error("UI component not found! (%s, %s)", uiname, compName);
		lua_pushinteger(L, 0);
		return 1;
	}
	IWinBase* gRememberedComp = 0;
	int FindAndRememberComponent(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		gRememberedComp = UIManager::GetUIManagerStatic()->FindComp(uiname, compName);
		lua_pushboolean(L, gRememberedComp != 0);
		return 1;
	}

	int SetActivationUIAnim(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, compName);
		if (!comp)
		{
			Error(DEFAULT_DEBUG_ARG, "no comp found");
			return 0;
		}
		const char* animName = luaL_checkstring(L, 3);
		bool active = lua_toboolean(L, 4) != 0;

		auto anim = comp->GetOrCreateUIAnimation(animName);
		anim->SetActivated(active);
		return 0;
	}

	int SetFocusUI(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		UIManager::GetUIManagerStatic()->SetFocusUI(uiname);
		return 0;
	}

	int CloneLuaUI(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* newUIname = luaL_checkstring(L, 2);
		gFBEnv->pUIManager->CloneUI(uiname, newUIname);
		return 0;
	}

	int IsButtonActivated(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto comp = gFBEnv->pUIManager->FindComp(uiname, compName);
		Button* btn = dynamic_cast<Button*>(comp);
		if (btn)
		{
			bool activated = btn->IsActivated();
			lua_pushboolean(L, activated);
			return 1;
		}

		return 0;
	}

	int StartUIAnimation(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto comp = gFBEnv->pUIManager->FindComp(uiname, compName);
		const char* animName = luaL_checkstring(L, 3);

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
				auto ganim = gFBEnv->pUIManager->GetGlobalAnimation(animName);
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
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto comp = gFBEnv->pUIManager->FindComp(uiname, compName);
		const char* animName = luaL_checkstring(L, 3);

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
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		bool checkName = false;
		if (!lua_isnil(L, 3))
			checkName = lua_toboolean(L, 3)!=0;

		auto comp = gFBEnv->pUIManager->FindComp(uiname, compName);
		if (comp)
		{
			Container* con = dynamic_cast<Container*>(comp);
			if (con)
			{
				con->MatchHeight(checkName);
			}
		}
		return 0;
	}

	int GetCheckedFromCheckBox(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto comp = dynamic_cast<CheckBox*>(gFBEnv->pUIManager->FindComp(uiname, compName));
		if (comp)
		{
			lua_pushboolean(L, comp->GetCheck());
			return 1;
		}
		return 0;
	}

	int GetNumericUpDownValue(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto comp = dynamic_cast<NumericUpDown*>(gFBEnv->pUIManager->FindComp(uiname, compName));
		if (comp)
		{
			lua_pushinteger(L, comp->GetValue());
			return 1;
		}
		return 0;
	}

	int SetNumericUpDownValue(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto comp = dynamic_cast<NumericUpDown*>(gFBEnv->pUIManager->FindComp(uiname, compName));
		if (comp)
		{
			int val = luaL_checkint(L, 3);
			comp->SetNumber(val);
			return 0;
		}
		return 0;
	}

	int MoveUIToBottom(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		gFBEnv->pUIManager->MoveToBottom(uiname);
		return 0;
	}

	int SetDropDownIndex(lua_State* L)
	{
		unsigned index = luaL_checkunsigned(L, 1);
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto dropDown = dynamic_cast<DropDown*>(gFBEnv->pUIManager->FindComp(uiname, compName));
		if (dropDown)
		{
			dropDown->SetSelectedIndex(index);
		}
		return 0;
	}

	int GetDropDownIndex(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto dropDown = dynamic_cast<DropDown*>(gFBEnv->pUIManager->FindComp(uiname, compName));
		if (dropDown)
		{
			lua_pushunsigned(L, dropDown->GetSelectedIndex());
			return 1;
		}
		return 0;
	}

	int SetVisibleLuaUIWithoutFocusing(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		luaL_checktype(L, 2, LUA_TBOOLEAN);
		bool visible = lua_toboolean(L, 2) != 0;
		UIManager::GetUIManagerStatic()->LockFocus(true);
		UIManager::GetUIManagerStatic()->SetVisible(uiname, visible);
		UIManager::GetUIManagerStatic()->LockFocus(false);
		return 0;
	}

	int GetColorRampUIValues(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto colorRamp = dynamic_cast<ColorRampComp*>(gFBEnv->pUIManager->FindComp(uiname, compName));
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
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto colorRamp = dynamic_cast<ColorRampComp*>(gFBEnv->pUIManager->FindComp(uiname, compName));
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
		UIManager::GetUIManagerStatic()->HideUIsExcept(excepts);
		return 0;
	}

	int StartHighlightUI(lua_State* L)
	{
		auto uiname = luaL_checkstring(L, 1);
		gFBEnv->pUIManager->HighlightUI(uiname);
		return 0;
	}

	int StopHighlightUI(lua_State* L)
	{
		auto uiname = luaL_checkstring(L, 1);
		gFBEnv->pUIManager->StopHighlightUI(uiname);
		return 0;
	}

	int SetEnableComponent(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		if (!lua_isboolean(L, 3))
		{
			Error(DEFAULT_DEBUG_ARG, "Invalid param.");
			return 0;
		}
		bool enable = lua_toboolean(L, 3)!=0;
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, compName);
		if (comp)
		{
			comp->SetEnable(enable);
		}
		return 0;
	}

	int GetUIPath(lua_State* L)
	{
		auto uiname = luaL_checkstring(L, 1);
		const char* path = gFBUIManager->GetUIPath(uiname);
		if (path)
		{
			lua_pushstring(L, path);
			return 1;
		}
		return 0;
	}

	int GetUIScriptPath(lua_State* L)
	{
		auto uiname = luaL_checkstring(L, 1);
		const char* path = gFBUIManager->GetUIScriptPath(uiname);
		if (path)
		{
			lua_pushstring(L, path);
			return 1;
		}
		return 0;
	}

	int GetNumUIProperties(lua_State* L)
	{
		lua_pushinteger(L, UIProperty::COUNT);
		return 1;
	}

	int GetUIPropertyName(lua_State* L)
	{
		unsigned i = luaL_checkunsigned(L, 1);
		assert(i < UIProperty::COUNT);
		lua_pushstring(L, UIProperty::ConvertToString(i));
		return 1;
	}

	int GetPropertyCurKeyValue(lua_State* L)
	{
		auto uiname = luaL_checkstring(L, 1);
		auto compName = luaL_checkstring(L, 2);
		auto comp = gFBUIManager->FindComp(uiname, compName);
		if (comp && comp->GetType() == ComponentType::PropertyList)
		{
			auto propertyList = (PropertyList*)comp;
			std::string key, value;
			propertyList->GetCurKeyValue(key, value);
			lua_pushstring(L, key.c_str());
			lua_pushstring(L, value.c_str());
			return 2;
		}
		return 0;

	}

	int GetNumUIEvents(lua_State* L){
		lua_pushinteger(L, UIEvents::EVENT_NUM);
		return 1;
	}

	int GetUIEventName(lua_State* L){
		UIEvents::Enum e = (UIEvents::Enum)luaL_checkunsigned(L, 1);
		lua_pushstring(L, UIEvents::ConvertToString(e));
		return 1;
	}


	//-----------------------------------------------------------------------
	// Listbox
	//-----------------------------------------------------------------------
	int ClearListBox(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compoName = luaL_checkstring(L, 2);
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, compoName);
		auto listBox = dynamic_cast<ListBox*>(comp);
		if (listBox)
		{
			listBox->Clear();
			lua_pushboolean(L, 1);
		}
		else
		{
			lua_pushboolean(L, 0);
		}
		return 1;
	}

	//-----------------------------------------------------------------------
	int ClearPropertyItems(lua_State* L)
	{
		auto uiname = luaL_checkstring(L, 1);
		auto compName = luaL_checkstring(L, 2);
		auto comp = gFBUIManager->FindComp(uiname, compName);
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
		auto uiname = luaL_checkstring(L, 1);
		auto compName = luaL_checkstring(L, 2);
		auto comp = gFBUIManager->FindComp(uiname, compName);
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
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		bool found = gFBUIManager->CacheListBox(uiname, compName);
		lua_pushboolean(L, found);
		return 1;
	}

	//-----------------------------------------------------------------------
	int InsertListBoxItem(lua_State* L) // string or numeric key
	{
		auto listbox = gFBUIManager->GetCachedListBox();
		if (!listbox)
			return 0;

		if (lua_isnumber(L, 1))
		{
			unsigned key = luaL_checkunsigned(L, 1);
			unsigned index = listbox->InsertItem(key);
			lua_pushunsigned(L, index);
			return 1;
		}
		else if (lua_isstring(L, 1))
		{
			const char* key = luaL_checkstring(L, 1);
			unsigned index = listbox->InsertItem(AnsiToWide(key));
			lua_pushunsigned(L, index);
			return 1;
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int SetListBoxItem(lua_State* L)
	{
		auto listbox = gFBUIManager->GetCachedListBox();
		if (!listbox)
			return 0;
		unsigned rowIndex = luaL_checkunsigned(L, 1);
		unsigned colIndex = luaL_checkunsigned(L, 2);
		ListItemDataType::Enum dataType = (ListItemDataType::Enum)luaL_checkunsigned(L, 3);
		
		switch (dataType){
		case ListItemDataType::String:
		case ListItemDataType::TextField:
		case ListItemDataType::TexturePath:
		case ListItemDataType::TextureRegion:
		{
			listbox->SetItem(Vec2I(rowIndex, colIndex), AnsiToWide(luaL_checkstring(L, 4)), dataType);
			break;
		}
		case ListItemDataType::CheckBox:
		{
			listbox->SetItem(Vec2I(rowIndex, colIndex), lua_toboolean(L, 4)!=0);
			break;
		}
		case ListItemDataType::NumericUpDown:
		{
			listbox->SetItem(Vec2I(rowIndex, colIndex), (int)lua_tonumber(L, 4));
			break;
		}
		case ListItemDataType::HorizontalGauge:
		{
			listbox->SetItem(Vec2I(rowIndex, colIndex), (float)lua_tonumber(L, 4));
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
		const char* uiname = luaL_checkstring(L, 1);
		const char* listCompName = luaL_checkstring(L, 2);
		auto comp = gFBEnv->pUIManager->FindComp(uiname, listCompName);
		PropertyList* propertyList = dynamic_cast<PropertyList*>(comp);
		if (propertyList)
		{
			const char* key = luaL_checkstring(L, 3);
			const char* value = luaL_checkstring(L, 4);
			std::wstring wKey = AnsiToWide(key);
			std::wstring wValue = AnsiToWide(value);
			propertyList->InsertItem(wKey.c_str(), wValue.c_str());
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int GetListBoxSelectedRows(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, compName);
		ListBox* listbox = dynamic_cast<ListBox*>(comp);
		if (listbox)
		{
			auto rows = listbox->GetSelectedRows();
			if (rows.empty())
			{
				lua_pushnil(L);
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
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, compName);
		ListBox* listbox = dynamic_cast<ListBox*>(comp);
		if (listbox)
		{
			std::vector<std::string> keys;
			listbox->GetSelectedUniqueIdsString(keys);

			if (keys.empty())
			{
				lua_pushnil(L);
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
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, compName);
		ListBox* listbox = dynamic_cast<ListBox*>(comp);
		if (listbox)
		{
			std::vector<unsigned> keys;
			listbox->GetSelectedUniqueIdsUnsigned(keys);

			if (keys.empty())
			{
				lua_pushnil(L);
				return 1;
			}
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
	int GetListBoxStringKeyCached(lua_State* L){
		auto listBox = gFBUIManager->GetCachedListBox();
		if (listBox){
			unsigned row = luaL_checkunsigned(L, 1);
			auto key = listBox->GetStringKey(row);
			if (key){
				lua_pushstring(L, WideToAnsi(key));
				return 1;
			}
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int GetListBoxNumericKeyCached(lua_State* L){
		auto listBox = gFBUIManager->GetCachedListBox();
		if (listBox){
			unsigned row = luaL_checkunsigned(L, 1);
			auto key = listBox->GetUnsignedKey(row);
			lua_pushnumber(L, key);
			return 1;
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int GetListBoxRowIndexCached(lua_State* L){
		auto listBox = gFBUIManager->GetCachedListBox();
		if (listBox)
		{
			if (lua_isstring(L, 1)){
				lua_pushnumber(L, listBox->FindRowIndex(
					AnsiToWide(luaL_checkstring(L, 1))
					));
				return 1;
			}
			else{
				lua_pushnumber(L, listBox->FindRowIndex(
					luaL_checkunsigned(L, 1)));
				return 1;
			}
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int SelectListBoxItem(lua_State* L)
	{
		const char* wnd = luaL_checkstring(L, 1);
		const char* comp = luaL_checkstring(L, 2);
		if (lua_isnil(L, 3))
		{
			return 0;
		}
		unsigned row = luaL_checkunsigned(L, 3);
		auto winbase = gFBEnv->pUIManager->FindComp(wnd, comp);
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
		const char* wnd = luaL_checkstring(L, 1);
		const char* comp = luaL_checkstring(L, 2);
		if (lua_isnil(L, 3))
		{
			return 0;
		}
		LuaObject rows(L, 3);
		if (!rows.IsTable())
		{
			Error(DEFAULT_DEBUG_ARG, "Invalid Param.");
			return 0;
		}
		auto winbase = gFBEnv->pUIManager->FindComp(wnd, comp);
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

	//-----------------------------------------------------------------------
	int GetListItemDataImpl(lua_State* L, ListBox* listbox, unsigned row, unsigned col){
		if (listbox)
		{
			auto itemdata = listbox->GetData(row, col);
			if (itemdata->IsTextData()){
				lua_pushstring(L, WideToAnsi(itemdata->GetText()));
				return 1;
			}
			else if (itemdata->GetDataType() == ListItemDataType::CheckBox){
				lua_pushboolean(L, itemdata->GetChecked());
				return 1;
			}
			else if (itemdata->GetDataType() == ListItemDataType::NumericUpDown){
				lua_pushnumber(L, itemdata->GetInt());
				return 1;
			}
			else if (itemdata->GetDataType() == ListItemDataType::HorizontalGauge){
				lua_pushnumber(L, itemdata->GetFloat());
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
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto listbox = dynamic_cast<ListBox*>(gFBEnv->pUIManager->FindComp(uiname, compName));
		unsigned row = luaL_checkunsigned(L, 3);
		unsigned col = luaL_checkunsigned(L, 4);
		return GetListItemDataImpl(L, listbox, row, col);
	}

	//-----------------------------------------------------------------------
	int GetListItemDataCachedListBox(lua_State* L){
		auto listbox = gFBUIManager->GetCachedListBox();
		unsigned row = luaL_checkunsigned(L, 1);
		unsigned col = luaL_checkunsigned(L, 2);
		return GetListItemDataImpl(L, listbox, row, col);		
	}

	//-----------------------------------------------------------------------
	int SetListItemProperty(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* listCompName = luaL_checkstring(L, 2);
		auto comp = gFBEnv->pUIManager->FindComp(uiname, listCompName);
		ListBox* listbox = dynamic_cast<ListBox*>(comp);
		if (listbox)
		{
			unsigned rowIndex = luaL_checkunsigned(L, 3);
			unsigned colIndex = luaL_checkunsigned(L, 4);
			auto item = listbox->GetItem(Vec2I(rowIndex, colIndex));
			auto prop = luaL_checkstring(L, 5);
			auto val = luaL_checkstring(L, 6);
			item->SetProperty(UIProperty::ConvertToEnum(prop), val);
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int StartUIAnimationForListItem(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto comp = gFBEnv->pUIManager->FindComp(uiname, compName);
		if (!comp || comp->GetType() != ComponentType::ListBox)
			return 0;
		unsigned index = luaL_checkunsigned(L, 3);
		const char* animName = luaL_checkstring(L, 4);

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
					auto ganim = gFBEnv->pUIManager->GetGlobalAnimation(animName);
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
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto comp = gFBEnv->pUIManager->FindComp(uiname, compName);
		if (!comp || comp->GetType() != ComponentType::ListBox)
			return 0;
		unsigned index = luaL_checkunsigned(L, 3);
		const char* animName = luaL_checkstring(L, 4);

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
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto listBox = dynamic_cast<ListBox*>(gFBEnv->pUIManager->FindComp(uiname, compName));
		if (listBox)
		{
			lua_pushunsigned(L, listBox->GetNumData());
			return 1;
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int SwapListBoxItem(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto listBox = dynamic_cast<ListBox*>(gFBEnv->pUIManager->FindComp(uiname, compName));
		if (listBox)
		{
			listBox->SwapItems(luaL_checkunsigned(L, 3), luaL_checkunsigned(L, 4));
		}
		return 0;
	}

	//-----------------------------------------------------------------------
	int SetTooltipString(lua_State* L){
		const char* s = luaL_checkstring(L, 1);
		gFBUIManager->SetTooltipString(AnsiToWide(s));
		auto mouse = gFBEnv->pEngine->GetMouse();
		gFBUIManager->SetTooltipPos(mouse->GetNPos());
		return 0;
	}

}