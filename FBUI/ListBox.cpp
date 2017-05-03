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
#include "ListBox.h"
#include "Scroller.h"
#include "ImageBox.h"
#include "Button.h"
#include "CheckBox.h"
#include "ListItem.h"
#include "ListBoxData.h"
#include "TextField.h"
#include "UIManager.h"
#include "NumericUpDown.h"
#include "HorizontalGauge.h"
#include "UIObject.h"
#include "FBInputManager/TextManipulator.h"


namespace fb
{

ListBoxPtr ListBox::Create(){
	ListBoxPtr p(new ListBox, [](ListBox* obj){ delete obj; });
	p->mSelfPtr = p;
	return p;
}

//-----------------------------------------------------------------------------
ListBox::ListBox()
	: mNumCols(0)
	, mRowHeight(24)
	, mRowGap(4)
	, mHighlightColor("0.1, 0.3, 0.3, 0.7")
	, mData(0)
	, mStartIndex(0)
	, mEndIndex(10)
	, mLastChangedItem(-1, -1), mNoSearch(false)	
{
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	mColSizes.push_back(0.97f);
	mColSizes.push_back(10);
	SetProperty(UIProperty::SCROLLERV, "true");
	mMultiSelection = GetDefaultValueBool(UIProperty::LISTBOX_MULTI_SELECTION);
	mNoHighlight = GetDefaultValueBool(UIProperty::LISTBOX_NO_HIGHLIGHT);
	mHighlightOnHover = GetDefaultValueBool(UIProperty::LISTBOX_HIGHLIGHT_ON_HOVER);
	mHand = GetDefaultValueBool(UIProperty::LISTBOX_HAND);
	SetProperty(UIProperty::BACK_COLOR, UIManager::GetInstance().GetStyleString(Styles::ListBoxBack));
}

ListBox::~ListBox()
{
	Clear(false);
	FB_DELETE(mData);
}

std::string ListBox::GetSelectedString()
{
	auto focusedItem = mFocusedListItem.lock();
	if (focusedItem)
	{
		unsigned row = focusedItem->GetRowIndex();
		unsigned col = focusedItem->GetColIndex();
		return WideToAnsi(mData->GetData(row)[col].GetText());

	}
	return std::string();
}

ListItemPtr ListBox::CreateNewItem(int row, int col)
{
	int hgap = mRowHeight + mRowGap;
	int y = hgap * row + mRowGap;
	int x = 0;	
	const auto& finalSize = GetFinalSize();
	for (int c = 0; c < col; ++c) {
		x += mColSizesInt[c];
	}	
	
	auto item = std::static_pointer_cast<ListItem>(AddChild(Vec2I(x, y), Vec2I(mColSizesInt[col], mRowHeight), ComponentType::ListItem));
	item->SetUseAbsXSize(true);
	item->SetUseAbsXPos(true);
	item->SetRuntimeChild(true);
	item->SetRuntimeChildRecursive(true);
	if (col < (int)mColAlignes.size())
		item->SetProperty(UIProperty::TEXT_ALIGN, mColAlignes[col].c_str());
	if (col < (int)mTextSizes.size())
	{
		item->SetProperty(UIProperty::TEXT_SIZE, mTextSizes[col].c_str());
	}
	item->SetProperty(UIProperty::BACK_COLOR, mHighlightColor.c_str());
	item->SetProperty(UIProperty::NO_BACKGROUND, "true");
	item->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&ListBox::OnItemClicked, this, std::placeholders::_1));
	item->RegisterEventFunc(UIEvents::EVENT_MOUSE_RIGHT_CLICK,
		std::bind(&ListBox::OnItemRClicked, this, std::placeholders::_1));
	item->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_DOUBLE_CLICK,
		std::bind(&ListBox::OnItemDoubleClicked, this, std::placeholders::_1));
	item->SetVisible(mVisibility.IsVisible());
	item->SetRowIndex(row);
	item->SetColIndex(col);
	item->RegisterMouseHoverEvent();

	return item;
}

unsigned ListBox::InsertItem(unsigned uniqueKey){
	if (!mData)
		return -1;
	unsigned index = mData->InsertData(uniqueKey);
	while (mItems.size() <= index)
	{
		mItems.push_back(ROW_WEAK());
		auto& row = mItems.back();
		for (unsigned c = 0; c < mNumCols; ++c){
			row.push_back(ListItemWeakPtr());
		}
	}
	RefreshVScrollbar();
	return index;
}

unsigned ListBox::InsertItem(const wchar_t* uniqueKey)
{	
	if (!mData)
		return -1;
	unsigned index = mData->InsertData(uniqueKey);
	while (mItems.size() <= index)
	{
		mItems.push_back(ROW_WEAK());
		auto& row = mItems.back();
		for (unsigned c = 0; c < mNumCols; ++c){
			row.push_back(ListItemWeakPtr());
		}
	}
	RefreshVScrollbar();

	return index;
}

unsigned ListBox::InsertEmptyData(){
	unsigned index = mData->InsertEmptyData();
	while (mItems.size() <= index)
	{
		mItems.push_back(ROW_WEAK());
		auto& row = mItems.back();
		for (unsigned c = 0; c < mNumCols; ++c){
			row.push_back(ListItemWeakPtr());
		}
	}
	RefreshVScrollbar();
	return index;
}

bool ListBox::ModifyKey(unsigned row, unsigned key){
	if (!mData) {
		Logger::Log(FB_ERROR_LOG_ARG, "No data prepared.");
		return false;
	}
	assert(row < mData->Size());
	return mData->ModifyKey(row, key);
}

void ListBox::SetItem(const Vec2I& rowcol, const wchar_t* string, ListItemDataType::Enum type){
	if (!mData) {
		Logger::Log(FB_ERROR_LOG_ARG, "No data prepared.");
		return;
	}
	mData->SetData(rowcol, string, type);
	auto dat = mData->GetData(rowcol.x);
	if (dat) {
		if (wcscmp(dat[rowcol.y].GetText(), string) != 0) {
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid data");
		}
	}
	else {
		Logger::Log(FB_ERROR_LOG_ARG, "no data");
	}
	VisualizeData(rowcol.x);

	if (!(!GetVisible() || !mData || mItems.empty())) {
		if (!mItems[rowcol.x][0].expired()) {
			auto item = mItems[rowcol.x][rowcol.y].lock();
			if (wcscmp(item->GetText(), string) != 0) {
				Logger::Log(FB_ERROR_LOG_ARG, "Invalid reflection");
			}
		}
		else {
			if (rowcol.x >= mStartIndex && rowcol.x <= mEndIndex) {
				Logger::Log(FB_ERROR_LOG_ARG, "Invalid index");
			}
		}
	}
}

void ListBox::SetItem(const Vec2I& rowcol, bool checked){
	if (!mData) {
		Logger::Log(FB_ERROR_LOG_ARG, "No data prepared.");
		return;
	}
	mData->SetData(rowcol, checked);
	VisualizeData(rowcol.x);
}

void ListBox::SetItem(const Vec2I& rowcol, TexturePtr texture){
	if (!mData) {
		Logger::Log(FB_ERROR_LOG_ARG, "No data prepared.");
		return;
	}
	mData->SetData(rowcol, texture);
	VisualizeData(rowcol.x);
}

// numeric updown
void ListBox::SetItem(const Vec2I& rowcol, int number){
	if (!mData) {
		Logger::Log(FB_ERROR_LOG_ARG, "No data prepared.");
		return;
	}
	mData->SetData(rowcol, number);
	VisualizeData(rowcol.x);
}

void ListBox::SetItem(const Vec2I& rowcol, float number){
	if (!mData) {
		Logger::Log(FB_ERROR_LOG_ARG, "No data prepared.");
		return;
	}
	mData->SetData(rowcol, number);
	VisualizeData(rowcol.x);
}

bool ListBox::GetCheckBox(const Vec2I& indexRowCol) const{
	if (!mData){
		Logger::Log(FB_ERROR_LOG_ARG, "No data");		
		return false;
	}
	if ((unsigned)indexRowCol.x >= mData->Size()){
		Logger::Log(FB_ERROR_LOG_ARG, "Invalid row index");
		return false;
	}
	if ((unsigned)indexRowCol.y >= mNumCols){
		Logger::Log(FB_ERROR_LOG_ARG, "Invalid col index");
		return false;
	}

	auto data = mData->GetData(indexRowCol.x);	
	auto item = data[indexRowCol.y];
	return item.GetChecked();
}

bool ListBox::RemoveRow(const wchar_t* uniqueKey)
{
	if (!mData)
		return false;
	unsigned deletedIndex = mData->DelDataWithKey(uniqueKey);
	auto focusedItem = mFocusedListItem.lock();
	if (focusedItem && focusedItem->GetRowIndex() == deletedIndex)
		ChangeFocusItem(0);

	if (deletedIndex != -1)
	{
		for (auto it = mSelectedIndices.begin(); it != mSelectedIndices.end(); ++it)
		{
			if (*it == deletedIndex)
				SetHighlightRow(deletedIndex, false);
		}
		VisualizeData(deletedIndex);
		return true;
	}
	return false;
}

bool ListBox::RemoveRow(unsigned uniqueKey){
	if (!mData)
		return false;
	unsigned deletedIndex = mData->DelDataWithKey(uniqueKey);
	auto focusedItem = mFocusedListItem.lock();
	if (focusedItem && focusedItem->GetRowIndex() == deletedIndex)
		ChangeFocusItem(0);
	if (deletedIndex != -1)
	{
		for (auto it = mSelectedIndices.begin(); it != mSelectedIndices.end(); ++it)
		{
			if (*it == deletedIndex)
				SetHighlightRow(deletedIndex, false);
		}
		for (unsigned i = deletedIndex; i <= mEndIndex; ++i){
			VisualizeData(i);
		}
		return true;
	}
	return false;
}

bool ListBox::RemoveRowWithIndex(unsigned index){
	if (!mData)
		return false;
	if (index >= mItems.size())
	{
		Logger::Log(FB_ERROR_LOG_ARG, "Out of index.");
		return false;
	}
	unsigned deletedIndex = mData->DelDataWithIndex(index);
	auto focusedItem = mFocusedListItem.lock();
	if (focusedItem && focusedItem->GetRowIndex() == deletedIndex)
		ChangeFocusItem(0);
	if (deletedIndex != -1)
	{
		for (auto it = mSelectedIndices.begin(); it != mSelectedIndices.end(); ++it)
		{
			if (*it == deletedIndex)
				SetHighlightRow(deletedIndex, false);
		}

		VisualizeData(deletedIndex);
		return true;
	}
	return false;
}

