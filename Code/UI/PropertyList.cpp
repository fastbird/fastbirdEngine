#include <UI/StdAfx.h>
#include <UI/PropertyList.h>

namespace fastbird
{
PropertyList::PropertyList()
	: ListBox()
	, mFocusRow(-1)
{
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	mRowHeight = 22;
}
PropertyList::~PropertyList()
{

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


IWinBase* PropertyList::GetTextField(const wchar_t* key, unsigned* index)
{
	for (unsigned row = 0; row < mItems.size(); ++row)
	{
		if (mItems[row].empty())
			continue;
		if (wcscmp(mItems[row][0]->GetText(), key) == 0)
		{
			if (index)
				*index = row;
			return mItems[row][1]->GetChild((unsigned)0);
		}
	}
	return 0;
}

const wchar_t* PropertyList::GetValue(const wchar_t* key)
{
	unsigned row;
	auto textField = GetTextField(key, &row);
	if (textField)
		return textField->GetText();
	return 0;
}

unsigned PropertyList::InsertItem(const wchar_t* key, const wchar_t* value)
{
	assert(key && value);
	if (GetValue(key))
	{
		ModifyItem(key, value);
		return -1;
	}
	float nh = PixelToLocalNHeight(mRowHeight);
	float nextPosY = PixelToLocalNHeight(mRowHeight + mRowGap);
	float nposY = 0.0f;
	if (!mItems.empty())
	{
		auto item = mItems.back();
		assert(!item.empty());
		nposY = item[0]->GetNPos().y + nextPosY;
	}

	mItems.push_back(ROW());
	ROW& row = mItems.back();
	ListItem* pKeyItem = CreateNewKeyItem(mItems.size() - 1, 0, nposY);
	row.push_back(pKeyItem);
	pKeyItem->SetText(key);
	unsigned curRow = mItems.size() - 1;
	GetRowId(curRow);

	ListItem* pValueItem = CreateNewValueItem(pKeyItem->GetRowIndex(), 1, nposY);
	row.push_back(pValueItem);
	auto textField = pValueItem->GetChild((unsigned)0);
	textField->SetText(value);

	return curRow;
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

	item->SetProperty(UIProperty::NO_BACKGROUND, "false");
	item->SetVisible(mVisibility.IsVisible());
	item->SetRowIndex(row);
	item->SetColIndex(col);
	item->SetProperty(UIProperty::TEXT_LEFT_GAP, "5");
	return item;
}

ListItem* PropertyList::CreateNewValueItem(int row, int col, float ny)
{
	float nh = PixelToLocalNHeight(mRowHeight);
	ListItem* item = (ListItem*)AddChild(0.41f, ny, 0.59f, nh, ComponentType::ListItem);
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

	unsigned row;
	auto textField = GetTextField(key, &row);
	if (textField)
	{
		textField->SetText(value);
		return row;
	}
	return -1;
}
void PropertyList::RemoveItem(const wchar_t* key)
{
	unsigned index = -1;
	for (unsigned row = 0; row < mItems.size(); ++row)
	{
		if (mItems[row].empty())
			continue;
		if (wcscmp(mItems[row][0]->GetText(), key) == 0)
		{
			index = row;
			break;
		}
	}
	if (index == -1)
		return;

	assert(index < mItems.size());
	float nh = PixelToLocalNHeight(mRowHeight + mRowGap);
	for (size_t row = index + 1; row < mItems.size(); ++row)
	{
		for (size_t col = 0; col < mItems[row].size(); ++col)
		{
			Vec2 npos = mItems[row][col]->GetNPos();
			npos.y -= nh;
			mItems[row][col]->SetNPos(npos);
			mItems[row][col]->SetRowIndex(row - 1);
		}
	}
	auto left = GetChild("__Key");
	left->RemoveChild(mItems[index][0]);
	auto right = GetChild("__Value");
	right->RemoveChild(mItems[index][1]);
	mItems.erase(mItems.begin() + index);
}

void PropertyList::ClearItems()
{
	auto left = GetChild("__Key");
	auto right = GetChild("__Value");
	left->RemoveAllChild();
	right->RemoveAllChild();
	mItems.clear();
	mRowIds.clear();
}

bool PropertyList::GetCurKeyValue(std::string& key, std::string& value)
{
	if (mFocusRow == -1)
		return false;
	assert(mFocusRow < mItems.size());
	key = WideToAnsi(mItems[mFocusRow][0]->GetText());
	value = WideToAnsi(mItems[mFocusRow][1]->GetChild((unsigned)0)->GetText());
	return true;
}

}