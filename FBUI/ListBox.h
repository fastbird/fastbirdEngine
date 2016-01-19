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

#pragma once

#include "Wnd.h"
#include "ListBoxDataSet.h"
#include "ListItemDataType.h"

namespace fb
{
class IUIObject;
class CheckBox;
FB_DECLARE_SMART_PTR(ListItem);
FB_DECLARE_SMART_PTR(ListBox);
//--------------------------------------------------------------------------------
class FB_DLL_UI ListBox : public Wnd
{
protected:
	ListItemWeakPtr mFocusedListItem;
	unsigned mStartIndex;
	unsigned mEndIndex;

	ListBoxDataSet* mData;
	typedef std::vector<ListItemPtr> ROW;
	typedef std::vector<ListItemWeakPtr> ROW_WEAK;
	std::vector< ROW > mRecycleBin;	
	ROW_WEAK mHeaders;
	std::vector<ROW_WEAK> mItems;
	typedef std::vector<size_t> SelectedRows;
	SelectedRows mSelectedIndices;
	unsigned mNumCols;

	std::vector < float > mColSizes;
	std::vector< std::string > mColAlignes;
	std::vector< std::string> mHeaderTextSize;
	std::vector < std::string > mTextSizes;
	int mRowHeight;
	int mRowGap;
	std::string mHighlightColor;
	std::string mTextureAtlas;

	std::string mStrCols;
	std::string mStrColSizes;
	std::string mStrTextSizes;
	std::string mStrColAlignH;
	std::string mStrHeaderTextSizes;
	std::string mStrHeaders;
	VectorMap<Vec2I, bool> mHighlighted;

	typedef std::vector<std::pair<UIProperty::Enum, std::string>> PropertyData;
	VectorMap<unsigned, PropertyData > mItemPropertyByUnsigned;
	VectorMap<std::wstring, PropertyData> mItemPropertyByString;
	VectorMap<unsigned, PropertyData> mItemPropertyByColumn;
	VectorMap<Vec2I, PropertyData> mItemPropertyKeyCol;
	std::set<unsigned> mNoVirtualizingRows;
	std::set<unsigned> mDisabledItemKeys;
	bool mMultiSelection;
	bool mNoHighlight;
	bool mNoSearch;
	bool mHand;

	Vec2I mLastChangedItem; // for numeric updown

	ListBox();
	~ListBox();

public:
	static ListBoxPtr Create();

	// IWinBase
	ComponentType::Enum GetType() const { return ComponentType::ListBox; }
	void Scrolled();
	float GetContentHeight() const;

protected:
	void OnSizeChanged();

public:
	// Own	
	unsigned InsertItem(unsigned uniqueKey);
	unsigned InsertItem(const wchar_t* uniqueKey);
	unsigned InsertEmptyData();
	bool ModifyKey(unsigned row, unsigned key);

	void SetItem(const Vec2I& rowcol, const wchar_t* string, ListItemDataType::Enum type);
	void SetItem(const Vec2I& rowcol, bool checked);
	void SetItem(const Vec2I& rowcol, TexturePtr texture);
	void SetItem(const Vec2I& rowcol, int number); // numeric updown
	void SetItem(const Vec2I& rowcol, float number); // numeric updown

	bool GetCheckBox(const Vec2I& indexRowCol) const;

	bool RemoveRow(const wchar_t* key);
	bool RemoveRow(unsigned uniqueKey);
	bool RemoveRowWithIndex(unsigned index);
	
	std::string GetSelectedString();
	const SelectedRows& GetSelectedRows() const { return mSelectedIndices; }
	void GetSelectedUniqueIdsString(std::vector<std::string>& ids) const;
	void GetSelectedUniqueIdsUnsigned(std::vector<unsigned>& ids) const;

	void OnItemClicked(void* arg);
	void OnItemDoubleClicked(void* arg);
	void OnItemEnter(void* arg);
	void OnNumericChanged(void* arg);
	void OnDragHeader(void* arg);

	void ChangeFocusItem(ListItemPtr newItem);

	size_t GetNumData() const;

	unsigned GetNumCols() const { return mNumCols; }

	bool SetProperty(UIProperty::Enum prop, const char* val);
	bool GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly);

	ListItemPtr GetItem(const Vec2I& indexRowCol) const;
	ListBoxData* GetData(unsigned rowIndex, unsigned colIndex) const;
	unsigned FindRowIndex(unsigned uniqueKey) const;
	unsigned FindRowIndex(wchar_t* uniqueKey) const;

	unsigned GetUnsignedKey(unsigned rowIndex) const;
	const wchar_t* GetStringKey(unsigned rowIndex) const;

	void SetRowHeight(int rowHeight) { mRowHeight = rowHeight; }

	void SelectRow(unsigned index);
	void DeselectRow(unsigned index);
	void DeselectAll();
	void ToggleSelection(unsigned index);

	bool OnInputFromHandler(IInputInjectorPtr injector);
	void ClearSelection();
	bool IsSelected(unsigned row);
	unsigned GetNumRows();
	WinBasePtr MakeMergedRow(unsigned row);
	WinBasePtr MakeMergedRow(unsigned row, const char* backColor, const char* textColor, bool noMouseEvent);
	void SwapItems(unsigned index0, unsigned index1);
	void SwapItems(const wchar_t* uniqueKey0, const wchar_t* uniqueKey1);

	void SetItemProperty(unsigned uniqueKey, UIProperty::Enum prop, const char* val);
	void SetItemProperty(const wchar_t* uniqueKey, UIProperty::Enum prop, const char* val);
	void SetItemPropertyCol(unsigned col, UIProperty::Enum prop, const char* val);
	void SetItemPropertyKeyCol(const Vec2I& keycol, UIProperty::Enum prop, const char* val);
	void ClearItemProperties();
	void DisableItemEvent(unsigned uniqueKey);
	void EnableItemEvent(unsigned uniqueKey);

	void VisualizeData(unsigned index);
	void FillItem(unsigned index);
	void MoveToRecycle(unsigned row);
	void Sort();

	

	void Clear(bool immediately=false);

	void SearchStartingChacrcter(char c, unsigned curIndex);
	void IterateItem(bool next, bool apply);
	void MakeSureRangeFor(unsigned rowIndex);

	void NoVirtualizingItem(unsigned rowIndex);

	void UpdateColSizes();
	void UpdateItemAlign();

	unsigned GetLastChangedRow() const { return mLastChangedItem.x; }

	float GetChildrenContentEnd() const;

	void RemoveAllChildren(bool immediately = false);

protected:

	ListItemPtr CreateNewItem(int row, int col);
	void SetHighlightRow(size_t row, bool highlight);
	void SetHighlightRowCol(unsigned row, unsigned col, bool highlight);
	void SetHighlightRowAndSelect(size_t row, bool highlight);
	void SetDefaultProperty(ListItemPtr listItem);



	
};

}