void ListBox::SetHighlightRow(size_t row, bool highlight)
{
	if (mNoHighlight)
		return;
	for (size_t i = 0; i < mNumCols; ++i)
	{
		if (mItems[row].size() <= i || mItems[row][i].expired())
			break;

		mHighlighted[Vec2I((int)row, (int)i)] = highlight;

		auto item = mItems[row][i].lock();
		if (highlight)
		{
			item->SetProperty(UIProperty::NO_BACKGROUND, "false");
			item->SetProperty(UIProperty::BACK_COLOR, mHighlightColor.c_str());
		}
		else
		{
			item->SetProperty(UIProperty::NO_BACKGROUND, StringConverter::ToString(item->GetNoBackground()).c_str());
			item->SetProperty(UIProperty::BACK_COLOR, item->GetBackColor());
		}
	}
}

void ListBox::SetHighlightRowCol(unsigned row, unsigned col, bool highlight)
{
	if (row >= mData->Size() || col >= mNumCols)
		return;
	
	mHighlighted[Vec2I((int)row, (int)col)] = highlight;

	auto item = mItems[row][col].lock();
	if (!item)
	{
		return;
	}
	if (highlight)
	{
		item->SetProperty(UIProperty::NO_BACKGROUND, "false");
		item->SetProperty(UIProperty::BACK_COLOR, mHighlightColor.c_str());
	}
	else
	{
		item->SetProperty(UIProperty::NO_BACKGROUND, StringConverter::ToString(item->GetNoBackground()).c_str());
		item->SetProperty(UIProperty::BACK_COLOR, item->GetBackColor());
	}
}

void ListBox::SetHighlightRowAndSelect(size_t row, bool highlight)
{
	if (highlight)
	{
		if (!ValueExistsInVector(mSelectedIndices, row))
		{
			mSelectedIndices.push_back(row);
			auto focusedItem = mFocusedListItem.lock();
			if (focusedItem){
				ChangeFocusItem(mItems[row][focusedItem->GetColIndex()].lock());
			}
			else{
				ChangeFocusItem(mItems[row][0].lock());
			}
		}
	}
	else
	{
		DeleteValuesInVector(mSelectedIndices, row);
	}
	SetHighlightRow(row, highlight);
}

void ListBox::Clear(bool immediately)
{
	ChangeFocusItem(0);
	for (auto items : mItems)
	{
		for (auto item : items)
		{
			RemoveChild(item.lock(), immediately);
		}
	}
	mItems.clear();
	mRecycleBin.clear();
	if (mData)
		mData->Clear();
	mSelectedIndices.clear();
	mHighlighted.clear();
	TriggerRedraw();
	if (UIManager::HasInstance())
		UIManager::GetInstance().DirtyRenderList(GetHwndId());
	
	auto scrollerV = mScrollerV.lock();
	auto contentUI = mWndContentUI.lock();
	if (!scrollerV && contentUI){
		scrollerV = contentUI->GetScrollerV();
	}

	if (scrollerV){
		scrollerV->SetOffset(Vec2(0.f, 0.f));
	}
}

size_t ListBox::GetNumData() const{
	if (!mData)
		return 0;
	return mData->Size();
}

bool ListBox::SetProperty(UIProperty::Enum prop, const char* val)
{
	switch (prop)
	{
	case UIProperty::HIGHLIGHT_COLOR:
	{
		mHighlightColor = val;
		return true;
	}
	case UIProperty::LISTBOX_COL:
	{
		mStrCols = val;
		mNumCols = StringConverter::ParseUnsignedInt(val);
		float colsize = 1.0f / (float)mNumCols;
		mColSizes.clear();
		mColSizesInt.clear();
		auto xsize = GetFinalSize().x;
		for (unsigned i = 0; i < mNumCols; ++i)
		{
			mColSizes.push_back(colsize);
			mColSizesInt.push_back(Round(colsize * xsize));
			mColAlignes.push_back("center");
		}
		if (mData){
			Clear();
			FB_DELETE(mData);
		}
		mData = FB_NEW( ListBoxDataSet(mNumCols) );
		UIManager::GetInstance().DirtyRenderList(GetHwndId());
		UpdateColSizes();		
		return true;
	}
	case UIProperty::LISTBOX_ROW_HEIGHT:
		{
			mRowHeight = StringConverter::ParseInt(val);
			return true;
		}
	case UIProperty::LISTBOX_ROW_GAP:
	{
			mRowGap = StringConverter::ParseInt(val);
			return true;
	}

	case UIProperty::LISTBOX_COL_SIZES:
		{
			// set UIProperty::LISTBOX_COL first
			// don't need to set this property if the num of col is 1.
			mStrColSizes = val;
			mColSizes.clear();
			mColSizesInt.clear();
			StringVector strs = Split(val);
			auto xsize = GetParentSize().x;
			assert(!strs.empty());
			for (unsigned i = 0; i < strs.size(); ++i)
			{
				mColSizes.push_back(StringConverter::ParseReal(strs[i]));
				mColSizesInt.push_back(Round(mColSizes.back() * xsize));
			}
			UpdateColSizes();
			return true;
		}

	case UIProperty::LISTBOX_TEXT_SIZES:
	{
		mStrTextSizes = val;
										  // set UIProperty::LISTBOX_COL first
										  mTextSizes.clear();
										  StringVector strs = Split(val);
										  assert(!strs.empty());
										  for (unsigned i = 0; i < strs.size(); ++i)
										  {
											  mTextSizes.push_back(strs[i]);
										  }
										  return true;
	}
	case UIProperty::LISTBOX_COL_ALIGNH:
		{
			mStrColAlignH = val;			
			mColAlignes.clear();
			mColAlignes.reserve(mNumCols);
			StringVector strs = Split(val);
			assert(!strs.empty());
			unsigned i = 0;
			for (; i < strs.size(); ++i)
			{
				mColAlignes.push_back(strs[i]);
			}
			const char* lastAlign = "center";
			if (!mColAlignes.empty())
				lastAlign = mColAlignes.back().c_str();
			while (mColAlignes.size() < mNumCols)
			{
				mColAlignes.push_back(lastAlign);
			}
			UpdateItemAlign();
			return true;
		}

	case UIProperty::LISTBOX_COL_HEADERS_TEXT_SIZE:
	{
		mStrHeaderTextSizes = val;
										   assert(mNumCols != 1);
										   mHeaderTextSize.clear();
										   StringVector strs = Split(val);
										   assert(!strs.empty());
										   for (unsigned i = 0; i < strs.size(); ++i)
										   {
											   mHeaderTextSize.push_back(strs[i]);
										   }
										   return true;
	}

	case UIProperty::LISTBOX_COL_HEADERS:
		{
			mStrHeaders = val;
			StringVector strs = Split(val, ",");
			auto numCols = std::min(strs.size(), mNumCols);
			const auto& finalSize = GetFinalSize();
			auto contentWndBackup = mWndContentUI.lock();
			mWndContentUI.reset();
			for (auto& h : mHeaders){
				RemoveChild(h.lock());
			}
			mHeaders.clear();
			const char* headerBack = UIManager::GetInstance().GetStyleString(Styles::ListBoxHeaderBack);
			assert(headerBack);
			for (unsigned i = 0; i < numCols; ++i)
			{
				int posX = 0;
				if (i >= 1)
				{
					posX = mHeaders[i - 1].lock()->GetPos().x + mHeaders[i - 1].lock()->GetFinalSize().x;
				}

				auto pAddedItem = std::static_pointer_cast<ListItem>(
					AddChild(Vec2I(posX, 0), Vec2I(mColSizesInt[i], mRowHeight), ComponentType::ListItem));
				mHeaders.push_back(pAddedItem);				
				pAddedItem->RegisterEventFunc(UIEvents::EVENT_MOUSE_DRAG,
					std::bind(&ListBox::OnDragHeader, this, std::placeholders::_1));
				pAddedItem->SetVisible(GetVisible());
				pAddedItem->SetRuntimeChild(true);
				pAddedItem->SetRuntimeChildRecursive(true);
				pAddedItem->SetUseAbsXSize(false);
				pAddedItem->SetUseAbsXPos(false);
				const Rect& rect = mUIObject->GetRegion();
				pAddedItem->SetProperty(UIProperty::NO_BACKGROUND, "false");
				pAddedItem->SetProperty(UIProperty::BACK_COLOR, headerBack);
				if (!mHeaderTextSize.empty() && i < mHeaderTextSize.size())
				{
					pAddedItem->SetProperty(UIProperty::TEXT_SIZE, mHeaderTextSize[i].c_str());
				}
					
				pAddedItem->SetProperty(UIProperty::TEXT_ALIGN, "center");
				pAddedItem->SetRowIndex(-1);
				pAddedItem->SetColIndex(i);
				pAddedItem->SetProperty(UIProperty::TEXT, strs[i].c_str());
			}

			mWndContentUI = contentWndBackup;
			auto contentUI = mWndContentUI.lock();
			if (!contentUI){
				bool prevScrollerV = mUseScrollerV;
				if (mUseScrollerV)
				{
					mUseScrollerV = false;
					if (mUseScrollerV)
						RemoveChild(mScrollerV.lock(), true);
					mScrollerV.reset();
				}
				auto wndContentUI = std::static_pointer_cast<Wnd>(AddChild(0.0f, 0.0f, 1.0f, 1.0f, ComponentType::Window));
				mWndContentUI = wndContentUI;
				wndContentUI->SetRuntimeChild(true);
				wndContentUI->SetRuntimeChildRecursive(true);
				Vec2I sizeMod = { 0, -(mRowHeight + 4) };
				wndContentUI->ModifySize(sizeMod);
				wndContentUI->SetUseAbsSize(false);
				wndContentUI->ChangePos(Vec2I(0, (mRowHeight + 4)));
				wndContentUI->SetProperty(UIProperty::NO_BACKGROUND, "true");
				wndContentUI->SetChildrenContentEndFunc(std::bind(&ListBox::GetChildrenContentEnd, this));
				wndContentUI->SetScrolledFunc(std::bind(&ListBox::Scrolled, this));
				if (prevScrollerV)
				{					
					wndContentUI->SetProperty(UIProperty::SCROLLERV, "true");
				}
			}
			UIManager::GetInstance().DirtyRenderList(GetHwndId());
			return true;
		}

	case UIProperty::LISTBOX_MULTI_SELECTION:
	{
		mMultiSelection = StringConverter::ParseBool(val);
		return true;
	}

	case UIProperty::LISTBOX_NO_HIGHLIGHT:
	{
		mNoHighlight = StringConverter::ParseBool(val);
		return true;
	}
	case UIProperty::TEXTUREATLAS:
	{
										assert(val);
										mTextureAtlas = val;
										return true;
	}
	case UIProperty::LISTBOX_NO_SEARCH:
	{
		mNoSearch = StringConverter::ParseBool(val);
		return true;
	}
	case UIProperty::LISTBOX_HAND:
	{
		mHand = StringConverter::ParseBool(val);
		return true;
	}
	case UIProperty::LISTBOX_HIGHLIGHT_ON_HOVER:
	{
		mHighlightOnHover = StringConverter::ParseBool(val);
		return true;
	}
	case UIProperty::SCROLLERV_OFFSET:
	{
		auto contentWnd = mWndContentUI.lock();
		if (contentWnd) {
			return contentWnd->SetProperty(prop, val);
		}
		auto scrollerV = mScrollerV.lock();
		if (scrollerV)
		{
			auto offset = scrollerV->GetOffset();
			offset.y = StringConverter::ParseReal(val);
			scrollerV->SetOffset(offset);
		}
		return true;
	}
	}

	return __super::SetProperty(prop, val);
}

