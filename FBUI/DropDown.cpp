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
#include "DropDown.h"
#include "Button.h"
#include "Wnd.h"
#include "UIObject.h"
#include "UIManager.h"

namespace fb
{

const float DropDown::LEFT_GAP = 0.001f;
const int DropDown::ITEM_HEIGHT = 24;
DropDownWeakPtr DropDown::sCurrentDropDown;

DropDownPtr DropDown::Create(){
	DropDownPtr p(new DropDown, [](DropDown* obj){ delete obj; });
	p->mSelfPtr = p;
	return p;
}

DropDown::DropDown()
	: mCursorPos(0)
	, mPasswd(false)
	, mCurIdx(0)
	, mReservedIdx(-1)	
	, mMaxHeight(200)
	, mTriggerEvent(true)
{
	mUIObject = UIObject::Create(GetRenderTargetSize());
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	mUIObject->SetTextColor(mTextColor);
	//mUIObject->SetNoDrawBackground(true);
	RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&DropDown::OnMouseClick, this, std::placeholders::_1));

	/*RegisterEventFunc(UIEvents::EVENT_MOUSE_HOVER,
		std::bind(&DropDown::OnMouseHover, this, std::placeholders::_1));
	RegisterEventFunc(UIEvents::EVENT_MOUSE_OUT,
		std::bind(&DropDown::OnMouseOut, this, std::placeholders::_1));*/
}

void DropDown::OnCreated()
{
	assert(mButton.expired());
	auto button = std::static_pointer_cast<Button>(AddChild(1.0f, 0.0f, 0.1f, 1.0f, ComponentType::Button));
	mButton = button;
	button->ChangeSize(Vec2I(ITEM_HEIGHT, ITEM_HEIGHT));
	button->SetProperty(UIProperty::ALIGNH, "right");
	char buf[512] = { 0 };
	if (GetProperty(UIProperty::BACK_COLOR, buf, 512, true)) {
		button->SetProperty(UIProperty::BACK_COLOR, buf);
	}
	button->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&DropDown::OnMouseClick, this, std::placeholders::_1));

	//mButton->SetProperty(UIProperty::NO_BACKGROUND, "true");
	button->SetProperty(UIProperty::TEXTUREATLAS, "EssentialEngineData/textures/ui.xml");
	button->SetProperty(UIProperty::REGION, "dropdown");
	button->SetProperty(UIProperty::HOVER_IMAGE, "dropdown_hover");
	button->SetRuntimeChild(true);

	CreateHolder();
}

void DropDown::CreateHolder() {
	if (!mHolder.expired())
		return;
	auto holder = std::static_pointer_cast<Wnd>(AddChild(0.f, 0.f, 1.f, 1.f, ComponentType::Window));
	mHolder = holder;
	holder->SetInitialOffset(Vec2I(0, mSize.y));
	holder->SetRuntimeChild(true);
	holder->SetProperty(UIProperty::NO_BACKGROUND, "true");
	holder->SetProperty(UIProperty::SCROLLERV, "true");
	holder->SetProperty(UIProperty::USE_SCISSOR, "false");
	holder->SetProperty(UIProperty::SPECIAL_ORDER, "3");
	holder->SetProperty(UIProperty::INHERIT_VISIBLE_TRUE, "false");
	holder->ChangeSizeY(ITEM_HEIGHT);
}

void DropDown::OnSizeChanged(){
	__super::OnSizeChanged();
	auto holder = mHolder.lock();
	if (holder)
		holder->SetInitialOffset(Vec2I(0, mSize.y));
}

void DropDown::GatherVisit(std::vector<UIObject*>& v)
{
	if (!mVisibility.IsVisible())
		return;
	v.push_back(mUIObject.get());
	__super::GatherVisit(v);
}

bool DropDown::SetProperty(UIProperty::Enum prop, const char* val)
{
	switch (prop)
	{
	case UIProperty::DROPDOWN_INDEX:
	{
		unsigned index = StringConverter::ParseUnsignedInt(val);
		if (index < mDropDownItems.size())
			SetSelectedIndex(index);
		else
			SetReservedIndex(index);
		return true;
	}
	case UIProperty::DROPDOWN_MAX_HEIGHT:
	{
		mMaxHeight = StringConverter::ParseInt(val);
		auto holder = mHolder.lock();
		if (holder && holder->GetSize().y <  (int)mDropDownItems.size() * ITEM_HEIGHT){
			int setheight = std::min(mMaxHeight, (int)mDropDownItems.size() * ITEM_HEIGHT);
			holder->ChangeSizeY(setheight);
		}
		return true;
	}

	case UIProperty::DROPDOWN_ITEMS:
	{
		ClearDropDownItems();
		mDropDownItemsString = val;
		auto v = Split(val, ",");
		for (auto& str : v) {
			if (!str.empty())
				AddDropDownItem(AnsiToWide(str.c_str()));
		}
		return true;
	}
	}


	return __super::SetProperty(prop, val);
}

