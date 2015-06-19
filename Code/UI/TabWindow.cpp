#include <UI/StdAfx.h>
#include <UI/TabWindow.h>
#include <UI/Button.h>
#include <UI/Container.h>

namespace fastbird{

	const Vec2I TabWindow::sButtonSize(150, 24);
	TabWindow::TabWindow()
		: mCurTabIndex(0)
	{
		mNumTabs = GetDefaultValueInt(UIProperty::TABWND_NUM_TABS);
		mUIObject = gFBEnv->pEngine->CreateUIObject(false, GetRenderTargetSize());
		mUIObject->mOwnerUI = this;
		mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	}

	TabWindow::~TabWindow() {

	}

	void TabWindow::OnCreated() {
		mMainWindow = AddChild(0.f, 0.f, 1.f, 1.f, ComponentType::Window);
		mMainWindow->SetVisible(mVisibility.IsVisible());
		mMainWindow->Move(Vec2I(0, sButtonSize.y));
		mMainWindow->ModifySize(Vec2I(0, -sButtonSize.y));
		mMainWindow->SetProperty(UIProperty::USE_BORDER, "true");
		mMainWindow->SetRuntimeChild(true);
		mMainWindow->SetProperty(UIProperty::NO_BACKGROUND, "true");
		mMainWindow->SetGhost(true);
		mMainWindow->SetUseAbsSize(false);
	}
	
	bool TabWindow::SetProperty(UIProperty::Enum prop, const char* val) {
		switch (prop){
		case UIProperty::TABWND_NUM_TABS:
		{
			unsigned num = StringConverter::parseUnsignedInt(val);
			if (num != mNumTabs)
			{
				BuildTabWnd(num);
			}
			return true;
		}
		case UIProperty::TABWND_TAB_NAMES:
		{
			mStrTabNames = val;
			auto names = Split(val, ",");
			SetTabNames(names);
			return true;
		}
		}
		return __super::SetProperty(prop, val);
	}

	bool TabWindow::GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly) {
		switch (prop) {

		case UIProperty::TABWND_NUM_TABS:
		{
			if (notDefaultOnly){
				if (mNumTabs == UIProperty::GetDefaultValueInt(prop))
					return false;
			}

			strcpy_s(val, bufsize, StringConverter::toString(mNumTabs).c_str());
			return true;
		}
		case UIProperty::TABWND_TAB_NAMES:
		{
			if (notDefaultOnly){
				if (mStrTabNames.empty())
					return false; 
			}
			// 255 is enough?
			strncpy_s(val, bufsize, mStrTabNames.c_str(), 255);
			return true;
		}
		}
		return __super::GetProperty(prop, val, bufsize, notDefaultOnly);
	}
	
	bool TabWindow::ParseXML(tinyxml2::XMLElement* pelem){	
		bool ret = WinBase::ParseXML(pelem);
		auto tab = pelem->FirstChildElement("tab");
		while (tab)
		{
			const char* sz  = tab->Attribute("index");
			if (sz){
				unsigned index = StringConverter::parseUnsignedInt(sz);
				if (index < mWindows.size()){
					Container* cont = (Container*)mWindows[index];
					cont->ParseXMLChildren(tab);
					cont->SetVisible(cont->GetVisible());
				}
			}
			tab = tab->NextSiblingElement("tab");
		}

		return ret;
	}

	void TabWindow::Save(tinyxml2::XMLElement& elem)
	{
		WinBase::Save(elem);
		for (unsigned i = 0; i < mNumTabs; ++i){
			auto tab = elem.GetDocument()->NewElement("tab");
			elem.InsertEndChild(tab);
			tab->SetAttribute("index", i);
			if (i < mWindows.size()){
				Container* cont = (Container*)mWindows[i];
				cont->SaveChildren(*tab);
			}
		}
	}

	bool TabWindow::SetVisible(bool show)
	{
		auto changed = WinBase::SetVisible(show);
		for (auto btn : mButtons){
			btn->SetVisible(show);
		}
		mMainWindow->SetVisible(show);
		UpdateTabIndex(mCurTabIndex);
		return changed;
	}

	void TabWindow::BuildTabWnd(unsigned numTabs) {
		if (numTabs == 0 && numTabs == -1)
			return;
		while (mNumTabs > numTabs)
		{
			RemoveChild(mButtons.back());
			RemoveChild(mWindows.back());
			--mNumTabs;
		}
		while (mNumTabs < numTabs){
			unsigned curNum = mButtons.size();
			mButtons.push_back(
				AddChild(Vec2I(sButtonSize.x*curNum, 0), sButtonSize, ComponentType::Button)
				);
			auto button = (Button*)mButtons.back();
			button->SetProperty(UIProperty::TEXT_ALIGN, "center");
			button->SetVisible(mVisibility.IsVisible());
			button->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
				std::bind(&TabWindow::OnTabClicked, this, std::placeholders::_1));
			button->SetProperty(UIProperty::USE_BORDER, "true");
			mWindows.push_back(
				AddChild(0.f, 0.f, 1.f, 1.f, ComponentType::Window)
				);
			auto wnd = mWindows.back();
			wnd->SetVisible(mVisibility.IsVisible());
			wnd->Move(Vec2I(0, sButtonSize.y));
			wnd->ModifySize(Vec2I(0, -sButtonSize.y));
			wnd->SetUseAbsSize(false);
			++mNumTabs;
		}

		if (mCurTabIndex >= mButtons.size()){
			mCurTabIndex = mButtons.size() - 1;
		}
		UpdateTabIndex(mCurTabIndex);
	}

	void TabWindow::SetTabNames(const StringVector& names) {
		unsigned size = names.size();
		if (size > mNumTabs)
		{
			BuildTabWnd(size);
		}
		size = std::min(size, mNumTabs);
		for (unsigned i = 0; i < size; ++i){
			auto translated = TranslateText(names[i].c_str());
			if (translated.empty())
				mButtons[i]->SetProperty(UIProperty::TEXT, names[i].c_str());
			else
				mButtons[i]->SetProperty(UIProperty::TEXT, translated.c_str());
		}
	}

	void TabWindow::OnTabClicked(void* arg) {
		auto it = std::find(mButtons.begin(), mButtons.end(), arg);
		if_assert_fail(it != mButtons.end())
			return;

		unsigned index = std::distance(mButtons.begin(), it);
		if_assert_fail(index < mWindows.size())
			return;
		
		UpdateTabIndex(index);
	}

	void TabWindow::UpdateTabIndex(unsigned index){
		mCurTabIndex = index;
		auto visible = GetVisible();
		for (unsigned i = 0; i < mWindows.size(); ++i){
			mWindows[i]->SetVisible(visible && i == index);
			mWindows[i]->SetEnable(visible && i == index);
		}

		for (unsigned i = 0; i < mButtons.size(); ++i){
			if (i == index){
				mButtons[i]->SetTextColor(Color(1.0f, 1.0f, 0.0f, 1.0f));
				mButtons[i]->SetProperty(UIProperty::BACK_COLOR, "0.1, 0.1, 0.1, 0.9");
			}
			else{
				mButtons[i]->SetTextColor(Color(0.5f, 0.5f, 0.5f, 1.0f));
				mButtons[i]->SetProperty(UIProperty::BACK_COLOR,
					StringConverter::toString(GetDefaultValueVec4(UIProperty::BACK_COLOR)).c_str());
			}			
		}
	}
}