bool ListBox::GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly)
{
	switch (prop)
	{
	case UIProperty::HIGHLIGHT_COLOR:
	{
		if (notDefaultOnly)
		{
			if (mHighlightColor == UIProperty::GetDefaultValueString(prop))
				return false;
		}
		strcpy_s(val, bufsize, mHighlightColor.c_str());
		return true;
	}
	case UIProperty::LISTBOX_COL:
	{
		if (notDefaultOnly)
		{
			if (mStrCols.empty())
				return false;
		}
		strcpy_s(val, bufsize, mStrCols.c_str());
		return true;
	}
	case UIProperty::LISTBOX_ROW_HEIGHT:
	{
		if (notDefaultOnly)
		{
			if (mRowHeight == UIProperty::GetDefaultValueInt(prop))
				return false;
		}
		auto data = StringConverter::ToString(mRowHeight);
		strcpy_s(val, bufsize, data.c_str());
		return true;

		
	}
	case UIProperty::LISTBOX_ROW_GAP:
	{
		if (notDefaultOnly)
		{
			if (mRowGap == UIProperty::GetDefaultValueInt(prop))
				return false;
		}
		auto data = StringConverter::ToString(mRowGap);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}

	case UIProperty::LISTBOX_COL_SIZES:
	{
		if (notDefaultOnly)
		{
			if (mStrColSizes.empty())
				return false;
		}
		strcpy_s(val, bufsize, mStrColSizes.c_str());
		return true;
	}

	case UIProperty::LISTBOX_TEXT_SIZES:
	{
		if (notDefaultOnly)
		{
			if (mStrTextSizes.empty())
				return false;
		}
		strcpy_s(val, bufsize, mStrTextSizes.c_str());
		return true;
	}
	case UIProperty::LISTBOX_COL_ALIGNH:
	{
		if (notDefaultOnly)
		{
			if (mStrColAlignH.empty())
				return false;
		}
		strcpy_s(val, bufsize, mStrColAlignH.c_str());
		return true;
	}

	case UIProperty::LISTBOX_COL_HEADERS_TEXT_SIZE:
	{
		if (notDefaultOnly)
		{
			if (mStrHeaderTextSizes.empty())
				return false;
		}
		strcpy_s(val, bufsize, mStrHeaderTextSizes.c_str());
		return true;
	}

	case UIProperty::LISTBOX_COL_HEADERS:
	{
		if (notDefaultOnly)
		{
			if (mStrHeaders.empty())
				return false;
		}
		strcpy_s(val, bufsize, mStrHeaders.c_str());
		return true;
	}
	case UIProperty::LISTBOX_MULTI_SELECTION:
	{
		if (notDefaultOnly){
			if (mMultiSelection == UIProperty::GetDefaultValueBool(prop)){
				return false;
			}
		}

		strcpy_s(val, bufsize, StringConverter::ToString(mMultiSelection).c_str());
		return true;
	}

	case UIProperty::LISTBOX_NO_HIGHLIGHT:
	{
		if (notDefaultOnly){
			if (mNoHighlight == UIProperty::GetDefaultValueBool(prop)){
				return false;
			}
		}

		strcpy_s(val, bufsize, StringConverter::ToString(mNoHighlight).c_str());
		return true;
	}
	case UIProperty::TEXTUREATLAS:
	{
		if (notDefaultOnly)
		{
			if (mTextureAtlas.empty())
				return false;
		}
		strcpy_s(val, bufsize, mTextureAtlas.c_str());
		return true;
	}

	case UIProperty::LISTBOX_NO_SEARCH:
	{
		if (notDefaultOnly){
			if (mNoSearch == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		strcpy_s(val, bufsize, StringConverter::ToString(mNoSearch).c_str());
		return true;
	}

	case UIProperty::LISTBOX_HAND:
	{
		if (notDefaultOnly) {
			if (mHand == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		strcpy_s(val, bufsize, StringConverter::ToString(mHand).c_str());
		return true;
	}

	case UIProperty::LISTBOX_HIGHLIGHT_ON_HOVER:
	{
		if (notDefaultOnly) {
			if (mHighlightOnHover == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		strcpy_s(val, bufsize, StringConverter::ToString(mHighlightOnHover).c_str());
		return true;
	}

	case UIProperty::SCROLLERV_OFFSET:
	{
		if (notDefaultOnly)
			return false;
		auto contentWnd = mWndContentUI.lock();
		if (contentWnd) {
			return contentWnd->GetProperty(prop, val, bufsize, notDefaultOnly);
		}
		auto scrollerV = mScrollerV.lock();
		if (scrollerV)
		{
			sprintf_s(val, 256, "%.4f", scrollerV->GetOffset().y);
			return true;
		}
		else
		{
			sprintf_s(val, 256, "%.4f", 0.f);
			return true;
		}
	}
	}

	return __super::GetProperty(prop, val, bufsize, notDefaultOnly);

}
ListItemPtr ListBox::GetItem(const Vec2I& indexRowCol) const
{
	unsigned row = indexRowCol.x;
	unsigned col = indexRowCol.y;
	assert(row < mItems.size());
	assert(col < mItems[row].size());
	return mItems[row][col].lock();

}

void ListBox::SelectRow(unsigned index)
{
	if (index < mItems.size())
	{
		if (!ValueExistsInVector(mSelectedIndices, index))
		{
			SetHighlightRow(index, true);
			mSelectedIndices.push_back(index);
		}
	}
	else if (!mItems.empty())
	{
		unsigned idx = mItems.size() - 1;
		if (!ValueExistsInVector(mSelectedIndices, idx))
		{
			SetHighlightRow(idx, true);
			mSelectedIndices.push_back(idx);
		}
	}
	OnEvent(UIEvents::EVENT_MOUSE_LEFT_CLICK);
}

void ListBox::SelectId(unsigned id) {
	auto row = FindRowIndex(id);
	if (row != -1) {
		SelectRow(row);
	}
}

void ListBox::DeselectRow(unsigned index){
	if (index >= mItems.size())
		return;

	DeleteValuesInVector(mSelectedIndices, index);
	if (mSelectedIndicesDeletedAny){
		SetHighlightRow(index, false);
	}	
}

void ListBox::DeselectAll(){
	for (auto index : mSelectedIndices){
		SetHighlightRow(index, false);
	}
	mSelectedIndices.clear();
}

void ListBox::ToggleSelection(unsigned index){
	if (index >= mItems.size())
		return;
	if (!ValueExistsInVector(mSelectedIndices, index))
	{
		SelectRow(index);
	}
	else
	{
		DeselectRow(index);
	}
}

void ListBox::ClearSelection()
{
	for (auto& row : mSelectedIndices)
	{
		SetHighlightRow(row, false);
	}
	mSelectedIndices.clear();
	OnEvent(UIEvents::EVENT_LISTBOX_CLEARED);
}

bool ListBox::IsSelected(unsigned row)
{
	return ValueExistsInVector(mSelectedIndices, row);
}

bool ListBox::OnInputFromHandler(IInputInjectorPtr injector)
{
	if (!mVisibility.IsVisible())
		return false;

	if (mNoMouseEvent)
	{
		return false;
	}
	
	bool mouseIn = __super::OnInputFromHandler(injector);

	if (injector->IsValid(InputDevice::Keyboard) && GetFocus(true))
	{
		auto c = injector->GetChar();
		if (c)
		{
			if (c == VK_TAB)
			{
				injector->ClearBuffer();
				injector->Invalidate(InputDevice::Keyboard);
				bool next = injector->IsKeyDown(VK_SHIFT) ? false : true;
				bool apply = true;
				IterateItem(next, apply);
			}
			else if (!mNoSearch)
			{
				injector->PopChar();
				auto focusedItem = mFocusedListItem.lock();
				if (focusedItem) {
					SearchStartingChacrcter(c, focusedItem->GetRowIndex());
				}
				else{					
					SearchStartingChacrcter(c, 0);
				}
			}
		}

		if (injector->IsKeyPressed(VK_UP))
		{
			if (injector->IsKeyDown(VK_SHIFT))
			{
				if (mSelectedIndices.empty())
				{
					if (!mItems.empty())
					{
						unsigned rowIndex = mData->Size()-1;
						auto focusedItem = mFocusedListItem.lock();
						if (focusedItem)
						{
							rowIndex = focusedItem->GetRowIndex() - 1;
							if (rowIndex ==-1){
								rowIndex = mData->Size() - 1;
							}
						}
						MakeSureRangeFor(rowIndex);
						SetHighlightRowAndSelect(rowIndex, true);						
						injector->Invalidate(InputDevice::Keyboard);
						OnEvent(UIEvents::EVENT_MOUSE_LEFT_CLICK);
					}
				}
				else
				{
					unsigned lastRow = mSelectedIndices.back();
					if (lastRow != 0){
						unsigned dest = lastRow - 1;
						if (IsSelected(dest))
						{
							SetHighlightRow(lastRow, false);
							DeleteValuesInVector(mSelectedIndices, lastRow);
							injector->Invalidate(InputDevice::Keyboard);
							OnEvent(UIEvents::EVENT_MOUSE_LEFT_CLICK);
						}
						else
						{
							MakeSureRangeFor(dest);
							SetHighlightRowAndSelect(dest, true);
							injector->Invalidate(InputDevice::Keyboard);
							OnEvent(UIEvents::EVENT_MOUSE_LEFT_CLICK);
						}
					}
					else{
						MakeSureRangeFor(0);
					}
				}
			}
			else
			{
				if (mSelectedIndices.empty())
				{
					if (!mItems.empty())
					{
						unsigned rowIndex = mData->Size() - 1;
						auto focusedItem = mFocusedListItem.lock();
						if (focusedItem)
						{
							rowIndex = focusedItem->GetRowIndex() - 1;
							if (rowIndex == -1){
								rowIndex = mData->Size() - 1;
							}
						}
						MakeSureRangeFor(rowIndex);
						SetHighlightRowAndSelect(rowIndex, true);
						injector->Invalidate(InputDevice::Keyboard);
						OnEvent(UIEvents::EVENT_MOUSE_LEFT_CLICK);
					}
				}
				else
				{
					unsigned last = mSelectedIndices.back();
					if (last != 0)
					{
						ClearSelection();
						MakeSureRangeFor(last - 1);
						SetHighlightRowAndSelect(last-1, true);
						injector->Invalidate(InputDevice::Keyboard);
						OnEvent(UIEvents::EVENT_MOUSE_LEFT_CLICK);
					}
					else
					{
						MakeSureRangeFor(0);
					}
				}
			}
			OnEvent(UIEvents::EVENT_LISTBOX_SELECTION_CHANGED);
		}
		else if (injector->IsKeyPressed(VK_DOWN))
		{
			if (injector->IsKeyDown(VK_SHIFT))
			{
				if (mSelectedIndices.empty())
				{
					if (!mItems.empty())
					{
						unsigned rowIndex = 0;
						auto focusedItem = mFocusedListItem.lock();
						if (focusedItem)
						{
							rowIndex = focusedItem->GetRowIndex() + 1;
							if (rowIndex >= mData->Size()){
								rowIndex = 0;
							}
						}
						MakeSureRangeFor(rowIndex);
						SetHighlightRowAndSelect(rowIndex, true);
						injector->Invalidate(InputDevice::Keyboard);
						OnEvent(UIEvents::EVENT_MOUSE_LEFT_CLICK);
					}
				}
				else
				{
					unsigned lastRow = mSelectedIndices.back();
					if (lastRow+1 < mItems.size())
					{
						unsigned dest = lastRow + 1;
						if (IsSelected(dest))
						{
							SetHighlightRowAndSelect(lastRow, false);
							injector->Invalidate(InputDevice::Keyboard);
							OnEvent(UIEvents::EVENT_MOUSE_LEFT_CLICK);
						}
						else
						{
							MakeSureRangeFor(dest);
							SetHighlightRowAndSelect(dest, true);
							injector->Invalidate(InputDevice::Keyboard);
							OnEvent(UIEvents::EVENT_MOUSE_LEFT_CLICK);
						}
					}
				}
			} //if (keyboard->IsKeyDown(VK_SHIFT))
			else
			{
				if (mSelectedIndices.empty())
				{
					if (!mItems.empty())
					{
						unsigned rowIndex = 0;
						auto focusedItem = mFocusedListItem.lock();
						if (focusedItem)
						{
							rowIndex = focusedItem->GetRowIndex() + 1;
							if (rowIndex >= mData->Size()){
								rowIndex = 0;
							}
						}
						SetHighlightRowAndSelect(rowIndex, true);
						MakeSureRangeFor(rowIndex);
						injector->Invalidate(InputDevice::Keyboard);
						OnEvent(UIEvents::EVENT_MOUSE_LEFT_CLICK);
					}
				}
				else
				{
					unsigned last = mSelectedIndices.back();
					if (last +1 < mItems.size())
					{
						ClearSelection();
						SetHighlightRowAndSelect(last + 1, true);
						MakeSureRangeFor(last + 1);
						injector->Invalidate(InputDevice::Keyboard);
						OnEvent(UIEvents::EVENT_MOUSE_LEFT_CLICK);
					}
				}
			}
			OnEvent(UIEvents::EVENT_LISTBOX_SELECTION_CHANGED);
		}

	}

	return mouseIn;
}

void ListBox::ChangeFocusItem(ListItemPtr newItem){
	mFocusedListItem = newItem;
}

void ListBox::OnItemClicked(void* arg)
{
	WinBase* clickedWin = (WinBase*)arg;
	while (clickedWin && clickedWin->GetType() != ComponentType::ListItem){
		clickedWin = clickedWin->GetParent().get();
	}
	if (!clickedWin || clickedWin->GetType() != ComponentType::ListItem)
		return;
	ListItem* listItem = (ListItem*)clickedWin;
	
	size_t rowIndex = listItem->GetRowIndex();
	if (rowIndex != ListItem::INVALID_INDEX)
	{
		auto injector = InputManager::GetInstance().GetInputInjector();		
		unsigned clickedIndex = rowIndex;
		unsigned lastIndex = -1;
		if (!mSelectedIndices.empty())
			lastIndex = mSelectedIndices.back();

		if (injector->IsKeyDown(VK_SHIFT) && mMultiSelection){
			if (clickedIndex == lastIndex){
				DeselectRow(clickedIndex);
			}
			else if (lastIndex!=-1){
				if (clickedIndex < lastIndex){
					std::swap(clickedIndex, lastIndex);
				}
				for (unsigned index = lastIndex; index <= clickedIndex; ++index){
					SetHighlightRowAndSelect(index, true);
				}
			}
			else if (lastIndex == -1){
				ToggleSelection(clickedIndex);
			}
		}
		else if (injector->IsKeyDown(VK_CONTROL) && mMultiSelection){
			ToggleSelection(clickedIndex);
		}		
		else
		{
			DeselectAll();
			SetHighlightRowAndSelect(rowIndex, true);
		}
	}

	auto listItemPtr = std::static_pointer_cast<ListItem>(listItem->GetPtr());
	ChangeFocusItem(listItemPtr);
	if (listItem->GetNumChildren() > 0){
		UIManager::GetInstance().SetFocusUI(listItem->GetChild(0));
	}
	else{
		UIManager::GetInstance().SetFocusUI(listItemPtr);
	}
	

	OnEvent(UIEvents::EVENT_MOUSE_LEFT_CLICK);
	OnEvent(UIEvents::EVENT_LISTBOX_SELECTION_CHANGED);
}

void ListBox::OnItemRClicked(void* arg) {
	WinBase* clickedWin = (WinBase*)arg;
	while (clickedWin && clickedWin->GetType() != ComponentType::ListItem) {
		clickedWin = clickedWin->GetParent().get();
	}
	if (!clickedWin || clickedWin->GetType() != ComponentType::ListItem)
		return;
	ListItem* listItem = (ListItem*)clickedWin;

	size_t rowIndex = listItem->GetRowIndex();
	if (rowIndex != ListItem::INVALID_INDEX)
	{
		auto injector = InputManager::GetInstance().GetInputInjector();
		unsigned clickedIndex = rowIndex;
		unsigned lastIndex = -1;
		DeselectAll();
		SetHighlightRowAndSelect(rowIndex, true);
	}

	auto listItemPtr = std::static_pointer_cast<ListItem>(listItem->GetPtr());
	ChangeFocusItem(listItemPtr);
	if (listItem->GetNumChildren() > 0) {
		UIManager::GetInstance().SetFocusUI(listItem->GetChild(0));
	}
	else {
		UIManager::GetInstance().SetFocusUI(listItemPtr);
	}

	OnEvent(UIEvents::EVENT_MOUSE_RIGHT_CLICK);
	OnEvent(UIEvents::EVENT_LISTBOX_SELECTION_CHANGED);
}

void ListBox::OnItemDoubleClicked(void* arg)
{
	WinBase* clickedWin = (WinBase*)arg;
	while (clickedWin && clickedWin->GetType() != ComponentType::ListItem){
		clickedWin = clickedWin->GetParent().get();
	}
	if (!clickedWin || clickedWin->GetType() != ComponentType::ListItem)
		return;
	ListItemPtr listItem = std::static_pointer_cast<ListItem>(clickedWin->GetPtr());
	size_t rowIndex = listItem->GetRowIndex();
	if (rowIndex != ListItem::INVALID_INDEX)
	{
		DeselectAll();
		SetHighlightRowAndSelect(rowIndex, true);
	}
	ChangeFocusItem(listItem);
	OnEvent(UIEvents::EVENT_MOUSE_LEFT_DOUBLE_CLICK);
	OnEvent(UIEvents::EVENT_LISTBOX_SELECTION_CHANGED);
}

void ListBox::OnItemEnter(void* arg){
	auto focusedItem = mFocusedListItem.lock();
	if (focusedItem && focusedItem->GetChild(0).get() == arg)
	{
		unsigned rowIndex = focusedItem->GetRowIndex();
		unsigned colIndex = focusedItem->GetColIndex();
		auto data = mData->GetData(rowIndex);
		if (data[colIndex].GetDataType() == ListItemDataType::TextField)
		{
			auto textfield = std::static_pointer_cast<TextField>(focusedItem->GetChild(0));
			if (wcscmp(textfield->GetText(), data[colIndex].GetText()) != 0){
				data[colIndex].SetText(textfield->GetText());
				OnEvent(UIEvents::EVENT_ENTER);
			}
		}
	}
}

void ListBox::OnNumericChanged(void* arg){
	NumericUpDown* numeric = (NumericUpDown*)(arg);
	assert(numeric->GetParent()->GetType() == ComponentType::ListItem);
	auto listItem = std::static_pointer_cast<ListItem>(numeric->GetParent());
	mData->SetData(listItem->GetRowCol(), numeric->GetValue());
	mLastChangedItem = listItem->GetRowCol();
	OnEvent(UIEvents::EVENT_NUMERIC_DOWN);
}

void ListBox::OnDragHeader(void* arg){
	ListItem* rawItem = (ListItem*)arg;
	assert(rawItem->GetType() == ComponentType::ListItem);
	ListItemPtr item = std::static_pointer_cast<ListItem>(rawItem->GetPtr());
	auto col = item->GetColIndex();
	auto injector = InputManager::GetInstance().GetInputInjector();	
	auto mouseMove = Vec2I(injector->GetDpiDependentDeltaXY()).x;
	if (mouseMove > 0){
		if (col != mNumCols - 1){ // not last
			float fMove = mouseMove / (float)GetParentSize().x;
			mColSizes[col] += fMove;
			mColSizesInt[col] += mouseMove;
			mColSizes[col+1] -= fMove;
			mColSizesInt[col+1] -= mouseMove;
			UpdateColSizes();
		}
		else{
			float fMove = mouseMove / (float)GetParentSize().x;
			mColSizes[col] += fMove;
			mColSizesInt[col] += mouseMove;
			mColSizes[col - 1] -= fMove;
			mColSizesInt[col-1] -= mouseMove;
			UpdateColSizes();
		}
	}
	else if (mouseMove < 0){
		if (col != mNumCols - 1){ // not last
			float fMove = mouseMove / (float)GetParentSize().x;
			mColSizes[col] += fMove;
			mColSizesInt[col] += mouseMove;
			mColSizes[col + 1] -= fMove;
			mColSizesInt[col+1] -= mouseMove;
			UpdateColSizes();
		}
		else{
			float fMove = mouseMove / (float)GetParentSize().x;
			mColSizes[col] += fMove;
			mColSizesInt[col] += mouseMove;
			mColSizes[col - 1] -= fMove;
			mColSizesInt[col-1] -= mouseMove;

			UpdateColSizes();
		}
	}
}

unsigned ListBox::GetNumRows()
{
	return mItems.size();
}

WinBasePtr ListBox::MakeMergedRow(unsigned row)
{
	if (mItems.size() < row)
		return 0;

	if (mItems[row].empty())
		return 0;

	NoVirtualizingItem(row);
	auto firstItem = mItems[row][0].lock();
	if (!firstItem)
		return nullptr;
	assert(firstItem);
	firstItem->ChangeNSizeX(1.0f);
	if (mScrollerV.lock())
		firstItem->ModifySize(Vec2I(-4, 0));
	for (unsigned i = 1; i < mNumCols; ++i){
		mItems[row][i].lock()->ChangeNPosX(1.f);
	}

	firstItem->SetMerged(true);
	return firstItem;
}

WinBasePtr ListBox::MakeMergedRow(unsigned row, const char* backColor, const char* textColor, bool noMouseEvent)
{
	if (mItems.size() < row)
		return 0;

	if (mItems[row].empty())
		return 0;

	NoVirtualizingItem(row);
	auto firstItem = mItems[row][0].lock();
	assert(firstItem);
	firstItem->SetNSizeX(1.0f);
	firstItem->ModifySize(Vec2I(-4, 0));
	for (unsigned i = 1; i < mNumCols; ++i){
		mItems[row][i].lock()->ChangeNPosX(1.f);
	}
	
	if (backColor && strlen(backColor) != 0)
	{
		firstItem->SetProperty(UIProperty::BACK_COLOR, backColor);
		firstItem->SetBackColor(backColor);
		firstItem->SetProperty(UIProperty::NO_BACKGROUND, "false");
		firstItem->SetNoBackground(false);
	}
		
	if (textColor && strlen(textColor) !=0)
		firstItem->SetProperty(UIProperty::TEXT_COLOR, textColor);

	if (noMouseEvent)
		firstItem->SetProperty(UIProperty::NO_MOUSE_EVENT, "true");
	firstItem->SetProperty(UIProperty::TEXT_ALIGN, "center");

	firstItem->SetMerged(true);
	return firstItem;
}

void ListBox::SwapItems(unsigned index0, unsigned index1)
{
	if (!mData)
		return;
	mData->SwapData(index0, index1);
	VisualizeData(index0);
	VisualizeData(index1);

	auto cols = mItems[index0].size();
	if_assert_fail(cols == mItems[index1].size())
		return;

	bool index0selected = ValueExistsInVector(mSelectedIndices, index0);
	bool index1selected = ValueExistsInVector(mSelectedIndices, index1);
	if (index0selected && !index1selected)
	{
		DeleteValuesInVector(mSelectedIndices, index0);
		mSelectedIndices.push_back(index1);
	}
	else if (!index0selected && index1selected)
	{
		DeleteValuesInVector(mSelectedIndices, index1);
		mSelectedIndices.push_back(index0);
	}
}

void ListBox::SwapItems(const wchar_t* uniqueKey0, const wchar_t* uniqueKey1){
	unsigned index0 = mData->FindRowIndexWithKey(uniqueKey0);
	unsigned index1 = mData->FindRowIndexWithKey(uniqueKey1);
	SwapItems(index0, index1);
}

void ListBox::GetSelectedUniqueIdsString(std::vector<std::string>& ids) const{
	if (!mData)
		return;

	for (unsigned index : mSelectedIndices){
		ids.push_back(WideToAnsi(mData->GetStringKey(index)));
	}
}
void ListBox::GetSelectedUniqueIdsUnsigned(std::vector<unsigned>& ids) const{
	if (!mData)
		return;

	for (unsigned index : mSelectedIndices){
		ids.push_back(mData->GetUnsignedKey(index));
	}
}


void ListBox::VisualizeData(unsigned index){
	if (!GetVisible() || !mData || mItems.empty())
		return;

	if (index >= mItems.size()) {		
		return;
	}

	bool noVirtualizing = mNoVirtualizingRows.find(index)!= mNoVirtualizingRows.end();
	if (index < mStartIndex || index > mEndIndex)
	{
		if (!noVirtualizing){
			if (!mItems[index][0].expired())
			{
				MoveToRecycle(index);
			}
			return;
		}
	}

	int hgap = mRowHeight + mRowGap;
	auto scrollerV = mScrollerV.lock();
	auto contentUI = mWndContentUI.lock();
	if (!scrollerV && contentUI){
		scrollerV = contentUI->GetScrollerV();
	}
	Vec2 offset(0, 0);
	if (scrollerV)
	{
		offset = scrollerV->GetOffset();
	}

	const auto data = mData->GetData(index);
	if (!data)
	{
		if (!noVirtualizing)
			MoveToRecycle(index);
		return;
	}
	
	if (!mItems[index][0].expired())
	{
		FillItem(index);		
	}
	else
	{
		if (!mRecycleBin.empty())
		{
			auto items = mRecycleBin.back();
			int y = hgap * index + mRowGap;
			for (unsigned c = 0; c < mNumCols; ++c){
				items[c]->SetPosY(y);
				items[c]->SetWNScrollingOffset(offset);
				items[c]->SetRowIndex(index);				
				AddChildSimple(items[c]);
				mItems[index][c] = items[c];
			}
			mRecycleBin.pop_back();
			FillItem(index);
			UIManager::GetInstance().DirtyRenderList(mHwndId);
		}
		else
		{
			int y = hgap * index + mRowGap;
			for (unsigned c = 0; c < mNumCols; ++c){
				auto item = CreateNewItem(index, c);
				if (c == 0)
				{
					item->SetProperty(UIProperty::TEXT_LEFT_GAP, "5");
				}
				item->SetWNScrollingOffset(offset);
				mItems[index][c] = item;				
			}
			FillItem(index);
		}
	}
}

void ListBox::FillItem(unsigned index){
	if (!mData)
		return;

	const auto data = mData->GetData(index);
	if_assert_fail (data)
		return;

	for (unsigned i = 0; i < mNumCols; ++i){
		auto item = mItems[index][i].lock();
		assert(item);
		item->SetRowIndex(index);
		switch (data[i].GetDataType())
		{
		case ListItemDataType::CheckBox:
		{
			auto checkbox = std::dynamic_pointer_cast<CheckBox>(item->GetChild(0));
			if (!checkbox)
			{
				item->RemoveAllChildren();
				checkbox = std::static_pointer_cast<CheckBox>(item->AddChild(0.f, 0.f, 1.f, 1.f, ComponentType::CheckBox));
				checkbox->SetUseAbsSize(false);
				checkbox->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
					std::bind(&ListBox::OnItemClicked, this, std::placeholders::_1));
				checkbox->RegisterEventFunc(UIEvents::EVENT_MOUSE_RIGHT_CLICK,
					std::bind(&ListBox::OnItemRClicked, this, std::placeholders::_1));
				checkbox->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_DOUBLE_CLICK,
					std::bind(&ListBox::OnItemDoubleClicked, this, std::placeholders::_1));
				checkbox->SetRuntimeChild(true);
				checkbox->SetRuntimeChildRecursive(true);
				checkbox->SetVisible(mVisibility.IsVisible());
				checkbox->SetProperty(UIProperty::ALIGNH, mColAlignes[i].c_str());
				if (checkbox->GetHAlign() == ALIGNH::CENTER){
					checkbox->ChangeNPosX(0.5f);
					checkbox->SetUseAbsXPos(false);
				}
				else if (checkbox->GetHAlign() == ALIGNH::RIGHT){
					checkbox->ChangeNPosX(1.f);
					checkbox->SetUseAbsXPos(false);
				}
			}
			if_assert_pass(checkbox){
				checkbox->SetCheck(data[i].GetChecked());
			}
			break;
		}
		case ListItemDataType::NumericUpDown:
		{
			auto numeric = std::dynamic_pointer_cast<NumericUpDown>(item->GetChild(0));
			if (!numeric){
				item->RemoveAllChildren();
				numeric = std::static_pointer_cast<NumericUpDown>(item->AddChild(0.f, 0.f, 0.7f, 1.f, ComponentType::NumericUpDown));
				numeric->SetUseAbsSize(false);
				numeric->RegisterEventFunc(UIEvents::EVENT_NUMERIC_UP,
					std::bind(&ListBox::OnNumericChanged, this, std::placeholders::_1));
				numeric->RegisterEventFunc(UIEvents::EVENT_NUMERIC_DOWN,
					std::bind(&ListBox::OnNumericChanged, this, std::placeholders::_1));
				numeric->SetRuntimeChild(true);
				numeric->SetRuntimeChildRecursive(true);
				numeric->SetVisible(mVisibility.IsVisible());
				numeric->SetProperty(UIProperty::ALIGNH, mColAlignes[i].c_str());
				if (numeric->GetHAlign() == ALIGNH::CENTER){
					numeric->ChangeNPosX(0.5f);
					numeric->SetUseAbsXPos(false);
				}
				else if (numeric->GetHAlign() == ALIGNH::RIGHT){
					numeric->ChangeNPosX(1.f);
					numeric->SetUseAbsXPos(false);
				}
			}
			if_assert_pass(numeric){
				numeric->SetNumber(data[i].GetInt());
			}
			break;
		}
		case ListItemDataType::HorizontalGauge:
		{
			auto gauge = std::dynamic_pointer_cast<HorizontalGauge>(item->GetChild(0));
			if (!gauge){
				item->RemoveAllChildren();
				gauge = std::static_pointer_cast<HorizontalGauge>(item->AddChild(0.f, 0.f, 0.8f, 0.8f, ComponentType::HorizontalGauge));
				gauge->SetUseAbsSize(false);				
				gauge->SetRuntimeChild(true);
				gauge->SetRuntimeChildRecursive(true);
				gauge->SetVisible(mVisibility.IsVisible());
				gauge->SetProperty(UIProperty::ALIGNH, mColAlignes[i].c_str());
				if (gauge->GetHAlign() == ALIGNH::CENTER){
					gauge->ChangeNPosX(0.5f);
					gauge->SetUseAbsXPos(false);
				}
				else if (gauge->GetHAlign() == ALIGNH::RIGHT){
					gauge->ChangeNPosX(1.f);
					gauge->SetUseAbsXPos(false);
				}
				gauge->SetProperty(UIProperty::GAUGE_MAX, "1.0");
				gauge->SetProperty(UIProperty::GAUGE_COLOR, "0, 1, 1, 1");
			}
			if_assert_pass(gauge){				
				gauge->SetPercentage(data[i].GetFloat());
			}
			break;
		}
		case ListItemDataType::String:
		{
			item->SetText(data[i].GetText());
			break;
		}
		case ListItemDataType::TextField:
		{
			auto textField = std::dynamic_pointer_cast<TextField>(item->GetChild(0));
			if (!textField)
			{
				item->RemoveAllChildren();
				textField = std::static_pointer_cast<TextField>(item->AddChild(0.f, 0.f, 1.f, 1.f, ComponentType::TextField));
				textField->SetProperty(UIProperty::USE_BORDER, "true");
				textField->SetProperty(UIProperty::TEXT_LEFT_GAP, "4");
				textField->SetRuntimeChild(true);
				textField->SetRuntimeChildRecursive(true);
				textField->SetVisible(mVisibility.IsVisible());
				textField->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
					std::bind(&ListBox::OnItemClicked, this, std::placeholders::_1));
				textField->RegisterEventFunc(UIEvents::EVENT_MOUSE_RIGHT_CLICK,
					std::bind(&ListBox::OnItemRClicked, this, std::placeholders::_1));
				textField->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_DOUBLE_CLICK,
					std::bind(&ListBox::OnItemDoubleClicked, this, std::placeholders::_1));
				textField->RegisterEventFunc(UIEvents::EVENT_ENTER,
					std::bind(&ListBox::OnItemEnter, this, std::placeholders::_1));
			}
			if_assert_pass(textField){
				textField->SetText(data[i].GetText());
			}
			break;
		}
		case ListItemDataType::TexturePath:
		{
			auto imageBox = std::dynamic_pointer_cast<ImageBox>(item->GetChild(0));
			if (!imageBox)
			{
				item->RemoveAllChildren();
				imageBox = std::static_pointer_cast<ImageBox>(item->AddChild(0.f, 0.f, 1.f, 1.f, ComponentType::ImageBox));
				imageBox->MatchUISizeToImageAtCenter();
				imageBox->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
					std::bind(&ListBox::OnItemClicked, this, std::placeholders::_1));
				imageBox->RegisterEventFunc(UIEvents::EVENT_MOUSE_RIGHT_CLICK,
					std::bind(&ListBox::OnItemRClicked, this, std::placeholders::_1));
				imageBox->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_DOUBLE_CLICK,
					std::bind(&ListBox::OnItemDoubleClicked, this, std::placeholders::_1));
				imageBox->SetRuntimeChild(true);
				imageBox->SetRuntimeChildRecursive(true);
				imageBox->SetVisible(mVisibility.IsVisible());
			}
			if_assert_pass(imageBox){
				imageBox->SetTexture(WideToAnsi(data[i].GetText()));
			}
			break;
		}
		case ListItemDataType::TextureRegion:
		{
			auto imageBox = std::dynamic_pointer_cast<ImageBox>(item->GetChild(0));
			if (!imageBox)
			{
				item->RemoveAllChildren();
				imageBox = std::static_pointer_cast<ImageBox>(item->AddChild(0.f, 0.f, 1.f, 1.f, ComponentType::ImageBox));
				imageBox->MatchUISizeToImageAtCenter();
				imageBox->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
					std::bind(&ListBox::OnItemClicked, this, std::placeholders::_1));
				imageBox->RegisterEventFunc(UIEvents::EVENT_MOUSE_RIGHT_CLICK,
					std::bind(&ListBox::OnItemRClicked, this, std::placeholders::_1));
				imageBox->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_DOUBLE_CLICK,
					std::bind(&ListBox::OnItemDoubleClicked, this, std::placeholders::_1));
				imageBox->SetRuntimeChild(true);
				imageBox->SetRuntimeChildRecursive(true);
				imageBox->SetVisible(mVisibility.IsVisible());
			}
			if_assert_pass(imageBox){
				imageBox->SetTextureAtlasRegion(mTextureAtlas.c_str(), WideToAnsi(data[i].GetText()));
			}
			break;
		}
		case ListItemDataType::Texture:
		{
			auto imageBox = std::dynamic_pointer_cast<ImageBox>(item->GetChild(0));
			if (!imageBox)
			{
				item->RemoveAllChildren();
				imageBox = std::static_pointer_cast<ImageBox>(item->AddChild(0.f, 0.f, 1.f, 1.f, ComponentType::ImageBox));
				imageBox->MatchUISizeToImageAtCenter();
				imageBox->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
					std::bind(&ListBox::OnItemClicked, this, std::placeholders::_1));
				imageBox->RegisterEventFunc(UIEvents::EVENT_MOUSE_RIGHT_CLICK,
					std::bind(&ListBox::OnItemRClicked, this, std::placeholders::_1));
				imageBox->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_DOUBLE_CLICK,
					std::bind(&ListBox::OnItemDoubleClicked, this, std::placeholders::_1));
				imageBox->SetRuntimeChild(true);
				imageBox->SetRuntimeChildRecursive(true);
				imageBox->SetVisible(mVisibility.IsVisible());
			}
			if_assert_pass(imageBox){
				imageBox->SetTexture(data[i].GetTexture());
			}
			break;
		}
		}

		auto it = mHighlighted.find(Vec2I((int)index, (int)i));
		if (it != mHighlighted.end() && it->second){
			SetHighlightRowCol(index, i, true);
		}
		else{
			SetHighlightRowCol(index, i, false);
		}
	}

	unsigned int key = mData->GetUnsignedKey(index);
	auto it = mItemPropertyByUnsigned.find(key);
	if (it != mItemPropertyByUnsigned.end()){
		for (auto& property : it->second){
			for (auto it : mItems[index]){
				auto col = it.lock();
				assert(col);
				col->SetProperty(property.first, property.second.c_str());
			}
		}
	}
	else{
		for (auto it : mItems[index]){
			auto col = it.lock();
			assert(col);
			SetDefaultProperty(col);			
		}
	}

	{
		auto key = mData->GetStringKey(index);
		if (wcslen(key) != 0) {
			auto it = mItemPropertyByString.find(key);
			if (it != mItemPropertyByString.end()){
				for (auto& property : it->second){
					for (auto it : mItems[index]){
						auto col = it.lock();
						assert(col);
						col->SetProperty(property.first, property.second.c_str());
					}
				}
			}
		}
	}

	for (unsigned i = 0; i < mNumCols; ++i){
		auto item = mItems[index][i].lock();
		assert(item);
		const auto& prop = mItemPropertyByColumn[i];
		for (auto p : prop){
			item->SetProperty(p.first, p.second.c_str());
		}
	}

	for (auto it = mItemPropertyKeyCol.begin(); it != mItemPropertyKeyCol.end(); it++){
		if (key == (unsigned)it->first.x){
			auto item = mItems[index][it->first.y].lock();
			assert(item);
			for (auto p : it->second){
				item->SetProperty(p.first, p.second.c_str());
			}
		}
	}

	auto itDisabled = mDisabledItemKeys.find(key);
	if (itDisabled != mDisabledItemKeys.end()){
		for (auto it : mItems[index]){
			auto col = it.lock();
			assert(col);
			col->SetEnable(false, false);
		}
	}
	else{
		for (auto it : mItems[index]){
			auto col = it.lock();
			assert(col);
			col->SetEnable(true, false);
		}
	}
}

