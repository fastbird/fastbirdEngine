#include <UI/StdAfx.h>
#include <UI/PropertyList.h>
#include <UI/Scroller.h>
#include <UI/IUIManager.h>
#include <UI/TextField.h>
#include <UI/ListItem.h>
#include <UI/ListBoxData.h>
#include <Engine/TextManipulator.h>

namespace fastbird
{
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
		mItems.push_back(ROW());
		mItems.back().push_back(0);
		mItems.back().push_back(0);
	}
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
		//scroll
	if (mScrollerV)
	{
		if (index < mStartIndex+1 || index >(mEndIndex>=3 ? mEndIndex - 3 : mEndIndex))
		{
			unsigned hgap = mRowHeight + mRowGap;
			unsigned destY = hgap * index + mRowGap;

			mScrollerV->SetOffset(Vec2(0.f, -(destY / (float)GetRenderTargetSize().y)));
		}
	}

	if (mItems[index][1])
	{
		gFBUIManager->SetFocusUI(mItems[index][1]->GetChild((unsigned)0));
		gFBUIManager->GetTextManipulator()->SelectAll();
		TriggerRedraw();
	}
}

void PropertyList::MoveLine(bool applyInput, bool next)
{
	// apply value;
	if (applyInput)
		OnEvent(EVENT_ENTER);

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
	if (mScrollerV)
	{
		if (nextLine < mStartIndex+1 ||
			(nextLine >(mEndIndex >= 3 ? mEndIndex - 3 : mEndIndex))
			)
		{
			unsigned hgap = mRowHeight + mRowGap;
			unsigned destY = hgap * nextLine + mRowGap;

				mScrollerV->SetOffset(Vec2(0.f, -(destY / (float)GetRenderTargetSize().y)));
		}
	}

	if (mItems[nextLine][0])
	{
		gFBUIManager->SetFocusUI(mItems[nextLine][0]);
	}

	mFocusRow = nextLine;	
	
}

void PropertyList::RemoveHighlight(unsigned index)
{
	if (mItems[index][0])
	{
		mItems[index][0]->SetProperty(UIProperty::NO_BACKGROUND, "true");
	}
}

void PropertyList::MoveFocusToKeyItem()
{
	if (mItems[mFocusRow][0])
	{
		gFBUIManager->SetFocusUI(mItems[mFocusRow][0]);
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