bool DropDown::GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly)
{
	switch (prop)
	{
	case UIProperty::DROPDOWN_INDEX:
	{
		std::string data;
		if (mReservedIdx!=-1)
		{
			data = StringConverter::ToString(mReservedIdx);
		}
		else
		{
			data = StringConverter::ToString(mCurIdx);
		}
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}
	case UIProperty::DROPDOWN_MAX_HEIGHT:
	{
		strcpy_s(val, bufsize, StringConverter::ToString(mMaxHeight).c_str());
		return true;
	}

	case UIProperty::DROPDOWN_ITEMS:
	{
		if (notDefaultOnly && mDropDownItems.empty()) {
			return false;			
		}

		if (mDropDownItemsString.empty()) {
			val[0] = 0;
			return true;
		}		
		sprintf_s(val, bufsize, "%s", mDropDownItemsString.c_str());
		return true;
	}

	}
	
	return __super::GetProperty(prop, val, bufsize, notDefaultOnly);
}

void DropDown::OnMouseClick(void* arg)
{
	if (mDropDownItems.empty())
		return;
	auto holder = mHolder.lock();
	if (holder){
		bool vis = holder->GetVisible();
		SetVisibleDropDownItems(!vis);
	}
}


void DropDown::SetVisibleDropDownItems(bool visible){
	if (visible){
		auto current = sCurrentDropDown.lock();
		if (current && current.get() != this)
			current->CloseOptions();
		sCurrentDropDown = std::static_pointer_cast<DropDown>(mSelfPtr.lock());
		auto holder = mHolder.lock();
		if (holder){
			holder->SetVisible(true);
			UIManager::GetInstance().AddAlwaysMouseOverCheck(holder);
		}

		for (auto it = mDropDownItems.begin(); it != mDropDownItems.end(); /**/)
		{
			IteratingWeakContainer(mDropDownItems, it, item);
			item->SetVisible(true);
		}
	}
	else{
		auto current = sCurrentDropDown.lock();
		if (current.get() == this){
			sCurrentDropDown.reset();
		}
		auto holder = mHolder.lock();
		if (holder){
			holder->SetVisible(false);
			UIManager::GetInstance().RemoveAlwaysMouseOverCheck(holder);
		}
		for (auto it = mDropDownItems.begin(); it != mDropDownItems.end(); /**/)
		{
			IteratingWeakContainer(mDropDownItems, it, item);			
			item->SetVisible(false);
		}
	}
	UIManager::GetInstance().DirtyRenderList(GetHwndId());
}

void DropDown::CloseOptions()
{
	SetVisibleDropDownItems(false);
}

void DropDown::OnFocusLost()
{
	if (UIManager::GetInstance().GetNewFocusUI() == mButton.lock())
		return;
	SetVisibleDropDownItems(false);
}

void DropDown::OnItemSelected(void* arg)
{
	size_t index = -1;
	size_t findingIndex = 0;
	for (auto it = mDropDownItems.begin(); it != mDropDownItems.end(); /**/)
	{
		IteratingWeakContainer(mDropDownItems, it, item);
		if (item.get() == arg)
		{
			index = findingIndex;
			SetText(item->GetText());
		}
		item->SetVisible(false);	
		++findingIndex;
	}
	assert(index != -1);
	mCurIdx = index; 
	if (mTriggerEvent)
		OnEvent(UIEvents::EVENT_DROP_DOWN_SELECTED);
	UIManager::GetInstance().DirtyRenderList(GetHwndId());
}

size_t DropDown::AddDropDownItem(WCHAR* szString)
{
	auto item = std::dynamic_pointer_cast<Button>(
		mHolder.lock()->AddChild(0.0f, 0.0f, 1.0f, 1.0f, ComponentType::Button)
		);
	mDropDownItems.push_back(item);	
	item->SetRuntimeChild(true);
	item->SetProperty(UIProperty::INHERIT_VISIBLE_TRUE, "false");
	item->SetProperty(UIProperty::TEXT_LEFT_GAP, "4");
	item->SetText(szString);	
	// This index currently asumming as immutable.
	// If need to change the index in runtime, More code needed.
	size_t index = mDropDownItems.size()-1;
	SetCommonProperty(item, index);
	if (mDropDownItems.size() == 1)
	{
		mTriggerEvent = false;
		OnItemSelected(item.get());
		mTriggerEvent = true;
	}	

	return index;
}

