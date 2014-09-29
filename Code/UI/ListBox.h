#pragma once

#include <UI/Wnd.h>

namespace fastbird
{
class IUIObject;
class Scroller;
class ListItem : public WinBase
{
public:
	ListItem();
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

	static const int ROW_HEIGHT = 24;
	// IWinBase
	virtual ComponentType::Enum GetType() const { return ComponentType::ListBox; }
	virtual void GatherVisit(std::vector<IUIObject*>& v);

	// Own
	virtual int InsertItem(const wchar_t* szString);
	virtual void RemoveItem(size_t index);
	virtual void SetItemString(size_t row, size_t col, const wchar_t* szString);
	virtual std::string GetSelectedString();
	virtual size_t GetSelectedRow();
	void OnItemClicked(void* arg);
	void OnItemDoubleClicked(void* arg);
	virtual void Clear();
	size_t GetNumItems() const { return mItems.size();}
	virtual bool SetProperty(UIProperty::Enum prop, const char* val);

	virtual ListItem* GetItem(size_t row, size_t col) const;

private:

	typedef std::vector<ListItem*> ROW;
	ROW mHeaders;
	std::vector<ROW> mItems;
	float mNextHeight;
	size_t mCurSelectedRow;
	size_t mCurSelectedCol;
	unsigned mNumCols;
	std::vector < float > mColSizes;

	
};

}