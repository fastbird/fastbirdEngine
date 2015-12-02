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
#include "ListItem.h"
#include "PropertyList.h"
#include "CheckBox.h"
#include "UIObject.h"

namespace fb{

//-----------------------------------------------------------------------------
const float ListItem::LEFT_GAP = 0.001f;
const size_t ListItem::INVALID_INDEX = -1;

ListItemPtr ListItem::Create(){
	ListItemPtr p(new ListItem, [](ListItem* obj){ delete obj; });
	p->mSelfPtr = p;
	return p;
}

ListItem::ListItem()
	: mRowIndex(INVALID_INDEX)
	, mColIndex(INVALID_INDEX)
	, mNoBackground(true)
	, mBackColor("0.1, 0.3, 0.3, 0.7")
	, mMerged(false)
{
	mUIObject = UIObject::Create(GetRenderTargetSize());
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
}

ListItem::~ListItem()
{

}

void ListItem::RegisterMouseHoverEvent(){
	RegisterEventFunc(UIEvents::EVENT_MOUSE_HOVER,
		std::bind(&ListItem::OnMouseHover, this, std::placeholders::_1));
}

void ListItem::SetRowIndex(size_t index)
{
	mRowIndex = index;
}
void ListItem::GatherVisit(std::vector<UIObject*>& v)
{
	v.push_back(mUIObject.get());

	__super::GatherVisit(v);
}

CheckBoxPtr ListItem::GetCheckBox() const
{
	for (auto& child : mChildren)
	{
		if (child->GetType() == ComponentType::CheckBox)
		{
			return std::dynamic_pointer_cast<CheckBox>(child);
		}
	}
	return 0;
}

void ListItem::SetBackColor(const char* backColor)
{
	if_assert_fail(backColor)
		return;
	mBackColor = backColor;
}

void ListItem::SetNoBackground(bool noBackground)
{
	mNoBackground = noBackground;
}

void ListItem::OnFocusGain()
{
	auto parent = GetParent();
	if (parent && parent->GetType() == ComponentType::PropertyList)
	{
		auto prop = std::static_pointer_cast<PropertyList>(parent);
		prop->SetFocusRow(mRowIndex);
	}
	//SetProperty(UIProperty::NO_BACKGROUND, "false");
	TriggerRedraw();
}

void ListItem::OnFocusLost()
{
	//SetProperty(UIProperty::NO_BACKGROUND, "true");
	auto parent = GetParent();
	if (parent && parent->GetType() == ComponentType::PropertyList)
	{

	}
	TriggerRedraw();
}

void ListItem::OnMouseHover(void* arg)
{
	SetCursor(WinBase::sCursorOver);
}

//bool ListItem::OnInputFromHandler(IInputInjectorPtr injector)
//{
//	if (!GetFocus(true))
//		return mMouseIn;
//
//	bool mousein = __super::OnInputFromHandler(mouse, keyboard);
//
//	if (keyboard->IsValid() && mParent && mParent->GetType() == ComponentType::PropertyList)
//	{
//		ListBox* listbox = (ListBox*)mParent;
//		auto c = keyboard->GetChar();
//		if (c)
//		{
//			keyboard->PopChar();
//			keyboard->Invalidate();
//			if (c == VK_TAB)
//			{
//				if (keyboard->IsKeyDown(VK_SHIFT))
//					listbox->MoveFocusToEdit(mRowIndex - 1);
//				else
//					listbox->MoveFocusToEdit(mRowIndex);
//			}
//			else
//			{
//				prop->GoToNext(c, mRowIndex);
//			}
//		}
//
//		if (keyboard->IsKeyPressed(VK_DOWN)){
//			prop->MoveLine(false, true);
//			keyboard->Invalidate();
//		}
//		else if (keyboard->IsKeyPressed(VK_UP)) 	{
//			prop->MoveLine(false, false);
//			keyboard->Invalidate();
//		}
//		else if (keyboard->IsKeyPressed(VK_RIGHT)){
//			prop->MoveFocusToEdit(mRowIndex);
//			keyboard->Invalidate();
//		}
//	}	
//
//	return mousein;
//}

}