void ListBox::SetDefaultProperty(ListItemPtr listItem){
	if (!listItem){
		Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg");
	}
	listItem->SetProperty(UIProperty::TEXT_COLOR, 
		StringMathConverter::ToString(UIProperty::GetDefaultValueVec4(UIProperty::TEXT_COLOR)).c_str());

}

void ListBox::MoveToRecycle(unsigned row){
	if (row < mItems.size())
	{
		if (!mItems[row][0].expired())
		{
			SetHighlightRow(row, false);
			for (unsigned c = 0; c < mNumCols; ++c){
				if (mItems[row][c].lock()->IsKeyboardFocused())
					return;
			}			
			
			mRecycleBin.push_back(ROW());
			auto& cols = mRecycleBin.back();			
			cols.reserve(mNumCols);
			for (unsigned c = 0; c < mNumCols; ++c){
				auto item = mItems[row][c].lock();
				cols.push_back(item);
				RemoveChild(item);
				mItems[row][c].reset();
			}

			UIManager::GetInstance().DirtyRenderList(mHwndId);
		}
	}
}

ListBoxData* ListBox::GetData(unsigned rowIndex, unsigned colIndex) const{
	if (!mData)
		return 0;
	if (rowIndex >= mData->Size() || colIndex >= mNumCols)
		return 0;

	auto cols = mData->GetData(rowIndex);
	return &cols[colIndex];

}

