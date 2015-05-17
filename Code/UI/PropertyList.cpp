#include <UI/StdAfx.h>
#include <UI/PropertyList.h>
#include <UI/Scroller.h>
#include <UI/IUIManager.h>
#include <UI/TextField.h>
#include <Engine/TextManipulator.h>

namespace fastbird
{
PropertyList::PropertyList()
	: ListBox()
	, mFocusRow(-1)
	, mStartIndex(0)
	, mEndIndex(10)
{
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	mRowHeight = 22;
	mNumCols = 2;
}
PropertyList::~PropertyList()
{
	for (auto& item : mRecycleBin)
	{
		gFBEnv->pUIManager->DeleteComponent(item.first);
		gFBEnv->pUIManager->DeleteComponent(item.second);
	}
	mRecycleBin.clear();
}

void PropertyList::OnCreated()
{
	/*auto left = AddChild(0.0f, 0.0f, 0.5f, 1.0f, ComponentType::Window);
	left->SetRuntimeChild(true);
	left->SetName("__Key");
	left->SetProperty(UIProperty::MATCH_HEIGHT, "true");

	auto right = AddChild(0.5f, 0.0f, 0.5f, 1.0f, ComponentType::Window);
	right->SetRuntimeChild(true);
	right->SetName("__Value");
	right->SetProperty(UIProperty::MATCH_HEIGHT, "true");
	right->SetSizeModificator(Vec2I(-4, 0));*/

	SetProperty(UIProperty::SCROLLERV, "true");
}

void PropertyList::OnSizeChanged()
{
	__super::OnSizeChanged();
	Scrolled();
}

const wchar_t* PropertyList::GetValue(const wchar_t* key)
{
	return mData.GetData(key).c_str();
}

unsigned PropertyList::InsertItem(const wchar_t* key, const wchar_t* value)
{
	unsigned index = mData.AddData(key, value);
	while (mItems.size() <= index)
	{
		mItems.push_back(ROW());
		mItems.back().push_back(0);
		mItems.back().push_back(0);
	}
	GetRowId(index);
	VisualizeData(index);
	return index;
}
ListItem* PropertyList::CreateNewKeyItem(int row, int col, float ny)
{
	float nh = PixelToLocalNHeight(mRowHeight);
	ListItem* item = (ListItem*)AddChild(0.f, ny, 0.4f, nh, ComponentType::ListItem);
	item->SetRuntimeChild(true);
	
	if (col < (int)mColAlignes.size())
		item->SetProperty(UIProperty::TEXT_ALIGN, mColAlignes[col].c_str());
	
	if (col < (int)mTextSizes.size())
		item->SetProperty(UIProperty::TEXT_SIZE, mTextSizes[col].c_str());

	item->SetProperty(UIProperty::NO_BACKGROUND, "true");
	item->SetProperty(UIProperty::BACK_COLOR, "0.1, 0.3, 0.3, 0.7");
	item->SetVisible(mVisibility.IsVisible());
	item->SetRowIndex(row);
	item->SetColIndex(col);
	item->SetProperty(UIProperty::TEXT_LEFT_GAP, "5");
	return item;
}

ListItem* PropertyList::CreateNewValueItem(int row, int col, float ny)
{
	float nh = PixelToLocalNHeight(mRowHeight);
	ListItem* item = (ListItem*)AddChild(0.41f, ny, 0.57f, nh, ComponentType::ListItem);
	item->SetRuntimeChild(true);

	if (col < (int)mColAlignes.size())
		item->SetProperty(UIProperty::TEXT_ALIGN, mColAlignes[col].c_str());

	if (col < (int)mTextSizes.size())
		item->SetProperty(UIProperty::TEXT_SIZE, mTextSizes[col].c_str());

	item->SetVisible(mVisibility.IsVisible());
	item->SetRowIndex(row);
	item->SetColIndex(col);
	auto textField = item->AddChild(0.f, 0.f, 1.f, 1.f, ComponentType::TextField);
	textField->SetRuntimeChild(true);
	textField->SetProperty(UIProperty::TEXT_LEFT_GAP, "5");
	textField->SetProperty(UIProperty::USE_BORDER, "true");
	textField->SetVisible(mVisibility.IsVisible());
	return item;
}

unsigned PropertyList::ModifyItem(const wchar_t* key, const wchar_t* value)
{
	if (!key || !value)
		return -1;
	
	unsigned index = mData.AddData(key, value);
	VisualizeData(index);
	return index;
}
void PropertyList::RemoveItem(const wchar_t* key)
{
	unsigned deletedIndex = mData.DelData(key);
	if (deletedIndex != -1)
	{
		VisualizeData(deletedIndex);
	}
}

void PropertyList::ClearItems()
{
	mData.Clear();
	for (auto& item : mItems)
	{
		RemoveChild(item[0]);
		RemoveChild(item[1]);
	}
	mItems.clear();
	mRowIds.clear();
	for (auto& item : mRecycleBin)
	{
		gFBEnv->pUIManager->DeleteComponent(item.first);
		gFBEnv->pUIManager->DeleteComponent(item.second);
	}
	mRecycleBin.clear();
}

bool PropertyList::GetCurKeyValue(std::string& key, std::string& value)
{
	if (mFocusRow == -1)
		return false;
	auto pdata = mData.GetData(mFocusRow);
	if (pdata)
	{
		if (mItems[mFocusRow][1])
		{
			pdata->second = mItems[mFocusRow][1]->GetChild((unsigned)0)->GetText();
		}
		key = WideToAnsi(pdata->first.c_str());
		value = WideToAnsi(pdata->second.c_str());
		return true;
	}
	return false;
}

void PropertyList::Scrolled()
{
	unsigned prevStart = mStartIndex;
	unsigned prevEnd = mEndIndex;
	int hgap = mRowHeight + mRowGap;

	if (mScrollerV)
	{
		Vec2 offset = mScrollerV->GetOffset();		
		int scrolledLen = -Round(offset.y * GetRenderTargetSize().y) - mRowGap;
		int topToBottom = mSize.y + scrolledLen - mRowGap;

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
		int topToBottom = mSize.y;
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
		--prevStart;
		unsigned visualIndex = prevStart;
		VisualizeData(visualIndex);
	}

	while (prevEnd < mEndIndex)
	{
		++prevEnd;
		unsigned visualIndex = prevEnd;
		VisualizeData(visualIndex);
	}

	__super::Scrolled();
}

void PropertyList::VisualizeData(unsigned index)
{
	if (index < mStartIndex || index > mEndIndex)
	{
		if (mItems[index][0] && mItems[index][1])
		{
			MoveToRecycle(index);
		}
		return;
	}

	int hgap = mRowHeight + mRowGap;
	Vec2 offset(0, 0);
	if (mScrollerV)
	{
		offset = mScrollerV->GetOffset();
	}

	ListItem *keyItem = 0, *valueItem = 0;
	const auto pData = mData.GetData(index);
	if (!pData)
	{
		MoveToRecycle(index);
		return;
	}

	auto data = *pData;
	if (mItems[index][0] && mItems[index][1])
	{
		keyItem = mItems[index][0];
		valueItem = mItems[index][1];
		keyItem->SetRowIndex(index);
		valueItem->SetRowIndex(index);
		keyItem->SetText(data.first.c_str());
		valueItem->GetChild((unsigned)0)->SetText(data.second.c_str());
	}
	else
	{

		if (!mRecycleBin.empty())
		{
			auto it = mRecycleBin.back();
			int y = hgap * index + mRowGap;
			keyItem = it.first;
			valueItem = it.second;

			keyItem->SetText(data.first.c_str());
			keyItem->SetPosY(y);
			keyItem->SetWNPosOffset(offset);
			keyItem->SetRowIndex(index);

			valueItem->GetChild((unsigned)0)->SetText(data.second.c_str());
			valueItem->SetPosY(y);
			valueItem->SetWNPosOffset(offset);
			valueItem->SetRowIndex(index);

			mChildren.push_back(keyItem);
			mChildren.push_back(valueItem);
			mRecycleBin.pop_back();
			mItems[index][0] = keyItem;
			mItems[index][1] = valueItem;
		}
		else
		{
			int y = hgap * index + mRowGap;
			float ny = PixelToLocalNHeight(y);
			keyItem = CreateNewKeyItem(index, 0, ny);
			keyItem->SetText(data.first.c_str());
			keyItem->SetWNPosOffset(offset);

			valueItem = CreateNewValueItem(index, 1, ny);
			valueItem->GetChild((unsigned)0)->SetText(data.second.c_str());
			valueItem->SetWNPosOffset(offset);

			mItems[index][0] = keyItem;
			mItems[index][1] = valueItem;
		}
	}

	if (index == mFocusRow)
	{
		keyItem->SetProperty(UIProperty::NO_BACKGROUND, "false");
	}
	else
	{
		keyItem->SetProperty(UIProperty::NO_BACKGROUND, "true");
	}
}


void PropertyList::MoveToRecycle(unsigned row)
{
	if (row < mItems.size())
	{
		if (mItems[row][0] && mItems[row][1])
		{
			mRecycleBin.push_back(std::make_pair(mItems[row][0], mItems[row][1]));
			RemoveChildNotDelete(mItems[row][0]);
			RemoveChildNotDelete(mItems[row][1]);
			mItems[row][0] = 0;
			mItems[row][1] = 0;
			gFBUIManager->DirtyRenderList(mHwndId);
		}		
	}
}


float PropertyList::GetContentHeight() const
{
	unsigned length = mData.Size();
	unsigned hgap = mRowHeight + mRowGap;
	unsigned contentLength = hgap * length + mRowGap; // for upper gap
	return contentLength / (float)GetRenderTargetSize().y;
}

void PropertyList::Sort()
{
	mData.Sort();
	if (mStartIndex != -1 && mEndIndex != -1) {
		for (unsigned i = mStartIndex; i <= mEndIndex; i++) {
			VisualizeData(i);
		}
	}
}

void PropertyList::GoToNext(char c, unsigned curIndex)
{
	unsigned index = mData.FindNext(c, curIndex);
	if (index == -1)
		return;

	if (mScrollerV)
	{
		if (index < mStartIndex || index > (mEndIndex>=3 ? mEndIndex-3: mEndIndex))
		{
			unsigned hgap = mRowHeight + mRowGap;
			unsigned destY = hgap * index + mRowGap;

			mScrollerV->SetOffset(Vec2(0.f, -(destY / (float)GetRenderTargetSize().y)));
		}
	}
	if (mItems[index][0])
	{
		gFBUIManager->SetFocusUI(mItems[index][0]);
	}
}

void PropertyList::MoveFocusToEdit(unsigned index)
{
	assert(mFocusRow == index);
		//scroll
	if (mScrollerV)
	{
		if (index < mStartIndex || index >(mEndIndex>=3 ? mEndIndex - 3 : mEndIndex))
		{
			unsigned hgap = mRowHeight + mRowGap;
			unsigned destY = hgap * index + mRowGap;

			mScrollerV->SetOffset(Vec2(0.f, -(destY / (float)GetRenderTargetSize().y)));
		}
	}

	if (mItems[index][1])
	{
		gFBUIManager->SetFocusUI(mItems[index][1]->GetChild((unsigned)0),

			std::bind(&TextField::SelectAllAfterGetFocused,
			(TextField*)mItems[mFocusRow][1]->GetChild((unsigned)0))
			);
	}
}

void PropertyList::MoveToNextLine()
{
	// apply value;
	OnEvent(EVENT_ENTER);

	++mFocusRow;
	if (mFocusRow >= mData.Size())
	{
		mFocusRow = 0;
	}
	//scroll
	if (mScrollerV)
	{
		if (mFocusRow < mStartIndex || 
			(mFocusRow > (mEndIndex>=3 ? mEndIndex - 3 : mEndIndex))
			)
		{
			unsigned hgap = mRowHeight + mRowGap;
			unsigned destY = hgap * mFocusRow + mRowGap;

				mScrollerV->SetOffset(Vec2(0.f, -(destY / (float)GetRenderTargetSize().y)));
		}
	}

	if (mItems[mFocusRow][0])
	{
		gFBUIManager->SetFocusUI(mItems[mFocusRow][0]);
	}
	
}

}