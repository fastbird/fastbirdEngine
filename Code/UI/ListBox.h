#pragma once

#include <UI/Wnd.h>

namespace fastbird
{
class IUIObject;
class Scroller;
class CheckBox;
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
	CheckBox* ListItem::GetCheckBox() const;

	void SetBackColor(const char* backColor);
	void SetNoBackground(bool noBackground);
	const char* GetBackColor() const {return mBackColor.c_str();}
	bool GetNoBackground() const { return mNoBackground; }

protected:
	const static float LEFT_GAP;
	virtual void OnPosChanged();
	virtual void OnSizeChanged();

	size_t mRowIndex;
	size_t mColIndex;

	// backup values
	std::string mBackColor;
	bool mNoBackground;
};

class ITexture;
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
	virtual unsigned InsertItem(const wchar_t* szString);
	virtual unsigned InsertItem(ITexture* texture);
	virtual unsigned InsertCheckBoxItem(bool check);
	virtual CheckBox* GetCheckBox(unsigned row, unsigned col) const;
	virtual void RemoveItem(size_t index);
	virtual void SetItemString(size_t row, size_t col, const wchar_t* szString);
	virtual void SetItemTexture(size_t row, size_t col, ITexture* texture);
	virtual void SetItemTexture(size_t row, size_t col, const char* texturePath);
	virtual void SetTextureAtlas(const char* atlas);
	virtual void SetItemTextureRegion(size_t row, size_t col, const char* region);
	virtual IWinBase* SetItemIconText(size_t row, size_t col, const char* region, const char* txt, unsigned iconSize);
	virtual std::string GetSelectedString();
	typedef std::vector<size_t> SelectedRows;
	virtual const SelectedRows& GetSelectedRows() const { return mSelectedRows; }
	virtual void GetSelectedRowIds(std::vector<unsigned>& ids) const;
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
	virtual unsigned GetNumRows();
	virtual IWinBase* MakeMergedRow(unsigned row);
	virtual IWinBase* MakeMergedRow(unsigned row, const char* backColor, const char* textColor, bool noMouseEvent);
	virtual void SetRowId(unsigned row, unsigned id);
	virtual void SwapItems(unsigned row0, unsigned row1);
	virtual unsigned GetRowId(unsigned row);
	virtual unsigned FindRowWithId(unsigned id);
	virtual void DeleteRow(unsigned row);

protected:

	ListItem* CreateNewItem(int row, int col, const Vec2& npos, const Vec2& nsize);
	void SetHighlightRow(size_t row, bool highlight);
	void SetHighlightRowAndSelect(size_t row, bool highlight);

protected:

	typedef std::vector<ListItem*> ROW;
	ROW mHeaders;
	std::vector<ROW> mItems;
	std::vector<unsigned> mRowIds;
	SelectedRows mSelectedRows;
	size_t mCurSelectedCol;
	unsigned mNumCols;
	std::vector < float > mColSizes;
	std::vector< std::string > mColAlignes;
	std::vector< std::string> mHeaderTextSize;
	std::vector < std::string > mTextSizes;
	int mRowHeight;
	int mRowGap;
	std::string mHighlightColor;
	std::string mTextureAtlas;

	
};

}