unsigned ListBox::FindRowIndex(unsigned uniqueKey) const{
	if (!mData)
		return -1;
	return mData->FindRowIndexWithKey(uniqueKey);
}
unsigned ListBox::FindRowIndex(wchar_t* uniqueKey) const{
	if (!mData)
		return -1;
	return mData->FindRowIndexWithKey(uniqueKey);
}

unsigned ListBox::GetUnsignedKey(unsigned rowIndex) const{
	if (mData == 0)
		return -1;
	return mData->GetUnsignedKey(rowIndex);
}
const wchar_t* ListBox::GetStringKey(unsigned rowIndex) const{
	if (mData == 0)
		return L"";

	return mData->GetStringKey(rowIndex);
}

void ListBox::OnSizeChanged()
{
	if (mNSize.x == NotDefined || mNSize.y == NotDefined)
		return;
	__super::OnSizeChanged();

	mColSizesInt.clear();
	auto parentX = GetFinalSize().x;
	for (auto nsize : mColSizes) {
		mColSizesInt.push_back(Round(nsize * parentX));
	}

	UpdateColSizes();	
	Scrolled();
}

Vec2 ListBox::Scrolled()
{
	unsigned prevStart = mStartIndex;
	unsigned prevEnd = mEndIndex;
	int hgap = mRowHeight + mRowGap;
	auto scrollerV = mScrollerV.lock();
	unsigned sizeY = mSize.y;
	auto contentUI = mWndContentUI.lock();
	if (!scrollerV && contentUI){
		scrollerV = contentUI->GetScrollerV();
		sizeY = contentUI->GetSize().y;
	}
	
	if (scrollerV)
	{
		Vec2 offset = scrollerV->GetOffset();
		int scrolledLen = -Round(offset.y * GetRenderTargetSize().y) - mRowGap;
		int topToBottom = sizeY + scrolledLen - mRowGap;

		// decide visual index range		
		mStartIndex = scrolledLen / hgap;
		mEndIndex = topToBottom / hgap;
		int remain = topToBottom % hgap;
		if (remain > 0)
			mEndIndex += 1;
	}
	else
	{
		// decide visual index range
		int topToBottom = sizeY;
		mStartIndex = 0;
		mEndIndex = topToBottom / hgap;
		int remain = topToBottom % hgap;
		if (remain > 0)
			mEndIndex += 1;
	}

	// to recycle
	while (prevStart < mStartIndex)
	{
		unsigned index = prevStart++;
		MoveToRecycle(index);
	}

	while (prevEnd > mEndIndex)
	{
		unsigned index = prevEnd--;
		MoveToRecycle(index);
	}

	//to visual
	while (prevStart > mStartIndex)
	{
		unsigned visualIndex = --prevStart;
		VisualizeData(visualIndex);
	}

	while (prevEnd <= mEndIndex)
	{
		unsigned visualIndex = prevEnd++;
		VisualizeData(visualIndex);
	}

	return __super::Scrolled();
}

