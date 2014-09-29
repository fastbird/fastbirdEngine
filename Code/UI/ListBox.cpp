#include <UI/StdAfx.h>
#include <UI/ListBox.h>
#include <UI/Scroller.h>
#include <CommonLib/StringUtils.h>

namespace fastbird
{
//-----------------------------------------------------------------------------
const float ListItem::LEFT_GAP = 0.001f;
const size_t ListItem::INVALID_INDEX = -1;

ListItem::ListItem()
: mRowIndex(INVALID_INDEX)
, mColIndex(INVALID_INDEX)

{
	mUIObject = IUIObject::CreateUIObject(false);
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
}

void ListItem::GatherVisit(std::vector<IUIObject*>& v)
{
	v.push_back(mUIObject);	
}

void ListItem::OnPosChanged()
{
	__super::OnPosChanged();
	//mUIObject->SetTextStartNPos(Vec2(mWNPos.x, mWNPos.y + mWNSize.y - GetTextBottomGap()));
}

void ListItem::OnSizeChanged()
{
	__super::OnSizeChanged();
	//mUIObject->SetTextStartNPos(Vec2(mWNPos.x, mWNPos.y + mWNSize.y - GetTextBottomGap()));
}

//-----------------------------------------------------------------------------
ListBox::ListBox()
	: mNextHeight(0.01f)
	, mCurSelectedRow(ListItem::INVALID_INDEX)
	, mCurSelectedCol(ListItem::INVALID_INDEX)
	, mNumCols(1)
{
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	mColSizes.push_back(1.0f);
	mUseScrollerV = true;
}

void ListBox::GatherVisit(std::vector<IUIObject*>& v)
{
	__super::GatherVisit(v);
}

std::string ListBox::GetSelectedString()
{
	if (mCurSelectedRow != ListItem::INVALID_INDEX && mCurSelectedCol != ListItem::INVALID_INDEX)
	{
		if (mItems[mCurSelectedRow][mCurSelectedCol]->GetText())
			return WideToAnsi(mItems[mCurSelectedRow][mCurSelectedCol]->GetText());
	}
	return std::string();
}

size_t ListBox::GetSelectedRow()
{
	return mCurSelectedRow;
}

int ListBox::InsertItem(const wchar_t* szString)
{
	float nRowSize = PixelToLocalNHeight(ROW_HEIGHT);
	
	mItems.push_back(ROW());
	ROW& row = mItems.back();
	row.push_back(static_cast<ListItem*>(
		AddChild(0.00f, mNextHeight, mColSizes[0], nRowSize, ComponentType::ListItem)));
	ListItem* pAddedItem = row.back();
	const RECT& rect = mUIObject->GetRegion();
	pAddedItem->SetScissorRect(true, rect);
	pAddedItem->SetProperty(UIProperty::BACK_COLOR, "0.4, 0.4, 0.3, 0.7");
	pAddedItem->SetProperty(UIProperty::NO_BACKGROUND, "true");
	pAddedItem->SetRowIndex(mItems.size()-1);
	pAddedItem->SetColIndex(0);
	pAddedItem->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&ListBox::OnItemClicked, this, std::placeholders::_1));
	pAddedItem->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_DOUBLE_CLICK,
		std::bind(&ListBox::OnItemDoubleClicked, this, std::placeholders::_1));
	pAddedItem->SetText(szString);
	
	
	mNextHeight += PixelToLocalNHeight(ROW_HEIGHT+4);

	return mItems.size() - 1;
}

void ListBox::RemoveItem(size_t index)
{
	assert(index < mItems.size());
	float nh = PixelToLocalNHeight(ROW_HEIGHT + 4);
	for (size_t row = index+1; row < mItems.size(); ++row)
	{
		for (size_t col = 0; col < mItems[row].size(); ++col)
		{
			Vec2 npos = mItems[row][col]->GetNPos();
			npos.y -= nh;
			mItems[row][col]->SetNPos(npos);
			mItems[row][col]->SetRowIndex(row - 1);
		}
	}
	for (size_t col = 0; col < mItems[index].size(); ++col)
	{
		RemoveChild(mItems[index][col]);
	}
	mItems.erase(mItems.begin() + index);
	if (mCurSelectedRow >= mItems.size())
		mCurSelectedRow = -1;
	else
	{
		for (size_t i = 0; i < mNumCols; ++i)
		{
			mItems[mCurSelectedRow][i]->SetProperty(UIProperty::NO_BACKGROUND, "false");
		}
	}

	mNextHeight -= PixelToLocalNHeight(ROW_HEIGHT + 4);
}

void ListBox::SetItemString(size_t row, size_t col, const wchar_t* szString)
{
	if (row >= mItems.size())
	{
		assert(0);
		return;
	}
	if (col >= mNumCols)
	{
		assert(0);
		return;
	}

	assert(szString);
	float nRowSize = PixelToLocalNHeight(ROW_HEIGHT);
	ROW& r = mItems[row];
	float posy = r.back()->GetNPos().y;
	while (r.size() <= col)
	{
		int prevColIndex = r.size() - 1;
		int addingColIndex = r.size();
		float posx = 0.0f;
		if (prevColIndex >= 0)
		{
			posx = r[prevColIndex]->GetNPos().x + r[prevColIndex]->GetNSize().x;
		}

		r.push_back(static_cast<ListItem*>(
			AddChild(posx, posy, mColSizes[addingColIndex], nRowSize, ComponentType::ListItem)));
		ListItem* pAddedItem = r.back();
		const RECT& rect = mUIObject->GetRegion();
		pAddedItem->SetScissorRect(true, rect);
		pAddedItem->SetProperty(UIProperty::BACK_COLOR, "0.4, 0.4, 0.3, 0.7");
		pAddedItem->SetProperty(UIProperty::NO_BACKGROUND, "true");
		pAddedItem->SetRowIndex(row);
		pAddedItem->SetColIndex(addingColIndex);
		pAddedItem->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK,
			std::bind(&ListBox::OnItemClicked, this, std::placeholders::_1));
		pAddedItem->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_DOUBLE_CLICK,
			std::bind(&ListBox::OnItemDoubleClicked, this, std::placeholders::_1));
	}

	mItems[row][col]->SetText(szString);
}

