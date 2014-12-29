#include <UI/StdAfx.h>
#include <UI/UIManager.h>
#include <UI/StaticText.h>
#include <UI/Button.h>
#include <UI/ListBox.h>
#include <UI/TextBox.h>

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
	int SetVisibleComponent(lua_State* L);
	int GetVisibleLuaUI(lua_State* L);
	int RemoveAllChildrenOf(lua_State* L);
	int AddComponent(lua_State* L);
	int BlinkButton(lua_State* L);
	int UpdateButtonProgressBar(lua_State* L);
	int StartButtonProgressBar(lua_State* L);
	int EndButtonProgressBar(lua_State* L);
	int SetTextBoxText(lua_State* L);
	int SetUIBackground(lua_State* L);
	int SetUIProperty(lua_State* L);
	int RemoveUIEventhandler(lua_State* L);
	int GetMousePos(lua_State* L);
	int GetComponentWidth(lua_State* L);
	int FindAndRememberComponent(lua_State* L);
	int GetListboxSelectedRows(lua_State* L);
	int SetActivationUIAnim(lua_State* L);
	int SetFocusUI(lua_State* L);
	int SelectListBoxItem(lua_State* L);
	int GetListBoxItemText(lua_State* L);
	int AddListItem(lua_State* L);
	int IsButtonActivated(lua_State* L);

	//--------------------------------------------------------------------------------
	void RegisterLuaFuncs(lua_State* mL)
	{
		LUA_SETCFUNCTION(mL, SetVisibleLuaUI);
		LUA_SETCFUNCTION(mL, GetVisibleLuaUI);
		LUA_SETCFUNCTION(mL, SetVisibleComponent);
		LUA_SETCFUNCTION(mL, LoadLuaUI);
		LUA_SETCFUNCTION(mL, IsLoadedUI);
		LUA_SETCFUNCTION(mL, CloneLuaUI);
		LUA_SETCFUNCTION(mL, AddLuaUI);
		LUA_SETCFUNCTION(mL, DeleteLuaUI);
		LUA_SETCFUNCTION(mL, ClearListBox);
		LUA_SETCFUNCTION(mL, SetStaticText);
		LUA_SETCFUNCTION(mL, RemoveAllChildrenOf);
		LUA_SETCFUNCTION(mL, AddComponent);
		LUA_SETCFUNCTION(mL, BlinkButton);
		LUA_SETCFUNCTION(mL, UpdateButtonProgressBar);
		LUA_SETCFUNCTION(mL, StartButtonProgressBar);
		LUA_SETCFUNCTION(mL, EndButtonProgressBar);
		LUA_SETCFUNCTION(mL, SetTextBoxText);
		LUA_SETCFUNCTION(mL, SetUIBackground);
		LUA_SETCFUNCTION(mL, SetUIProperty);
		LUA_SETCFUNCTION(mL, RemoveUIEventhandler);
		LUA_SETCFUNCTION(mL, GetMousePos);
		LUA_SETCFUNCTION(mL, GetComponentWidth);
		LUA_SETCFUNCTION(mL, FindAndRememberComponent);
		LUA_SETCFUNCTION(mL, GetListboxSelectedRows);
		LUA_SETCFUNCTION(mL, SetActivationUIAnim);
		LUA_SETCFUNCTION(mL, SetFocusUI);
		LUA_SETCFUNCTION(mL, SelectListBoxItem);
		LUA_SETCFUNCTION(mL, GetListBoxItemText);
		LUA_SETCFUNCTION(mL, AddListItem);
		LUA_SETCFUNCTION(mL, IsButtonActivated);
	}

	int LoadLuaUI(lua_State* L)
	{
		const char* uiFile = luaL_checkstring(L, 1);
		std::vector<IWinBase*> temp;
		std::string name;
		UIManager::GetUIManagerStatic()->ParseUI(uiFile, temp, name, true);
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
		UIManager::GetUIManagerStatic()->AddLuaUI(uiname, ui);
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

	int AddComponent(lua_State* L)
	{
		Profiler p("AddComponent_Lua");
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
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, compName);
		if (comp)
		{
			comp->SetProperty(UIProperty::ConverToEnum(prop), val);
			return 0;
		}
		assert(0);
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
		gEnv->pEngine->GetMousePos(x, y);

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

	int GetListboxSelectedRows(lua_State* L)
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
		lua_pushnil(L);
		return 1;

	}

	int SetActivationUIAnim(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto comp = UIManager::GetUIManagerStatic()->FindComp(uiname, compName);
		const char* animName = luaL_checkstring(L, 3);
		bool active = lua_toboolean(L, 4) != 0;

		auto anim = comp->GetUIAnimation(animName);
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
		auto winbase = IUIManager::GetUIManager().FindComp(wnd, comp);
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

	int GetListBoxItemText(lua_State* L)
	{
		const char* wnd = luaL_checkstring(L, 1);
		const char* comp = luaL_checkstring(L, 2);
		unsigned row = luaL_checkunsigned(L, 3);
		unsigned col = luaL_checkunsigned(L, 4);
		auto winbase = IUIManager::GetUIManager().FindComp(wnd, comp);
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
		IUIManager::GetUIManager().CloneUI(uiname, newUIname);
		return 0;
	}

	int AddListItem(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* listCompName = luaL_checkstring(L, 2);
		auto comp = IUIManager::GetUIManager().FindComp(uiname, listCompName);
		ListBox* listbox = dynamic_cast<ListBox*>(comp);
		if (listbox)
		{
			unsigned n = 3;
			int col = 0;
			int row = 0;
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
		}
		return 0;
	}

	int IsButtonActivated(lua_State* L)
	{
		const char* uiname = luaL_checkstring(L, 1);
		const char* compName = luaL_checkstring(L, 2);
		auto comp = IUIManager::GetUIManager().FindComp(uiname, compName);
		Button* btn = dynamic_cast<Button*>(comp);
		if (btn)
		{
			bool activated = btn->IsActivated();
			lua_pushboolean(L, activated);
			return 1;
		}

		return 0;
	}
}