float ListBox::GetContentHeight() const
{
	if (!mData)
		return mRowHeight / (float)GetRenderTargetSize().y;
	unsigned length = mData->Size();
	unsigned hgap = mRowHeight + mRowGap;
	unsigned contentLength = hgap * length + mRowGap; // for upper gap
	return contentLength / (float)GetRenderTargetSize().y;
}

void ListBox::Sort()
{
	mData->Sort(0);

	if (mStartIndex != -1 && mEndIndex != -1) {
		for (unsigned i = mStartIndex; i < mEndIndex; i++) {
			VisualizeData(i);
		}
	}
}

void ListBox::SearchStartingChacrcter(char c, unsigned curIndex)
{
	unsigned index = mData->FindNext(0, c, curIndex);
	if (index == -1)
		return;
	
	auto scrollerV = mScrollerV.lock();
	auto contentUI = mWndContentUI.lock();
	if (!scrollerV && contentUI){
		scrollerV = contentUI->GetScrollerV();
	}

	if (scrollerV)
	{
		if (index < mStartIndex + 1 || index >(mEndIndex >= 3 ? mEndIndex - 3 : mEndIndex))
		{
			unsigned hgap = mRowHeight + mRowGap;
			unsigned destY = hgap * index + mRowGap;

			scrollerV->SetOffset(Vec2(0.f, -(destY / (float)GetRenderTargetSize().y)));
		}
	}
	auto firstItem = mItems[index][0].lock();
	if (firstItem)
	{
		DeselectAll();
		SetHighlightRowAndSelect(index, true);
		ChangeFocusItem(firstItem);
		UIManager::GetInstance().SetFocusUI(firstItem);
	}
	OnEvent(UIEvents::EVENT_LISTBOX_SELECTION_CHANGED);
}


