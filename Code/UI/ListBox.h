#pragma once

#include <UI/Wnd.h>

namespace fastbird
{
class IUIObject;
class Scroller;
class ListItem : public Wnd
{
public:
	ListItem();
	virtual ~ListItem();
	static const size_t INVALID_INDEX;

	// IWinBase
	virtual ComponentType::Enum GetType() const { return ComponentType::ListItem; }
	virtual void GatherVisit(std::vector<IUIObject*>& v);

	// Own
	void SetRowIndex(size_t index) { mRowIndex = index; }
	size_t GetRowIndex() const { return mRowIndex; }

	void SetColIndex(size_t index) { mColIndex = index; }
	size_t GetColIndex() const { return mColIndex; }

protected:
	const static float LEFT_GAP;
	virtual void OnPosChanged();
	virtual void OnSizeChanged();

	size_t mRowIndex;
	size_t mColIndex;
};

//--------------------------------------------------------------------------------
class ListBox : public Wnd
{
public:
	ListBox();
	virtual ~ListBox();
	// IWinBase
	virtual ComponentType::Enum GetType() const { return ComponentType::ListBox; }
	virtual void GatherVisit(std::vector<IUIObject*>& v);

	// Own
	virtual int InsertItem(const wchar_t* szString);
	virtual int ListBox::InsertItem(ITexture* texture);
	virtual void RemoveItem(size_t index);
	virtual void SetItemString(size_t row, size_t col, const wchar_t* szString);
	virtual void SetItemTexture(size_t row, size_t col, ITexture* texture);
	virtual void SetItemTexture(size_t row, size_t col, const char* texturePath);
	virtual std::string GetSelectedString();
	typedef std::vector<size_t> SelectedRows;
	virtual const SelectedRows& GetSelectedRows() const { return mSelectedRows; }
	void OnItemClicked(void* arg);
	void OnItemDoubleClicked(void* arg);
	virtual void Clear();
	size_t GetNumItems() const { return mItems.size();}
	unsigned GetNumCols() const { return mNumCols; }
	virtual bool SetProperty(UIProperty::Enum prop, const char* val);

	virtual ListItem* GetItem(size_t row, size_t col) const;
	virtual void SetRowHeight(int rowHeight) { mRowHeight = rowHeight; }
	virtual void SelectRow(unsigned row);
	virtual bool OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard);
	virtual void ClearSelection();
	virtual bool IsSelected(unsigned row);

private:

	ListItem* CreateNewItem(int row, int col, const Vec2& npos, const Vec2& nsize);
	void SetHighlightRow(size_t row, bool highlight);
	void SetHighlightRowAndSelect(size_t row, bool highlight);

private:

	typedef std::vector<ListItem*> ROW;
	ROW mHeaders;
	std::vector<ROW> mItems;
	SelectedRows mSelectedRows;
	size_t mCurSelectedCol;
	unsigned mNumCols;
	std::vector < float > mColSizes;
	std::vector< std::string > mColAlignes;
	std::vector < std::string > mTextSizes;
	int mRowHeight;
	int mRowGap;

	
};

}