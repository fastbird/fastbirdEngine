#pragma once

#include <UI/Wnd.h>
#include <UI/ListBoxDataSet.h>
#include <UI/ListItemDataType.h>

namespace fastbird
{
class IUIObject;
class CheckBox;
class ListItem;

class ITexture;
//--------------------------------------------------------------------------------
class ListBox : public Wnd
{
protected:
	ListItem* mFocusedListItem;
	unsigned mStartIndex;
	unsigned mEndIndex;

	ListBoxDataSet* mData;
	typedef std::vector<ListItem*> ROW;
	std::vector< ROW > mRecycleBin;	
	ROW mHeaders;
	std::vector<ROW> mItems;
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

public:
	ListBox();
	virtual ~ListBox();
	// IWinBase
	virtual ComponentType::Enum GetType() const { return ComponentType::ListBox; }
	virtual void GatherVisit(std::vector<IUIObject*>& v);
	virtual void Scrolled();
	virtual float GetContentHeight() const;

protected:
	virtual void OnSizeChanged();

public:
	// Own	
	virtual unsigned InsertItem(unsigned uniqueKey);
	virtual unsigned InsertItem(const wchar_t* uniqueKey);

	virtual void SetItem(const Vec2I& rowcol, const wchar_t* string, ListItemDataType::Enum type);
	virtual void SetItem(const Vec2I& rowcol, bool checked);

	virtual bool GetCheckBox(const Vec2I& indexRowCol) const;

	virtual void RemoveRow(const wchar_t* key);
	virtual void RemoveRow(unsigned uniqueKey);
	virtual void RemoveRowWithIndex(unsigned index);
	
	virtual std::string GetSelectedString();
	virtual const SelectedRows& GetSelectedRows() const { return mSelectedIndices; }
	virtual void GetSelectedUniqueIdsString(std::vector<std::string>& ids) const;
	virtual void GetSelectedUniqueIdsUnsigned(std::vector<unsigned>& ids) const;

	void OnItemClicked(void* arg);
	void OnItemDoubleClicked(void* arg);
	void OnItemEnter(void* arg);

	void ChangeFocusItem(ListItem* newItem);

	size_t GetNumItems() const;

	unsigned GetNumCols() const { return mNumCols; }

	virtual bool SetProperty(UIProperty::Enum prop, const char* val);
	virtual bool GetProperty(UIProperty::Enum prop, char val[], bool notDefaultOnly);

	virtual ListItem* GetItem(const Vec2I& indexRowCol) const;

	virtual void SetRowHeight(int rowHeight) { mRowHeight = rowHeight; }

	virtual void SelectRow(unsigned index);
	virtual void DeselectRow(unsigned index);
	virtual void DeselectAll();
	virtual void ToggleSelection(unsigned index);

	virtual bool OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard);
	virtual void ClearSelection();
	virtual bool IsSelected(unsigned row);
	virtual unsigned GetNumRows();
	virtual IWinBase* MakeMergedRow(unsigned row);
	virtual IWinBase* MakeMergedRow(unsigned row, const char* backColor, const char* textColor, bool noMouseEvent);
	virtual void SwapItems(unsigned index0, unsigned index1);
	virtual void SwapItems(const wchar_t* uniqueKey0, const wchar_t* uniqueKey1);
	virtual unsigned FindRowWithKey(unsigned uniqueKey);

	virtual void VisualizeData(unsigned index);
	void FillItem(unsigned index);
	void MoveToRecycle(unsigned row);

	ListBoxData* GetData(unsigned rowIndex, unsigned colIndex) const;
	void Sort();

	

	virtual void Clear();

	void SearchStartingChacrcter(char c, unsigned curIndex);
	void IterateItem(bool next, bool apply);
	void MakeSureRangeFor(unsigned rowIndex);

protected:

	ListItem* CreateNewItem(int row, int col);
	void SetHighlightRow(size_t row, bool highlight);
	void SetHighlightRowCol(unsigned row, unsigned col, bool highlight);
	void SetHighlightRowAndSelect(size_t row, bool highlight);



	
};

}