void ListBox::IterateItem(bool next, bool apply)
{
	DeselectAll();

	unsigned rowIndex = 0;
	unsigned colIndex = 0;
	auto focusedItem = mFocusedListItem.lock();
	if (focusedItem){
		rowIndex = focusedItem->GetRowIndex();
		colIndex = focusedItem->GetColIndex();
		if (apply)
		{
			auto data = mData->GetData(rowIndex);
			if (data[colIndex].GetDataType() == ListItemDataType::TextField)
			{
				auto textfield = std::static_pointer_cast<TextField>(focusedItem->GetChild(0));
				if (wcscmp(textfield->GetText(), data[colIndex].GetText()) != 0){
					data[colIndex].SetText(textfield->GetText());
					OnEvent(UIEvents::EVENT_ENTER);
				}								
			}
			
		}
		SetHighlightRow(rowIndex, false);
		
		if (next)
			mData->FindNextFocus(rowIndex, colIndex);
		else
			mData->FindPrevFocus(rowIndex, colIndex);
	}
		
	MakeSureRangeFor(rowIndex);
	auto newFocusItem = mItems[rowIndex][colIndex].lock();
	if_assert_pass(newFocusItem)
	{
		ChangeFocusItem(newFocusItem);
		SetHighlightRowAndSelect(rowIndex, true);
		auto child = newFocusItem->GetChild(0);
		if (child)
		{			
			UIManager::GetInstance().SetFocusUI(child);
			if (child->GetType() == ComponentType::TextField)
			{
				UIManager::GetInstance().GetTextManipulator()->SelectAll();
			}
		}
		else
			UIManager::GetInstance().SetFocusUI(newFocusItem);
	}
	OnEvent(UIEvents::EVENT_LISTBOX_SELECTION_CHANGED);
		
}

void ListBox::MakeSureRangeFor(unsigned rowIndex){
	auto scrollerV = mScrollerV.lock();
	auto contentUI = mWndContentUI.lock();
	if (!scrollerV && contentUI){
		scrollerV = contentUI->GetScrollerV();
	}
	if (scrollerV)
	{
		if (rowIndex < mStartIndex + 1 ||
			(rowIndex >(mEndIndex >= 3 ? mEndIndex - 3 : mEndIndex))
			)
		{
			unsigned hgap = mRowHeight + mRowGap;
			unsigned destY = hgap * rowIndex + mRowGap;

			scrollerV->SetOffset(Vec2(0.f, -(destY / (float)GetRenderTargetSize().y)));
		}
	}
}

void ListBox::SetItemProperty(unsigned uniqueKey, UIProperty::Enum prop, const char* val){
	auto& properties = mItemPropertyByUnsigned[uniqueKey];
	bool found = false;
	for (auto it = properties.begin(); it != properties.end(); ++it){
		if (it->first == prop){
			found = true;
			it->second = val;
		}
	}
	if (!found){
		properties.push_back(std::make_pair(prop, val));
	}
	if (!mData)
		return;
	auto rowIndex = mData->FindRowIndexWithKey(uniqueKey);
	if (rowIndex == -1)
		return;
	VisualizeData(rowIndex);
}

void ListBox::SetItemProperty(const wchar_t* uniqueKey, UIProperty::Enum prop, const char* val){
	auto& properties = mItemPropertyByString[uniqueKey];
	bool found = false;
	for (auto it = properties.begin(); it != properties.end(); ++it){
		if (it->first == prop){
			found = true;
			it->second = val;
		}
	}
	if (!found){
		properties.push_back(std::make_pair(prop, val));
	}

	auto rowIndex = mData->FindRowIndexWithKey(uniqueKey);
	if (rowIndex == -1)
		return;
	VisualizeData(rowIndex);
}

void ListBox::SetItemPropertyCol(unsigned col, UIProperty::Enum prop, const char* val){
	auto& properties = mItemPropertyByColumn[col];
	bool found = false;
	for (auto& it : properties){
		if (it.first == prop){
			it.second = val;
			found = true;
		}
	}
	if (!found)
		properties.push_back(std::make_pair(prop, val));
	for (auto row : mItems){
		if (row.size() > col && !row[col].expired()){
			row[col].lock()->SetProperty(prop, val);
		}		
	}
}

bool ListBox::SetItemPropertyKeyCol(const Vec2I& keyCol, UIProperty::Enum prop, const char* val){
	auto& properties = mItemPropertyKeyCol[keyCol];
	bool found = false;
	for (auto& it : properties){
		if (it.first == prop){
			it.second = val;
			found = true;
		}
	}
	if (!found)
		properties.push_back(std::make_pair(prop, val));
	unsigned row = mData->FindRowIndexWithKey((unsigned)keyCol.x);
	if (mItems.size() > (unsigned)row && !mItems[row][keyCol.y].expired()){
		mItems[row][keyCol.y].lock()->SetProperty(prop, val);
	}
	// returns 'added?'
	return !found;
}

bool ListBox::RemoveItemPropertyKeyCol(const Vec2I& keyCol, UIProperty::Enum prop) {
	auto& properties = mItemPropertyKeyCol[keyCol];
	bool found = false;
	for (auto it = properties.begin(); it != properties.end(); /**/) {
		if (it->first == prop) {
			it = properties.erase(it);
			found = true;
		}
		else {
			++it;
		}
	}
	unsigned row = mData->FindRowIndexWithKey((unsigned)keyCol.x);
	if (mItems.size() > (unsigned)row && !mItems[row][keyCol.y].expired()) {
		SetDefaultPropertyForUI(mItems[row][keyCol.y].lock(), prop);		
	}
	return found;
}

