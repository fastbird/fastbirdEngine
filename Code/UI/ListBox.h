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
	void SetIndex(size_t index) { mIndex = index; }
	size_t GetIndex() const { return mIndex; }

protected:
	const static float LEFT_GAP;
	virtual void OnPosChanged();
	virtual void OnSizeChanged();

	size_t mIndex;
};

class ListBox : public Wnd
{
public:
	ListBox();

	// IWinBase
	virtual ComponentType::Enum GetType() const { return ComponentType::ListBox; }
	virtual void GatherVisit(std::vector<IUIObject*>& v);
	virtual void Scrolled();

	// Own
	void InsertItem(const wchar_t* szString);
	std::string GetSelectedString();
	void OnItemClicked(void* arg);
	void OnItemDoubleClicked(void* arg);
	void Clear();
	size_t GetNumItems() const { return mItems.size();}

private:

	std::vector<ListItem*> mItems;
	float mNextHeight;
	Scroller* mScroller;
	size_t mCurSelected;

	
};

}