void ListBox::OnItemClicked(void* arg)
{
	ListItem* pItem = (ListItem*)arg;
	size_t rowindex = pItem->GetRowIndex();
	size_t colindex = pItem->GetColIndex();
	if (rowindex != ListItem::INVALID_INDEX)
	{
		if (mCurSelectedRow != ListItem::INVALID_INDEX)
		{
			for (size_t i = 0; i < mNumCols; ++i)
			{
				mItems[mCurSelectedRow][i]->SetProperty(UIProperty::NO_BACKGROUND, "true");
			}
		}
		for (size_t i = 0; i < mNumCols; ++i)
		{
			mItems[rowindex][i]->SetProperty(UIProperty::NO_BACKGROUND, "false");
		}
		mCurSelectedRow = rowindex;
		mCurSelectedCol = colindex;
	}
	OnEvent(IEventHandler::EVENT_MOUSE_LEFT_CLICK);
}

void ListBox::OnItemDoubleClicked(void* arg)
{
	ListItem* pItem = (ListItem*)arg;
	size_t rowindex = pItem->GetRowIndex();
	size_t colindex = pItem->GetColIndex();
	if (rowindex!=ListItem::INVALID_INDEX && colindex!=ListItem::INVALID_INDEX)
	{
		if (mCurSelectedRow != ListItem::INVALID_INDEX)
		{
			for (size_t i = 0; i < mNumCols; ++i)
			{
				mItems[mCurSelectedRow][i]->SetProperty(UIProperty::NO_BACKGROUND, "true");
			}
		}
		for (size_t i = 0; i < mNumCols; ++i)
		{
			mItems[rowindex][i]->SetProperty(UIProperty::NO_BACKGROUND, "false");
		}
		mCurSelectedRow = rowindex;
		mCurSelectedCol = colindex;
	}
	OnEvent(IEventHandler::EVENT_MOUSE_LEFT_DOUBLE_CLICK);
}

void ListBox::Clear()
{
	for each(auto items in mItems)
	{
		for each (auto item	 in items)
		{
			RemoveChild(item);
		}
	}
	mItems.clear();
	mNextHeight = 0.01f;
	mCurSelectedRow = ListItem::INVALID_INDEX;
	mCurSelectedCol = ListItem::INVALID_INDEX;
}

bool ListBox::SetProperty(UIProperty::Enum prop, const char* val)
{
	if (prop == UIProperty::LISTBOX_COL)
	{
		mNumCols = StringConverter::parseUnsignedInt(val);
		return true;
	}

	if (prop == UIProperty::LISTBOX_COL_SIZES)
	{
		// set UIProperty::LISTBOX_COL first
		// don't need to set this property if the num of col is 1.
		assert(mNumCols != 1);
		mColSizes.clear();
		StringVector strs = Split(val);
		assert(!strs.empty());
		for (unsigned i = 0; i < strs.size(); ++i)
		{
			mColSizes.push_back(StringConverter::parseReal(strs[i]));
		}
		return true;
	}

	if (prop == UIProperty::LISTBOX_COL_HEADERS)
	{
		StringVector strs = Split(val, ",");
		assert(strs.size() == mNumCols);
		float nRowSize = PixelToLocalNHeight(ROW_HEIGHT);

		for (unsigned i = 0; i < mNumCols; ++i)
		{
			float posx = 0.0f;
			if (i >= 1)
			{
				posx = mHeaders[i - 1]->GetNPos().x + mHeaders[i - 1]->GetNSize().x;
			}

			mHeaders.push_back(static_cast<ListItem*>(
				AddChild(posx, 0.0f, mColSizes[i], nRowSize, ComponentType::ListItem)));
			ListItem* pAddedItem = mHeaders.back();
			const RECT& rect = mUIObject->GetRegion();
			pAddedItem->SetScissorRect(true, rect);
			pAddedItem->SetProperty(UIProperty::BACK_COLOR, "0.2, 0.2, 0.2, 0.7");
			pAddedItem->SetProperty(UIProperty::TEXT_ALIGN, "center");
			pAddedItem->SetRowIndex(-1);
			pAddedItem->SetColIndex(i);
			pAddedItem->SetText(AnsiToWide(strs[i].c_str()));
		}
		mWndContentUI = (Wnd*)AddChild(0.0f, 0.0f, 1.0f, 1.0f, ComponentType::Window);
		Vec2I sizeMod = { 0, -(ROW_HEIGHT + 4) };
		mWndContentUI->SetSizeModificator(sizeMod);
		mWndContentUI->SetUseAbsYSize(true);
		mWndContentUI->SetPos(Vec2I(0, (ROW_HEIGHT + 4)));
		mWndContentUI->SetProperty(UIProperty::NO_BACKGROUND, "true");
		if (mUseScrollerV)
		{
			if (mScrollerV)
				mPendingDelete.push_back(mScrollerV);
			mUseScrollerV = false;
			mWndContentUI->SetProperty(UIProperty::SCROLLERV, "true");
		}
		return true;
	}

	return __super::SetProperty(prop, val);
}

ListItem* ListBox::GetItem(size_t row, size_t col) const
{
	assert(row < mItems.size());
	assert(col < mItems[row].size());
	return mItems[row][col];

}

}