size_t DropDown::AddDropDownItem(unsigned key, WCHAR* szString) {
	auto index = AddDropDownItem(szString);
	mIndexKeyMap[index] = key;
	return index;
}

size_t DropDown::AddDropDownItem(WinBasePtr winbase)
{
	auto item = std::dynamic_pointer_cast<Button>(winbase);
	if (!item){
		Logger::Log(FB_ERROR_LOG_ARG, "Drop down item should be a button.");
		return -1;
	}
	RemoveChild(item);	
	auto holder = mHolder.lock();
	if (holder){
		holder->AddChild(item);
	}
	mDropDownItems.push_back(item);	
	item->SetProperty(UIProperty::INHERIT_VISIBLE_TRUE, "false");
	item->SetNSizeX(1.0f);
	item->ChangeSizeY(ITEM_HEIGHT);
	item->ChangeNPos(Vec2(0.0f, 0.0f));	

	size_t index = mDropDownItems.size() - 1;
	SetCommonProperty(item, index);
	if (mDropDownItems.size() == 1)
	{
		OnItemSelected(item.get());
	}
	if (mReservedIdx != -1 && mReservedIdx < mDropDownItems.size())
	{
		unsigned resv = mReservedIdx;
		mReservedIdx = -1;
		SetSelectedIndex(resv);
	}

	return index;
}

void DropDown::ClearDropDownItems(){
	auto holder = mHolder.lock();
	if (holder){
		holder->RemoveAllChildren();
	}	
	mDropDownItems.clear();
	mIndexKeyMap.clear();
	SetProperty(UIProperty::TEXT, "");
	CreateHolder();
}

void DropDown::SetCommonProperty(WinBasePtr item, size_t index)
{
	item->SetVisible(false);
	item->SetProperty(UIProperty::NO_BACKGROUND, "false");
	item->SetProperty(UIProperty::BACK_COLOR, "0, 0, 0, 1.0");
	item->SetProperty(UIProperty::BACK_COLOR_OVER, "0.1, 0.1, 0.1, 1.0");
	item->SetProperty(UIProperty::USE_SCISSOR, "false");
	item->SetProperty(UIProperty::SPECIAL_ORDER, "3");
	item->SetInitialOffset(Vec2I(0, ITEM_HEIGHT * index));
	item->ChangeSizeY(ITEM_HEIGHT);
	
	auto holder = mHolder.lock();
	if (holder && holder->GetSize().y < (int)mDropDownItems.size() * ITEM_HEIGHT){
		int setheight = std::min(mMaxHeight, (int)mDropDownItems.size() * ITEM_HEIGHT);
		holder->ChangeSizeY(setheight);
	}

	item->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&DropDown::OnItemSelected, this, std::placeholders::_1));
}

size_t DropDown::GetSelectedIndex() const
{
	return mCurIdx;
}

void DropDown::SetSelectedIndex(size_t index)
{
	if (index >= mDropDownItems.size())
	{
		assert(0);
		return;
	}

	mCurIdx = index;
	auto item = mDropDownItems[index].lock();
	assert(item);
	SetText(item->GetText());
}

void DropDown::SetReservedIndex(size_t index)
{
	mReservedIdx = index;
}

bool DropDown::OnInputFromHandler(IInputInjectorPtr injector)
{
	if (mDropDownItems.empty())
		return __super::OnInputFromHandler(injector);

	if (injector->IsValid(InputDevice::Keyboard))
	{
		if (injector->IsKeyPressed(VK_ESCAPE))
		{
			auto holder = mHolder.lock();
			if (holder->GetVisible()){
				SetVisibleDropDownItems(false);
				injector->Invalidate(InputDevice::Keyboard);
			}
		}
	}
	return __super::OnInputFromHandler(injector);
}

void DropDown::OnParentVisibleChanged(bool show)
{
	if (!show)
	{
		SetVisibleDropDownItems(false);		
	}
}

void DropDown::ModifyItem(unsigned index, UIProperty::Enum prop, const char* szString){
	if (index >= mDropDownItems.size()) {
		Logger::Log(FB_ERROR_LOG_ARG, FormatString(
			"out of index(%u)", index).c_str());
		return;
	}
	mDropDownItems[index].lock()->SetProperty(prop, szString);
	if (mCurIdx == index){
		SetProperty(prop, szString);
	}
}

const wchar_t* DropDown::GetItemString(unsigned index){
	if (index < mDropDownItems.size()){
		return mDropDownItems[index].lock()->GetText();
	}
	else{
		Error("DropDown::GetItemString : invalid index(%u)", index);
	}
	return L"";
}

unsigned DropDown::GetKey(unsigned index) const{
	auto it = mIndexKeyMap.find(index);
	if (it != mIndexKeyMap.end())
		return it->second;

	return -1;
}

}