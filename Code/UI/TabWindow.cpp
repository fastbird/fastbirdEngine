#include <UI/StdAfx.h>
#include <UI/TabWindow.h>
#include <UI/Button.h>

namespace fastbird{

	const Vec2I TabWindow::sButtonSize(150, 24);
	TabWindow::TabWindow() {
		mNumTabs = GetDefaultValueInt(UIProperty::TABWND_NUM_TABS);
		mUIObject = gFBEnv->pEngine->CreateUIObject(false, GetRenderTargetSize());
		mUIObject->mOwnerUI = this;
		mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	}

	TabWindow::~TabWindow() {

	}

	void TabWindow::OnCreated() {
		mMainWindow = AddChild(0.f, 0.f, 1.f, 1.f, ComponentType::Window);
		mMainWindow->SetInitialOffset(Vec2I(0, sButtonSize.y));
		mMainWindow->SetSizeModificator(Vec2I(0, -sButtonSize.y));
		mMainWindow->SetRuntimeChild(true);
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

	bool TabWindow::GetProperty(UIProperty::Enum prop, char val[], bool notDefaultOnly) {
		switch (prop) {

		case UIProperty::TABWND_NUM_TABS:
		{
			if (notDefaultOnly){
				if (mNumTabs == UIProperty::GetDefaultValueInt(prop))
					return false;
			}

			strcpy(val, StringConverter::toString(mNumTabs).c_str());
			return true;
		}
		case UIProperty::TABWND_TAB_NAMES:
		{
			if (notDefaultOnly){
				if (mStrTabNames.empty())
					return false;
			}
			// 255 is enough?
			strncpy(val, mStrTabNames.c_str(), 255);
			return true;
		}
		}
		return __super::GetProperty(prop, val, notDefaultOnly);
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
			button->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK,
				std::bind(&TabWindow::OnTabClicked, this, std::placeholders::_1));
			button->SetRuntimeChild(true);
			mWindows.push_back(
				AddChild(0.f, 0.f, 1.f, 1.f, ComponentType::Window)
				);
			mWindows.back()->SetInitialOffset(Vec2I(0, sButtonSize.y));
			mWindows.back()->SetSizeModificator(Vec2I(0, -sButtonSize.y));
			mWindows.back()->SetRuntimeChild(true);
			++mNumTabs;
		}
	}

	void TabWindow::SetTabNames(const StringVector& names) {
		unsigned size = names.size();
		size = std::min(size, mNumTabs);
		for (unsigned i = 0; i < size; ++i){
			mButtons[i]->SetText( AnsiToWide(TranslateText(names[i].c_str()).c_str()) );
		}
	}

	void TabWindow::OnTabClicked(void* arg) {
		auto it = std::find(mButtons.begin(), mButtons.end(), arg);
		if_assert_fail(it != mButtons.end())
			return;

		unsigned index = std::distance(mButtons.begin(), it);
		if_assert_fail(index < mWindows.size())
			return;
		
		for (unsigned i = 0; i < mWindows.size(); ++i){
			mWindows[i]->SetVisible(i == index);
			mWindows[i]->SetEnable(i == index);
		}
	}
}