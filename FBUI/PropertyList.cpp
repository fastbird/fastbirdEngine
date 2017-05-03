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
#include "PropertyList.h"
#include "Scroller.h"
#include "UIManager.h"
#include "UIObject.h"
#include "TextField.h"
#include "ListItem.h"
#include "ListBoxData.h"
#include "FBInputManager/TextManipulator.h"

namespace fb
{
PropertyListPtr PropertyList::Create(){
	PropertyListPtr p(new PropertyList, [](PropertyList* obj){ delete obj; });
	p->mSelfPtr = p;
	return p;
}
PropertyList::PropertyList()
	: ListBox()
	, mFocusRow(-1)
{
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	mRowHeight = 22;
	mNumCols = 2;
	mColSizes.clear();
	mColSizes.push_back(0.47f);
	mColSizes.push_back(0.50f);
}
PropertyList::~PropertyList()
{
}

void PropertyList::OnCreated()
{
	if (mData)
	{
		Clear();
	}

	mData = FB_NEW(ListBoxDataSet)(mNumCols);

	mColSizesInt.clear();
	auto parentX = GetParentSize().x;
	mColSizesInt.push_back(Round(0.47f * parentX));
	mColSizesInt.push_back(Round(0.50f * parentX));
	SetChildrenContentEndFunc(std::bind(&ListBox::GetChildrenContentEnd, this));
}

const wchar_t* PropertyList::GetValue(const wchar_t* key)
{
	return mData->GetValueWithKey(key).c_str();
}

unsigned PropertyList::InsertItem(const wchar_t* key, const wchar_t* value)
{
	if (!mData)
	{
		mData = FB_NEW(ListBoxDataSet)(mNumCols);
	}
	unsigned index = mData->AddPropertyListData(key, key, value);
	while (mItems.size() <= index)
	{
		mItems.push_back(ROW_WEAK());
		mItems.back().push_back(ListItemWeakPtr());
		mItems.back().push_back(ListItemWeakPtr());
	}
	VisualizeData(index);
	return index;
}

unsigned PropertyList::ModifyItem(const wchar_t* key, const wchar_t* value)
{
	if (!key || !value)
		return -1;
	
	unsigned index = mData->AddPropertyListData(key, key, value);
	VisualizeData(index);
	return index;
}

bool PropertyList::GetCurKeyValue(std::string& key, std::string& value)
{
	if (mFocusRow == -1)
		return false;
	auto pdata = mData->GetData(mFocusRow);
	if (pdata)
	{
		key = WideToAnsi(pdata[0].GetText());
		value = WideToAnsi(pdata[1].GetText());
		return true;
	}
	return false;
}

void PropertyList::MoveFocusToEdit(unsigned index)
{
	if (index >= mData->Size()) 	{
		index = mData->Size()-1;
	}
	else{

	}
	auto scroll = mScrollerV.lock();
	//scroll
	if (scroll)
	{
		if (index < mStartIndex+1 || index >(mEndIndex>=3 ? mEndIndex - 3 : mEndIndex))
		{
			unsigned hgap = mRowHeight + mRowGap;
			unsigned destY = hgap * index + mRowGap;

			scroll->SetOffset(Vec2(0.f, -(destY / (float)GetRenderTargetSize().y)));
		}
	}

	auto second = mItems[index][1].lock();
	if (second)
	{
		UIManager::GetInstance().SetFocusUI(second->GetChild((unsigned)0));
		UIManager::GetInstance().GetTextManipulator()->SelectAll();
		TriggerRedraw();
	}
}

void PropertyList::MoveLine(bool applyInput, bool next)
{
	// apply value;
	if (applyInput)
		OnEvent(UIEvents::EVENT_ENTER);

	unsigned nextLine = next ? mFocusRow + 1 : mFocusRow - 1;
	if (nextLine == -1)
	{
		nextLine = mData->Size() - 1;
	}
	else if (nextLine >= mData->Size())
	{
		nextLine = 0;
	}
	//scroll
	auto scroll = mScrollerV.lock();
	if (scroll)
	{
		if (nextLine < mStartIndex+1 ||
			(nextLine >(mEndIndex >= 3 ? mEndIndex - 3 : mEndIndex))
			)
		{
			unsigned hgap = mRowHeight + mRowGap;
			unsigned destY = hgap * nextLine + mRowGap;

			scroll->SetOffset(Vec2(0.f, -(destY / (float)GetRenderTargetSize().y)));
		}
	}

	auto item = mItems[nextLine][0].lock();
	if (item)
	{
		UIManager::GetInstance().SetFocusUI(item);
	}

	mFocusRow = nextLine;	
	
}

void PropertyList::RemoveHighlight(unsigned index)
{
	auto item = mItems[index][0].lock();
	if (item)
	{
		item->SetProperty(UIProperty::NO_BACKGROUND, "true");
	}
}

void PropertyList::MoveFocusToKeyItem()
{
	auto item = mItems[mFocusRow][0].lock();
	if (item)
	{
		UIManager::GetInstance().SetFocusUI(item);
	}
}
//
//void PropertyList::VisualizeData(unsigned index){
//	if (!mData)
//		return;
//	if (index >= mItems.size())
//		return;
//
//	__super::VisualizeData(index);
//	
//	auto keyItem = mItems[index][0];
//	if (keyItem) {
//		if (index == mFocusRow)
//		{
//			keyItem->SetProperty(UIProperty::NO_BACKGROUND, "false");
//		}
//		else
//		{
//			keyItem->SetProperty(UIProperty::NO_BACKGROUND, "true");
//		}
//	}
//
//}

}