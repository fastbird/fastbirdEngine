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
#include "TabWindow.h"
#include "Button.h"
#include "Wnd.h"
#include "UIObject.h"

namespace fb{

	const Vec2I TabWindow::sButtonSize(150, 24);

	TabWindowPtr TabWindow::Create(){
		TabWindowPtr p(new TabWindow, [](TabWindow* obj){ delete obj; });
		p->mSelfPtr = p;
		return p;
	}

	TabWindow::TabWindow()
		: mCurTabIndex(0)
	{
		mNumTabs = GetDefaultValueInt(UIProperty::TABWND_NUM_TABS);
		mUIObject = UIObject::Create(GetRenderTargetSize());
		mUIObject->mOwnerUI = this;
		mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	}

	TabWindow::~TabWindow() {

	}

	void TabWindow::OnCreated() {
		auto wnd = std::dynamic_pointer_cast<Wnd>(AddChild(0.f, 0.f, 1.f, 1.f, ComponentType::Window));
		mMainWindow = wnd;
		wnd->SetVisible(mVisibility.IsVisible());
		wnd->Move(Vec2I(0, sButtonSize.y));
		wnd->ModifySize(Vec2I(0, -sButtonSize.y));
		wnd->SetProperty(UIProperty::USE_BORDER, "true");
		wnd->SetRuntimeChild(true);
		wnd->SetProperty(UIProperty::NO_BACKGROUND, "true");
		wnd->SetGhost(true);
		wnd->SetUseAbsSize(false);
	}
	
	bool TabWindow::SetProperty(UIProperty::Enum prop, const char* val) {
		switch (prop){
		case UIProperty::TABWND_NUM_TABS:
		{
			unsigned num = StringConverter::ParseUnsignedInt(val);
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
		case UIProperty::TABWND_CURRENT_INDEX:
		{
			auto index = StringConverter::ParseInt(val);
			UpdateTabIndex(index);
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

			strcpy_s(val, bufsize, StringConverter::ToString(mNumTabs).c_str());
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
		case UIProperty::TABWND_CURRENT_INDEX:
		{
			if (notDefaultOnly) {
				if (mCurTabIndex == UIProperty::GetDefaultValueInt(prop))
					return false;
			}

			strcpy_s(val, bufsize, StringConverter::ToString(mCurTabIndex).c_str());
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
				unsigned index = StringConverter::ParseUnsignedInt(sz);
				if (index < mWindows.size()){
					auto cont = mWindows[index].lock();
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
				auto cont = mWindows[i].lock();
				cont->SaveChildren(*tab);
			}
		}
	}

	bool TabWindow::SetVisible(bool show)
	{
		auto changed = WinBase::SetVisible(show);
		for (auto btn : mButtons){
			btn.lock()->SetVisible(show);
		}
		mMainWindow.lock()->SetVisible(show);
		UpdateTabIndex(mCurTabIndex);
		return changed;
	}

	void TabWindow::BuildTabWnd(unsigned numTabs) {
		if (numTabs == 0 && numTabs == -1)
			return;
		while (mNumTabs > numTabs)
		{
			RemoveChild(mButtons.back().lock());
			RemoveChild(mWindows.back().lock());
			--mNumTabs;
		}
		while (mNumTabs < numTabs){
			unsigned curNum = mButtons.size();
			auto button = std::static_pointer_cast<Button>(AddChild(Vec2I(sButtonSize.x*curNum, 0), sButtonSize, ComponentType::Button));
			mButtons.push_back(button);			
			button->SetProperty(UIProperty::TEXT_ALIGN, "center");
			button->SetVisible(mVisibility.IsVisible());
			button->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
				std::bind(&TabWindow::OnTabClicked, this, std::placeholders::_1));
			button->SetProperty(UIProperty::USE_BORDER, "true");
			auto wnd = std::static_pointer_cast<Wnd>(AddChild(0.f, 0.f, 1.f, 1.f, ComponentType::Window));
			mWindows.push_back(wnd);			
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
				mButtons[i].lock()->SetProperty(UIProperty::TEXT, names[i].c_str());
			else
				mButtons[i].lock()->SetProperty(UIProperty::TEXT, translated.c_str());
		}
	}

	void TabWindow::OnTabClicked(void* arg) {
		WinBase* p = (WinBase*)arg;
		if (p->GetType() != ComponentType::Button){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return;
		}
		
		auto it = std::find(mButtons.begin(), mButtons.end(), std::static_pointer_cast<Button>(p->GetPtr()));
		if_assert_fail(it != mButtons.end())
			return;

		unsigned index = std::distance(mButtons.begin(), it);
		if_assert_fail(index < mWindows.size())
			return;
		
		UpdateTabIndex(index);
		OnEvent(UIEvents::EVENT_TAB_WINDOW_CHANGED);
	}

	void TabWindow::UpdateTabIndex(unsigned index){
		mCurTabIndex = index;
		auto visible = GetVisible();
		for (unsigned i = 0; i < mWindows.size(); ++i){
			mWindows[i].lock()->SetVisible(visible && i == index);
			mWindows[i].lock()->SetEnable(visible && i == index);
		}

		for (unsigned i = 0; i < mButtons.size(); ++i){
			if (i == index){
				mButtons[i].lock()->SetTextColor(Color(1.0f, 1.0f, 0.0f, 1.0f));
				mButtons[i].lock()->SetProperty(UIProperty::BACK_COLOR, "0.1, 0.1, 0.1, 0.9");
			}
			else{
				mButtons[i].lock()->SetTextColor(Color(0.5f, 0.5f, 0.5f, 1.0f));
				mButtons[i].lock()->SetProperty(UIProperty::BACK_COLOR,
					StringMathConverter::ToString(GetDefaultValueVec4(UIProperty::BACK_COLOR)).c_str());
			}			
		}
	}

	void TabWindow::AddTabChild(unsigned index, LuaObject data)
	{
		if (index >= mNumTabs || index >= mWindows.size()) {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString(
				"Invalid index(%u)", index).c_str());
			return;
		}

		auto windows = mWindows[index].lock();
		if (!windows) {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString(
				"Tab windows(%u) is deleted.", index).c_str());
			return;
		}
		windows->AddChild(data);
	}
}