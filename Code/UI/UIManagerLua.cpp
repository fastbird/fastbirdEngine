#include <UI/StdAfx.h>
#include <UI/UIManager.h>
#include <UI/StaticText.h>
#include <UI/Button.h>
#include <UI/ListBox.h>
#include <UI/TextBox.h>
#include <UI/CheckBox.h>
#include <UI/NumericUpDown.h>
#include <UI/DropDown.h>
#include <UI/ColorRampComp.h>
#include <UI/CardScroller.h>
//--------------------------------------------------------------------------------

namespace fastbird
{
	int ClearListBox(lua_State* L);
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
	int GetListBoxSelectedRows(lua_State* L);
	int GetListBoxSelectedRowsIds(lua_State* L);
	int SetActivationUIAnim(lua_State* L);
	int SetFocusUI(lua_State* L);
	int SelectListBoxItem(lua_State* L);
	int SelectListBoxItems(lua_State* L);
	int GetListBoxItemText(lua_State* L);
	int AddListItem(lua_State* L);
	int SetListItemProperty(lua_State* L);
	int IsButtonActivated(lua_State* L);
	int StartUIAnimation(lua_State* L);
	int StopUIAnimation(lua_State* L);
	int StartUIAnimationForListItem(lua_State* L);
	int StopUIAnimationForListItem(lua_State* L);
	int MatchUIHeight(lua_State* L);
	int CacheListBox(lua_State* L);
	int InsertCachedListItemCheckBox(lua_State* L);
	int InsertCachedListItemString(lua_State* L);
	int SetCachedListItemString(lua_State* L);
	int GetCheckedFromCheckBox(lua_State* L);
	int GetNumericUpDownValue(lua_State* L);
	int SetNumericUpDownValue(lua_State* L);
	int GetListItemText(lua_State* L);
	int MoveUIToBottom(lua_State* L);
	int SetDropDownIndex(lua_State* L);
	int GetDropDownIndex(lua_State* L);
	int SetVisibleLuaUIWithoutFocusing(lua_State* L);
	int GetNumListBoxItems(lua_State* L);
	int SetListBoxRowId(lua_State* L);
	int GetListBoxRowIds(lua_State* L);
	int GetListBoxSelectedRowIds(lua_State* L);
	int SwapListBoxItem(lua_State* L);
	int GetColorRampUIValues(lua_State* L);
	int SetColorRampUIValues(lua_State* L);
	int HideUIsExcept(lua_State* L);
	int StartHighlightUI(lua_State* L);
	int StopHighlightUI(lua_State* L);
	int SetEnableComponent(lua_State* L);
	//--------------------------------------------------------------------------------
	void RegisterLuaFuncs(lua_State* mL)
	{
		LUA_SETCFUNCTION(mL, SetEnableComponent);
		LUA_SETCFUNCTION(mL, StartHighlightUI);
		LUA_SETCFUNCTION(mL, StopHighlightUI);
		LUA_SETCFUNCTION(mL, HideUIsExcept);
		LUA_SETCFUNCTION(mL, GetListBoxSelectedRowIds);
		LUA_SETCFUNCTION(mL, SetColorRampUIValues);
		LUA_SETCFUNCTION(mL, GetColorRampUIValues);
		LUA_SETCFUNCTION(mL, GetListBoxRowIds);
		LUA_SETCFUNCTION(mL, SwapListBoxItem);
		LUA_SETCFUNCTION(mL, SetListBoxRowId);
		LUA_SETCFUNCTION(mL, GetNumListBoxItems);
		LUA_SETCFUNCTION(mL, SetVisibleLuaUIWithoutFocusing);
		LUA_SETCFUNCTION(mL, GetDropDownIndex);
		LUA_SETCFUNCTION(mL, SetDropDownIndex);
		LUA_SETCFUNCTION(mL, MoveUIToBottom);
		LUA_SETCFUNCTION(mL, SetNumericUpDownValue);
		LUA_SETCFUNCTION(mL, GetNumericUpDownValue);
		LUA_SETCFUNCTION(mL, GetCheckedFromCheckBox);
		LUA_SETCFUNCTION(mL, InsertCachedListItemCheckBox);
		LUA_SETCFUNCTION(mL, InsertCachedListItemString);
		LUA_SETCFUNCTION(mL, SetCachedListItemString);
		LUA_SETCFUNCTION(mL, CacheListBox);
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
		LUA_SETCFUNCTION(mL, ClearListBox);
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
		LUA_SETCFUNCTION(mL, GetListBoxSelectedRows);
		LUA_SETCFUNCTION(mL, GetListBoxSelectedRowsIds);
		LUA_SETCFUNCTION(mL, SetActivationUIAnim);
		LUA_SETCFUNCTION(mL, SetFocusUI);
		LUA_SETCFUNCTION(mL, SelectListBoxItem);
		LUA_SETCFUNCTION(mL, SelectListBoxItems);
		LUA_SETCFUNCTION(mL, GetListBoxItemText);
		LUA_SETCFUNCTION(mL, AddListItem);
		LUA_SETCFUNCTION(mL, SetListItemProperty);
		LUA_SETCFUNCTION(mL, IsButtonActivated);
		LUA_SETCFUNCTION(mL, StartUIAnimation);
		LUA_SETCFUNCTION(mL, StopUIAnimation);
		LUA_SETCFUNCTION(mL, StartUIAnimationForListItem);
		LUA_SETCFUNCTION(mL, StopUIAnimationForListItem);
		LUA_SETCFUNCTION(mL, GetListItemText);
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

	int AddListBoxItem(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compoName = luaL_checkstring(L, 2);
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, compoName);
		auto listBox = dynamic_cast<ListBox*>(comp);
		if (listBox)
		{
			unsigned numCols = listBox->GetNumCols();
			LuaObject seq(L, 3);
			int row = 0;
			for (unsigned i = 0; i < numCols; ++i)
			{
				if (i == 0)
				{
					row = listBox->InsertItem(
						AnsiToWide(seq.GetSeqTable(i + 1).GetString().c_str()));
				}
				else
				{
					listBox->SetItemString(row, i,
						AnsiToWide(seq.GetSeqTable(i + 1).GetString().c_str()));
				}
			}
			lua_pushboolean(L, 1);
		}
		else
		{
			lua_pushboolean(L, 0);
		}
		return 1;
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
				button->Blink(blink);
				lua_pushboolean(L, 1);
				return 1;
			}
		}
		assert(0);
		lua_pushboolean(L, 0);
		return 1;
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

			bool result = comp->GetProperty(UIProperty::ConverToEnum(prop), buf);
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
				eventHandler->UnregisterEventLuaFunc(ConvertToEventEnum(eventName));
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

	int GetListBoxSelectedRowsIds(lua_State* L)
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
				luaRows.SetSeq(i++, listbox->GetRowId(row));
			}
			luaRows.PushToStack();
			return 1;
		}

		assert(0);
		return 0;
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

	int GetListBoxItemText(lua_State* L)
	{
		const char* wnd = luaL_checkstring(L, 1);
		const char* comp = luaL_checkstring(L, 2);
		unsigned row = luaL_checkunsigned(L, 3);
		unsigned col = luaL_checkunsigned(L, 4);
		auto winbase = gFBEnv->pUIManager->FindComp(wnd, comp);
		ListBox* listbox = static_cast<ListBox*>(winbase);
		if (listbox)
		{
			auto listItem = listbox->GetItem(row, col);
			lua_pushstring(L, WideToAnsi(listItem->GetText()));
			return 1;
		}


		return 0;
	}

	int CloneLuaUI(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* newUIname = luaL_checkstring(L, 2);
		gFBEnv->pUIManager->CloneUI(uiname, newUIname);
		return 0;
	}

	int AddListItem(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* listCompName = luaL_checkstring(L, 2);
		auto comp = gFBEnv->pUIManager->FindComp(uiname, listCompName);
		ListBox* listbox = dynamic_cast<ListBox*>(comp);
		if (listbox)
		{
			unsigned n = 3;
			unsigned col = 0;
			unsigned row = 0;
			while (!lua_isnone(L, n))
			{
				const char* data = luaL_checkstring(L, n++);
				if (col == 0)
				{
					row = listbox->InsertItem(AnsiToWide(data));
					col++;
				}
				else
				{
					listbox->SetItemString(row, col++, AnsiToWide(data));
				}
			}
			lua_pushunsigned(L, row);
			lua_pushunsigned(L, col);
			return 2;
		}
		return 0;
	}

	int SetListItemProperty(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* listCompName = luaL_checkstring(L, 2);
		auto comp = gFBEnv->pUIManager->FindComp(uiname, listCompName);
		ListBox* listbox = dynamic_cast<ListBox*>(comp);
		if (listbox)
		{
			unsigned row = luaL_checkunsigned(L, 3);
			unsigned col = luaL_checkunsigned(L, 4);
			auto item = listbox->GetItem(3, 4);
			auto prop = luaL_checkstring(L, 5);
			auto val = luaL_checkstring(L, 6);
			item->SetProperty(UIProperty::ConverToEnum(prop), val);
		}
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
			auto item = listbox->GetItem(index, c);
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
			auto item = listbox->GetItem(index, c);
			auto uiAnimation = item->GetUIAnimation(animName);
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

	int CacheListBox(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		bool found = gFBEnv->pUIManager->CacheListBox(uiname, compName);
		lua_pushboolean(L, found);
		return 1;
	}

	int InsertCachedListItemCheckBox(lua_State* L)
	{
		bool checked = lua_toboolean(L, 1) != 0;
		const char* name = luaL_checkstring(L, 2);
		const char* eventFunc = luaL_checkstring(L, 3);
		ListBox* listbox = gFBEnv->pUIManager->GetCachedListBox();
		if_assert_fail(listbox)
			return 0;
		unsigned row = listbox->InsertCheckBoxItem(checked);
		CheckBox* checkbox = listbox->GetCheckBox(row, 0);
		checkbox->SetName(name);
		checkbox->RegisterEventLuaFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK, eventFunc);
		lua_pushunsigned(L, row);
		return 1;
	}

	int InsertCachedListItemString(lua_State* L)
	{
		const char* str = luaL_checkstring(L, 1);
		ListBox* listbox = gFBEnv->pUIManager->GetCachedListBox();
		if_assert_fail(listbox)
			return 0;

		unsigned row = listbox->InsertItem(AnsiToWide(str));
		lua_pushunsigned(L, row);
		return 1;
	}

	int SetCachedListItemString(lua_State* L)
	{
		unsigned row = luaL_checkunsigned(L, 1);
		unsigned col = luaL_checkunsigned(L, 2);
		const char* text = luaL_checkstring(L, 3);
		ListBox* listbox = gFBEnv->pUIManager->GetCachedListBox();
		if_assert_fail(listbox)
			return 0;
		listbox->SetItemString(row, col, AnsiToWide(text));
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

	int GetListItemText(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto comp = dynamic_cast<ListBox*>(gFBEnv->pUIManager->FindComp(uiname, compName));
		if (comp)
		{
			unsigned row = luaL_checkunsigned(L, 3);
			unsigned col = luaL_checkunsigned(L, 4);
			auto item = comp->GetItem(row, col);
			if (item)
			{
				lua_pushstring(L, WideToAnsi(item->GetText()));
				return 1;
			}
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

	int GetNumListBoxItems(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto listBox = dynamic_cast<ListBox*>(gFBEnv->pUIManager->FindComp(uiname, compName));
		if (listBox)
		{
			lua_pushunsigned(L, listBox->GetNumItems());
			return 1;
		}
		return 0;
	}

	int SetListBoxRowId(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto listBox = dynamic_cast<ListBox*>(gFBEnv->pUIManager->FindComp(uiname, compName));
		if (listBox)
		{
			listBox->SetRowId(luaL_checkunsigned(L, 3), luaL_checkunsigned(L, 4));
		}
		return 0;
	}

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

	int GetListBoxRowIds(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto listBox = dynamic_cast<ListBox*>(gFBEnv->pUIManager->FindComp(uiname, compName));
		if (listBox)
		{
			auto numItems = listBox->GetNumItems();
			if (numItems == 0)
				return 0;

			LuaObject table;
			table.NewTable(L);
			for (unsigned i = 0; i < numItems; i++)
			{
				unsigned rowId = listBox->GetRowId(i);
				table.SetSeq(i + 1, rowId);
			}
			table.PushToStack();
			return 1;
		}
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

	int GetListBoxSelectedRowIds(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto listbox = dynamic_cast<ListBox*>(gFBEnv->pUIManager->FindComp(uiname, compName));
		if (listbox)
		{
			std::vector<unsigned> rowIds;
			listbox->GetSelectedRowIds(rowIds);
			int n = 1;
			LuaObject table;
			table.NewTable(L);
			for (auto rowId : rowIds)
			{
				table.SetSeq(n++, rowId);
			}
			table.PushToStack();
			return 1;
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
}