bool ListBox::RemoveItemPropertiesForCol(int col, UIProperty::Enum prop) {
	bool found = false;
	for (auto keyColIt = mItemPropertyKeyCol.begin(); keyColIt != mItemPropertyKeyCol.end(); ++keyColIt) {
		if (keyColIt->first.y != col)
			continue;
		auto& properties = keyColIt->second;
		
		for (auto it = properties.begin(); it != properties.end(); /**/) {
			if (it->first == prop) {
				it = properties.erase(it);
				found = true;
			}
			else {
				++it;
			}
		}
		unsigned row = mData->FindRowIndexWithKey((unsigned)keyColIt->first.x);
		if (mItems.size() > (unsigned)row && !mItems[row][col].expired()) {
			SetDefaultPropertyForUI(mItems[row][col].lock(), prop);			
		}
	}
	return found;
}

void ListBox::NoVirtualizingItem(unsigned rowIndex){
	if (!mData){
		assert(0);
		return;
	}
	mNoVirtualizingRows.insert(rowIndex);
	VisualizeData(rowIndex);
}

void ListBox::UpdateColSizes(){	
	if (mColSizes.empty())
		return;
	float totalSize = 0.f;
	for (auto size : mColSizes)	{
		totalSize += size;
	}
	float gap = 1.0f - totalSize;
	mColSizes[0] += gap;
	mColSizesInt[0] += Round(gap * GetParentSize().x);
	mStrColSizes.clear();
	for (auto size : mColSizes){
		mStrColSizes += StringConverter::ToString(size);
		mStrColSizes += ',';
	}
	mStrColSizes.pop_back();

	unsigned colHeader = 0;
	int posHeader = 0;
	for (auto itemIt : mHeaders){
		if (colHeader >= mColSizes.size())
			break;
		auto item = itemIt.lock();
		assert(item);
		item->ChangePosX(posHeader);
		item->ChangeSizeX(mColSizesInt[colHeader]);
		posHeader += mColSizesInt[colHeader++];
	}
	for (auto row : mItems){
		unsigned col = 0;
		int pos = 0;
		for (auto itemIt : row){
			auto item = itemIt.lock();
			if (item) {
				if (item->GetMerged())
					break;
				if (col >= mColSizes.size())
					break;
				item->ChangePosX(pos);
				item->ChangeSizeX(mColSizesInt[col]);
				pos += mColSizesInt[col++];
			}
		}
	}

	for (auto row : mRecycleBin) {
		unsigned col = 0;
		int pos = 0;
		for (auto item : row) {			
			if (item) {
				if (item->GetMerged())
					break;
				if (col >= mColSizes.size())
					break;
				item->ChangePosX(pos);
				item->ChangeSizeX(mColSizesInt[col]);
				pos += mColSizesInt[col++];
			}
		}
	}
}

void ListBox::UpdateItemAlign(){
	for (auto row : mItems){
		unsigned col = 0;
		for (auto itemIt : row){
			auto item = itemIt.lock();
			if (!item)
				continue;

			if (item->GetMerged())
				break;
			item->SetProperty(UIProperty::TEXT_ALIGN, mColAlignes[col].c_str());
			col++;
			auto child = item->GetChild(0);
			if (child){
				child->SetProperty(UIProperty::ALIGNH, mColAlignes[col].c_str());
				if (child->GetHAlign() == ALIGNH::CENTER){
					child->ChangeNPosX(0.5f);
					child->SetUseAbsXPos(false);
				}
				else if (child->GetHAlign() == ALIGNH::RIGHT){
					child->ChangeNPosX(1.f);
					child->SetUseAbsXPos(false);
				}
				else{
					child->ChangePosX(0);
					child->SetUseAbsXPos(true);
				}
			}
		}
	}
}

void ListBox::ClearItemProperties(){
	for (auto items : mItems) {
		int col = 0;
		for (auto itemIt : items) {
			auto item = itemIt.lock();
			if (item) {
				const auto& prop = mItemPropertyByColumn[col];
				for (auto p : prop) {
					SetDefaultPropertyForUI(item, p.first);
				}
			}
			++col;
		}
	}
	mItemPropertyByColumn.clear();

	int row = 0;
	for (auto items : mItems) {		
		auto key = mData->GetUnsignedKey(row);
		auto it = mItemPropertyByUnsigned.find(key);
		if (it != mItemPropertyByUnsigned.end()) {
			for (auto& property : it->second) {
				for (auto itemIt : items) {
					auto item = itemIt.lock();
					if (item)
						SetDefaultPropertyForUI(item, property.first);
				}
			}
		}
		++row;
	}	
	mItemPropertyByUnsigned.clear();


	row = 0;
	for (auto items : mItems) {
		auto key = mData->GetStringKey(row);
		auto it = mItemPropertyByString.find(key);
		if (it != mItemPropertyByString.end()) {
			for (auto& property : it->second) {
				for (auto itemIt : items) {
					auto item = itemIt.lock();
					if (item)
						SetDefaultPropertyForUI(item, property.first);
				}
			}
		}
		++row;
	}
	mItemPropertyByString.clear();


	for (auto it : mItemPropertyKeyCol) {
		auto key = mData->FindRowIndexWithKey(it.first.x);
		if (key != -1) {
			auto item = mItems[key][it.first.y].lock();
			if (item) {
				for (auto prop : it.second) {
					SetDefaultPropertyForUI(item, prop.first);
				}
			}
		}
	}
	mItemPropertyKeyCol.clear();
	mRecycleBin.clear();
}

void ListBox::DisableItemEvent(unsigned uniqueKey){
	mDisabledItemKeys.insert(uniqueKey);
	auto rowIndex = mData->FindRowIndexWithKey(uniqueKey);
	if (rowIndex == -1)
		return;
	assert(rowIndex < mItems.size());
	auto& row = mItems[rowIndex];
	for (auto& col : row){
		if (!col.expired()){
			col.lock()->SetEnable(false);
		}
	}
}

void ListBox::EnableItemEvent(unsigned uniqueKey){
	auto it = mDisabledItemKeys.find(uniqueKey);
	if (it != mDisabledItemKeys.end()){
		mDisabledItemKeys.erase(it);
	}
	auto rowIndex = mData->FindRowIndexWithKey(uniqueKey);
	if (rowIndex == -1)
		return;
	assert(rowIndex < mItems.size());
	auto& row = mItems[rowIndex];
	for (auto& col : row){
		if (!col.expired()){
			col.lock()->SetEnable(true);
		}
	}
}

float ListBox::GetChildrenContentEnd() const{
	auto num = mItems.size();
	int hgap = mRowHeight + mRowGap;

	int sizeY = hgap * num;
	if (!mHeaders.empty())
		sizeY += mRowGap + mRowHeight;
	if (num > 0)
		hgap -= mRowGap;
	return mWNPos.y + sizeY / (float)GetRenderTargetSize().y;
}

void ListBox::RemoveAllChildren(bool immediately/* = false*/){
	__super::RemoveAllChildren(immediately);
	Clear(immediately);

}

void ListBox::MoveUpListBoxItems(const std::vector<unsigned>& ids) {
	if (mData->Size() == 0)
		return;
	for (auto id : ids) {
		auto index = mData->FindRowIndexWithKey(id);
		bool moved = mData->MoveUpData(id);
		if (moved) {
			auto it = std::find(mSelectedIndices.begin(), mSelectedIndices.end(), index);
			if (it != mSelectedIndices.end()) {
				DeselectRow(index);
				SelectRow(index - 1);
			}
		}
	}
	RefreshVisual();
}

void ListBox::MoveDownListBoxItems(const std::vector<unsigned>& ids) {
	if (mData->Size() == 0)
		return;
	auto reversed = ids;
	std::reverse(reversed.begin(), reversed.end());
	for (auto id : reversed) {
		auto index = mData->FindRowIndexWithKey(id);
		auto moved = mData->MoveDownData(id);
		if (moved) {
			auto it = std::find(mSelectedIndices.begin(), mSelectedIndices.end(), index);
			if (it != mSelectedIndices.end()) {
				DeselectRow(index);
				SelectRow(index + 1);
			}
		}
	}
	RefreshVisual();
}

void ListBox::RemoveDataWithKeys(const std::vector<unsigned>& ids) {
	for (auto id : ids) {
		mData->DelDataWithKey(id);
	}
	RefreshVisual();
}

void ListBox::RemoveDataWithRowIndex(size_t rowIndex) {
	mData->DelDataWithIndex(rowIndex);
	RefreshVisual();
}

void ListBox::RefreshVisual() {
	// remove deleted items
	int dataSize = mData->Size();
	int itemSize = mItems.size();
	if (itemSize > dataSize) {
		auto deleteStart = dataSize;
		for (int i = itemSize - 1; i >= dataSize; --i) {
			bool noVirtualizing = mNoVirtualizingRows.find(i) != mNoVirtualizingRows.end();
			if (!noVirtualizing) {
				if (!mItems[i][0].expired())
				{
					MoveToRecycle(i);
				}
				else {
					auto columnSize = mItems[i].size();
					for (unsigned c = 0; c < columnSize; ++c) {
						auto item = mItems[i][c].lock();						
						RemoveChild(item);
						mItems[i][c].reset();
					}					
				}
			}
		}
	}	

	// refresh
	int startIndex = mStartIndex - 1;
	startIndex = std::max(0, startIndex);
	int endIndex = mEndIndex - 1;
	endIndex = std::min((int)mData->Size(), endIndex);
	for (int i = startIndex; i <= endIndex; ++i)
		VisualizeData(i);
}

void ListBox::OnParentSizeChanged() {
	__super::OnParentSizeChanged();
	mColSizesInt.clear();
	auto parentX = GetFinalSize().x;
	for (auto nsize : mColSizes) {
		mColSizesInt.push_back(Round(nsize * parentX));
	}
	UpdateColSizes();
}

bool ListBox::GetHand() const {
	return mHand;
}

bool ListBox::GetHighlighOnHover() const {
	return mHighlightOnHover;
}

void ListBox::SetVisibleInternal(bool visible) {
	__super::SetVisibleInternal(visible);
	if (visible) {
		for (unsigned i = mStartIndex; i <= mEndIndex; ++i) {
			VisualizeData(i);
		}
	}		
}
}