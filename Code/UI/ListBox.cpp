#include <UI/StdAfx.h>
#include <UI/ListBox.h>
#include <UI/Scroller.h>
#include <UI/ImageBox.h>
#include <UI/Button.h>
#include <UI/CheckBox.h>
#include <CommonLib/StringUtils.h>

namespace fastbird
{
//-----------------------------------------------------------------------------
const float ListItem::LEFT_GAP = 0.001f;
const size_t ListItem::INVALID_INDEX = -1;

ListItem::ListItem()
: mRowIndex(INVALID_INDEX)
, mColIndex(INVALID_INDEX)
, mNoBackground(true)
, mBackColor("0.1, 0.3, 0.3, 0.7")
{
	assert(mUIObject);
	//mUIObject = IUIObject::CreateUIObject(false);
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
}

ListItem::~ListItem()
{

}

void ListItem::GatherVisit(std::vector<IUIObject*>& v)
{
	v.push_back(mUIObject);	

	__super::GatherVisit(v);
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

CheckBox* ListItem::GetCheckBox() const
{
	for (auto& child : mChildren)
	{
		if (child->GetType() == ComponentType::CheckBox)
		{
			return dynamic_cast<CheckBox*>(child);
		}
	}
	return 0;
}

void ListItem::SetBackColor(const char* backColor)
{
	if_assert_fail(backColor)
		return;
	mBackColor = backColor;
}

void ListItem::SetNoBackground(bool noBackground)
{
	mNoBackground = noBackground;
}

//-----------------------------------------------------------------------------
ListBox::ListBox()
	: mCurSelectedCol(ListItem::INVALID_INDEX)
	, mNumCols(1)
	, mRowHeight(26)
	, mRowGap(4)
	, mHighlightColor("0.1, 0.3, 0.3, 0.7")
{
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	mColSizes.push_back(1.0f);
	mUseScrollerV = true;
}

ListBox::~ListBox()
{

}

void ListBox::GatherVisit(std::vector<IUIObject*>& v)
{
	__super::GatherVisit(v);
}

std::string ListBox::GetSelectedString()
{
	if (!mSelectedRows.empty() && mSelectedRows.back() != ListItem::INVALID_INDEX && mCurSelectedCol != ListItem::INVALID_INDEX)
	{
		if (mItems[mSelectedRows.back()][mCurSelectedCol]->GetText())
			return WideToAnsi(mItems[mSelectedRows.back()][mCurSelectedCol]->GetText());
	}
	return std::string();
}

ListItem* ListBox::CreateNewItem(int row, int col, const Vec2& npos, const Vec2& nsize)
{
	ListItem* item = (ListItem*)AddChild(npos.x, npos.y, nsize.x, nsize.y, ComponentType::ListItem);
	if (col < (int)mColAlignes.size())
		item->SetProperty(UIProperty::TEXT_ALIGN, mColAlignes[col].c_str());
	if (col < (int)mTextSizes.size())
	{
		item->SetProperty(UIProperty::TEXT_SIZE, mTextSizes[col].c_str());
	}
	item->SetProperty(UIProperty::BACK_COLOR, mHighlightColor.c_str());
	item->SetProperty(UIProperty::NO_BACKGROUND, "true");
	item->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&ListBox::OnItemClicked, this, std::placeholders::_1));
	item->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_DOUBLE_CLICK,
		std::bind(&ListBox::OnItemDoubleClicked, this, std::placeholders::_1));
	item->SetVisible(mVisible);
	item->SetRowIndex(row);
	item->SetColIndex(col);
	item->SetBackColor(mHighlightColor.c_str());
	item->SetNoBackground(true);
	return item;
}

unsigned ListBox::InsertItem(const wchar_t* szString)
{	
	float nh = PixelToLocalNHeight(mRowHeight);
	float nextPosY = PixelToLocalNHeight(mRowHeight + mRowGap);
	float posY = 0.0f;
	if (!mItems.empty())
	{
		auto item = mItems.back();
		assert(!item.empty());
		posY = item[0]->GetNPos().y + nextPosY;
	}

	mItems.push_back(ROW());
	ROW& row = mItems.back();
	ListItem* pAddedItem = CreateNewItem(mItems.size()-1, 0, Vec2(0, posY), Vec2(mColSizes[0], nh));
	row.push_back(pAddedItem);
	if (szString)
		pAddedItem->SetText(szString);

	unsigned curRow = mItems.size() - 1;
	GetRowId(curRow);

	return curRow;
}

unsigned ListBox::InsertItem(ITexture* texture)
{
	auto row = InsertItem(L"");
	auto imageBox = (ImageBox*)mItems[row][0]->AddChild(0, 0, 1.0, 1.0, ComponentType::ImageBox);
	imageBox->SetTexture(texture);
	imageBox->SetVisible(mVisible);
	imageBox->SetProperty(UIProperty::NO_MOUSE_EVENT, "true");
	return row;
}

unsigned ListBox::InsertCheckBoxItem(bool check)
{
	auto row = InsertItem(L"");
	auto checkbox = (CheckBox*)mItems[row][0]->AddChild(0, 0, 1, 1, ComponentType::CheckBox);
	checkbox->SetSize(Vec2I(24, 24));
	checkbox->SetCheck(check);
	checkbox->SetVisible(mVisible);
	return row;
}

CheckBox* ListBox::GetCheckBox(unsigned row, unsigned col) const
{
	if (row >= mItems.size())
	{
		assert(0);
		return 0;
	}

	return mItems[row][col]->GetCheckBox();
}

void ListBox::RemoveItem(size_t index)
{
	assert(index < mItems.size());
	float nh = PixelToLocalNHeight(mRowHeight + mRowGap);
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
	for (auto& idx : mSelectedRows)
	{
		if (idx > index)
		{
			idx--;
		}
	}
	
	mItems.erase(mItems.begin() + index);
	if (index >= mItems.size())
	{
		DeleteValuesInVector(mSelectedRows, index);
	}
	else if (!ValueNotExistInVector(mSelectedRows, index))
	{
		for (size_t i = 0; i < mNumCols; ++i)
		{
			mItems[index][i]->SetProperty(UIProperty::NO_BACKGROUND, "false");
		}
	}
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
	float nh = PixelToLocalNHeight(mRowHeight);
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
		ListItem* pAddedItem = CreateNewItem(row, addingColIndex, Vec2(posx, posy), Vec2(mColSizes[addingColIndex], nh));
		r.push_back(pAddedItem);		
	}

	mItems[row][col]->SetText(szString);
}

void ListBox::SetItemTexture(size_t row, size_t col, ITexture* texture)
{
	SetItemString(row, col, L"");
	auto imageBox = (ImageBox*)mItems[row][col]->AddChild(0, 0, 1.0, 1.0, ComponentType::ImageBox);
	imageBox->SetTexture(texture);
	imageBox->SetVisible(mVisible);
	imageBox->SetProperty(UIProperty::NO_MOUSE_EVENT, "true");
}

void ListBox::SetItemTexture(size_t row, size_t col, const char* texturePath)
{
	SetItemString(row, col, L"");
	auto imageBox = (ImageBox*)mItems[row][col]->AddChild(0, 0, 1.0, 1.0, ComponentType::ImageBox);
	imageBox->SetTexture(texturePath);
	imageBox->SetVisible(mVisible);
	imageBox->SetProperty(UIProperty::NO_MOUSE_EVENT, "true");
}

void ListBox::SetTextureAtlas(const char* atlas)
{
	assert(atlas);
	mTextureAtlas = atlas;
}

void ListBox::SetItemTextureRegion(size_t row, size_t col, const char* region)
{
	assert(region && strlen(region)!=0);
	assert(!mTextureAtlas.empty());
	SetItemString(row, col, L"");
	auto imageBox = (ImageBox*)mItems[row][col]->AddChild(0, 0, 1.0, 1.0, ComponentType::ImageBox);
	imageBox->SetProperty(UIProperty::TEXTUREATLAS, mTextureAtlas.c_str());
	imageBox->SetProperty(UIProperty::REGION, region);
	imageBox->SetVisible(mVisible);
	imageBox->SetProperty(UIProperty::NO_MOUSE_EVENT, "true");
	imageBox->DrawAsFixedSizeAtCenter();
}

IWinBase* ListBox::SetItemIconText(size_t row, size_t col, const char* region, const char* txt, unsigned iconSize)
{
	assert(region && strlen(region) != 0);
	assert(!mTextureAtlas.empty());
	SetItemString(row, col, L"");
	Button* button = 0;
	if (mItems[row][col]->GetNumChildren() > 0)
	{
		button = dynamic_cast<Button*>(mItems[row][col]->GetChild((unsigned)0));
	}
	if (!button)
		button = (Button*)mItems[row][col]->AddChild(0.f, 0.f, 1.0f, 1.0f, ComponentType::Button);

	button->SetVisible(mVisible);
	button->SetProperty(UIProperty::NO_MOUSE_EVENT, "true");
	button->SetProperty(UIProperty::TEXT_ALIGN, "center");
	button->SetProperty(UIProperty::TEXT, txt);	
	button->SetProperty(UIProperty::NO_BACKGROUND, "true");
	button->SetProperty(UIProperty::TEXTUREATLAS, mTextureAtlas.c_str());
	button->SetProperty(UIProperty::REGION, region);
	char buf[128];
	sprintf_s(buf, "%u", iconSize);
	button->SetProperty(UIProperty::BUTTON_ICON_TEXT, buf);
	return button;
}

void ListBox::SetHighlightRow(size_t row, bool highlight)
{
	for (size_t i = 0; i < mNumCols; ++i)
	{
		if (mItems[row].size() <= i)
			break;
		if (highlight)
		{
			mItems[row][i]->SetProperty(UIProperty::NO_BACKGROUND, "false");
			mItems[row][i]->SetProperty(UIProperty::BACK_COLOR, mHighlightColor.c_str());
		}
		else
		{
			mItems[row][i]->SetProperty(UIProperty::NO_BACKGROUND, StringConverter::toString(mItems[row][i]->GetNoBackground()).c_str());
			mItems[row][i]->SetProperty(UIProperty::BACK_COLOR, mItems[row][i]->GetBackColor());
		}
	}
}

void ListBox::SetHighlightRowAndSelect(size_t row, bool highlight)
{
	SetHighlightRow(row, highlight);
	if (highlight)
	{
		if (ValueNotExistInVector(mSelectedRows, row))
			mSelectedRows.push_back(row);
	}
	else
	{
		DeleteValuesInVector(mSelectedRows, row);
	}
}

void ListBox::OnItemClicked(void* arg)
{
	ListItem* pItem = (ListItem*)arg;
	size_t rowindex = pItem->GetRowIndex();
	size_t colindex = pItem->GetColIndex();
	if (rowindex != ListItem::INVALID_INDEX)
	{
		auto keyboard = gEnv->pEngine->GetKeyboard();
		if (keyboard->IsKeyDown(VK_CONTROL))
		{			
			if (ValueNotExistInVector(mSelectedRows, rowindex))
			{
				SetHighlightRow(rowindex, true);
				mSelectedRows.push_back(rowindex);
			}
			else
			{
				SetHighlightRow(rowindex, false);
				DeleteValuesInVector(mSelectedRows, rowindex);
			}
		}
		else if (keyboard->IsKeyDown(VK_SHIFT))
		{
			if (mSelectedRows.empty())
			{
				SetHighlightRow(rowindex, true);
				mSelectedRows.push_back(rowindex);
			}
			else
			{
				size_t start = mSelectedRows.back();
				int num = rowindex - start;
				if (num > 0)
				{
					for (size_t i = start+1; i <= rowindex; i++)
					{
						SetHighlightRow(i, true);
						if (ValueNotExistInVector(mSelectedRows, i))
							mSelectedRows.push_back(i);
					}
				}
				else if (num < 0)
				{
					for (int i = start - 1; i >= (int)rowindex; i--)
					{
						SetHighlightRow(i, true);
						if (ValueNotExistInVector(mSelectedRows, i))
							mSelectedRows.push_back(i);
					}
				}
				else
				{
					SetHighlightRow(rowindex, false);
					DeleteValuesInVector(mSelectedRows, rowindex);
				}
			}

		}
		else
		{
			for (const auto& idx : mSelectedRows)
			{
				SetHighlightRow(idx, false);
			}
			mSelectedRows.clear();
			
			SetHighlightRow(rowindex, true);
			mSelectedRows.push_back(rowindex);
		}
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
		for (const auto& idx : mSelectedRows)
		{
			SetHighlightRow(idx, false);
		}
		mSelectedRows.clear();
		mSelectedRows.push_back(rowindex);
		SetHighlightRow(rowindex, true);
		mCurSelectedCol = colindex;
	}
	OnEvent(IEventHandler::EVENT_MOUSE_LEFT_DOUBLE_CLICK);
}

void ListBox::Clear()
{
	for (auto items : mItems)
	{
		for (auto item : items)
		{
			RemoveChild(item);
		}
	}
	mItems.clear();
	mSelectedRows.clear();
	mRowIds.clear();
	mCurSelectedCol = ListItem::INVALID_INDEX;
}

bool ListBox::SetProperty(UIProperty::Enum prop, const char* val)
{
	switch (prop)
	{
	case UIProperty::LISTBOX_HIGHLIGHT_COLOR:
	{
		mHighlightColor = val;
		return true;
	}
	case UIProperty::LISTBOX_COL:
	{
									mNumCols = StringConverter::parseUnsignedInt(val);
									float colsize = 1.0f / (float)mNumCols;
									mColSizes.clear();
									for (unsigned i = 0; i < mNumCols; ++i)
									{
										mColSizes.push_back(colsize);
									}
									return true;
	}
	case UIProperty::LISTBOX_ROW_HEIGHT:
		{
			mRowHeight = StringConverter::parseInt(val);
			return true;
		}
	case UIProperty::LISTBOX_ROW_GAP:
	{
										mRowGap = StringConverter::parseInt(val);
										return true;
	}

	case UIProperty::LISTBOX_COL_SIZES:
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

	case UIProperty::LISTBOX_TEXT_SIZES:
	{
										  // set UIProperty::LISTBOX_COL first
										  mTextSizes.clear();
										  StringVector strs = Split(val);
										  assert(!strs.empty());
										  for (unsigned i = 0; i < strs.size(); ++i)
										  {
											  mTextSizes.push_back(strs[i]);
										  }
										  return true;
	}
	case UIProperty::LISTBOX_COL_ALIGNH:
		{
			assert(mNumCols != 1);
			mColAlignes.clear();
			mColAlignes.reserve(mNumCols);
			StringVector strs = Split(val);
			assert(!strs.empty());
			unsigned i = 0;
			for (; i < strs.size(); ++i)
			{
				mColAlignes.push_back(strs[i]);
			}
			const char* lastAlign = "center";
			if (!mColAlignes.empty())
				lastAlign = mColAlignes.back().c_str();
			while (mColAlignes.size() < mNumCols)
			{
				mColAlignes.push_back(lastAlign);
			}
			return true;
		}

	case UIProperty::LISTBOX_COL_HEADERS_TEXT_SIZE:
	{
										   assert(mNumCols != 1);
										   mHeaderTextSize.clear();
										   StringVector strs = Split(val);
										   assert(!strs.empty());
										   for (unsigned i = 0; i < strs.size(); ++i)
										   {
											   mHeaderTextSize.push_back(strs[i]);
										   }
										   return true;
	}

	case UIProperty::LISTBOX_COL_HEADERS:
		{
			StringVector strs = Split(val, ",");
			assert(strs.size() == mNumCols);
			float nh = PixelToLocalNHeight(mRowHeight);
			if (mHeaders.empty())
			{
				for (unsigned i = 0; i < mNumCols; ++i)
				{
					float posx = 0.0f;
					if (i >= 1)
					{
						posx = mHeaders[i - 1]->GetNPos().x + mHeaders[i - 1]->GetNSize().x;
					}

					mHeaders.push_back(static_cast<ListItem*>(
						AddChild(posx, 0.0f, mColSizes[i], nh, ComponentType::ListItem)));
					ListItem* pAddedItem = mHeaders.back();
					const RECT& rect = mUIObject->GetRegion();
					pAddedItem->SetProperty(UIProperty::NO_BACKGROUND, "false");
					pAddedItem->SetProperty(UIProperty::BACK_COLOR, "0.0, 0.0, 0.0, 0.5");
					if (!mHeaderTextSize.empty() && i < mHeaderTextSize.size())
					{
						pAddedItem->SetProperty(UIProperty::TEXT_SIZE, mHeaderTextSize[i].c_str());
					}
					else
					{
						pAddedItem->SetProperty(UIProperty::TEXT_SIZE, "24");
					}
					pAddedItem->SetProperty(UIProperty::TEXT_ALIGN, "center");
					pAddedItem->SetRowIndex(-1);
					pAddedItem->SetColIndex(i);
					pAddedItem->SetText(AnsiToWide(strs[i].c_str()));
				}
				assert(!mWndContentUI);
				mWndContentUI = (Wnd*)AddChild(0.0f, 0.0f, 1.0f, 1.0f, ComponentType::Window);
				Vec2I sizeMod = { 0, -(mRowHeight + 4) };
				mWndContentUI->SetSizeModificator(sizeMod);
				mWndContentUI->SetUseAbsYSize(true);
				mWndContentUI->SetPos(Vec2I(0, (mRowHeight + 4)));
				mWndContentUI->SetProperty(UIProperty::NO_BACKGROUND, "true");
				if (mUseScrollerV)
				{
					mUseScrollerV = false;
					mWndContentUI->SetProperty(UIProperty::SCROLLERV, "true");
				}
			}
			else
			{
				assert(mHeaders.size() == strs.size());
				int i = 0;
				for (auto& str : strs)
				{
					mHeaders[i]->SetText(AnsiToWide(strs[i].c_str()));
					++i;
				}
			}
			if (mUseScrollerV)
			{
				RemoveChild(mScrollerV, true);
				mScrollerV = 0;
			}
			
			return true;
		}
	case UIProperty::TEXTUREATLAS:
	{
									 assert(val);
									 mTextureAtlas = val;
									 return true;
	}
	}

	

	return __super::SetProperty(prop, val);
}

ListItem* ListBox::GetItem(size_t row, size_t col) const
{
	assert(row < mItems.size());
	assert(col < mItems[row].size());
	return mItems[row][col];

}

void ListBox::SelectRow(unsigned row)
{
	if (row < mItems.size())
	{
		if (ValueNotExistInVector(mSelectedRows, row))
		{
			SetHighlightRow(row, true);
			mSelectedRows.push_back(row);
		}
	}
	else if (!mItems.empty())
	{
		unsigned idx = mItems.size() - 1;
		if (ValueNotExistInVector(mSelectedRows, idx))
		{
			SetHighlightRow(idx, true);
			mSelectedRows.push_back(idx);
		}
	}
	OnEvent(IEventHandler::EVENT_MOUSE_LEFT_CLICK);
}

void ListBox::ClearSelection()
{
	for (auto& row : mSelectedRows)
	{
		SetHighlightRow(row, false);
	}
	mSelectedRows.clear();
	OnEvent(IEventHandler::EVENT_LISTBOX_CLEARED);
}

bool ListBox::IsSelected(unsigned row)
{
	return !ValueNotExistInVector(mSelectedRows, row);
}

bool ListBox::OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard)
{
	if (!mVisible)
		return false;

	if (mNoMouseEvent)
	{
		return false;
	}

	mMouseIn = __super::OnInputFromHandler(mouse, keyboard);

	if (keyboard->IsValid() && mMouseIn)
	{
		if (keyboard->IsKeyPressed(VK_UP))
		{
			if (keyboard->IsKeyDown(VK_SHIFT))
			{
				if (mSelectedRows.empty())
				{
					if (!mItems.empty())
					{
						SetHighlightRow(mItems.size()-1, true);
						mSelectedRows.push_back(mItems.size() - 1);
						keyboard->Invalidate();
						OnEvent(IEventHandler::EVENT_MOUSE_LEFT_CLICK);
					}
				}
				else
				{
					unsigned lastRow = mSelectedRows.back();
					if (lastRow != 0)
					{
						unsigned dest = lastRow - 1;
						if (IsSelected(dest))
						{
							SetHighlightRow(lastRow, false);
							DeleteValuesInVector(mSelectedRows, lastRow);
							keyboard->Invalidate();
							OnEvent(IEventHandler::EVENT_MOUSE_LEFT_CLICK);
						}
						else
						{
							SetHighlightRow(dest, true);
							if (ValueNotExistInVector(mSelectedRows, dest))
							{
								mSelectedRows.push_back(dest);
								keyboard->Invalidate();
								OnEvent(IEventHandler::EVENT_MOUSE_LEFT_CLICK);
							}								
						}
					}
				}
			}
			else
			{
				if (mSelectedRows.empty())
				{
					if (!mItems.empty())
					{
						unsigned dest = mItems.size() - 1;
						SetHighlightRowAndSelect(dest, true);
						keyboard->Invalidate();
						OnEvent(IEventHandler::EVENT_MOUSE_LEFT_CLICK);
					}
				}
				else
				{
					unsigned last = mSelectedRows.back();
					if (last != 0)
					{
						ClearSelection();
						SetHighlightRowAndSelect(last-1, true);
						keyboard->Invalidate();
						OnEvent(IEventHandler::EVENT_MOUSE_LEFT_CLICK);
					}
				}
			}
		}
		else if (keyboard->IsKeyPressed(VK_DOWN))
		{
			if (keyboard->IsKeyDown(VK_SHIFT))
			{
				if (mSelectedRows.empty())
				{
					if (!mItems.empty())
					{
						SetHighlightRowAndSelect(0, true);
						keyboard->Invalidate();
						OnEvent(IEventHandler::EVENT_MOUSE_LEFT_CLICK);
					}
				}
				else
				{
					unsigned lastRow = mSelectedRows.back();
					if (lastRow+1 < mItems.size())
					{
						unsigned dest = lastRow + 1;
						if (IsSelected(dest))
						{
							SetHighlightRowAndSelect(lastRow, false);
							keyboard->Invalidate();
							OnEvent(IEventHandler::EVENT_MOUSE_LEFT_CLICK);
						}
						else
						{
							SetHighlightRowAndSelect(dest, true);
							keyboard->Invalidate();
							OnEvent(IEventHandler::EVENT_MOUSE_LEFT_CLICK);
						}
					}
				}
			}
			else
			{
				if (mSelectedRows.empty())
				{
					if (!mItems.empty())
					{
						unsigned dest = 0;
						SetHighlightRowAndSelect(dest, true);
						keyboard->Invalidate();
						OnEvent(IEventHandler::EVENT_MOUSE_LEFT_CLICK);
					}
				}
				else
				{
					unsigned last = mSelectedRows.back();
					if (last +1 < mItems.size())
					{
						ClearSelection();
						SetHighlightRowAndSelect(last + 1, true);
						keyboard->Invalidate();
						OnEvent(IEventHandler::EVENT_MOUSE_LEFT_CLICK);
					}
				}
			}
		}

	}
	return mMouseIn;
}
unsigned ListBox::GetNumRows()
{
	return mItems.size();
}

IWinBase* ListBox::MakeMergedRow(unsigned row)
{
	if (mItems.size() < row)
		return 0;

	if (mItems[row].empty())
		return 0;

	mItems[row][0]->SetNSizeX(1.0f);
	mItems[row][0]->SetSizeModificator(Vec2I(-4, 0));
	mItems[row][0]->SetProperty(UIProperty::NO_BACKGROUND, "false");
	mItems[row][0]->SetProperty(UIProperty::BACK_COLOR, "0, 0, 0, 0.3");
	mItems[row][0]->SetProperty(UIProperty::NO_MOUSE_EVENT, "true");
	mItems[row][0]->SetProperty(UIProperty::TEXT_COLOR, "0.2, 0.6, 0.2, 1.0");
	mItems[row][0]->SetProperty(UIProperty::TEXT_ALIGN, "center");
	mItems[row][0]->SetNoBackground(false);
	mItems[row][0]->SetBackColor("0, 0, 0, 0.3");


	return mItems[row][0];
}

IWinBase* ListBox::MakeMergedRow(unsigned row, const char* backColor, const char* textColor, bool noMouseEvent)
{
	if (mItems.size() < row)
		return 0;

	if (mItems[row].empty())
		return 0;

	mItems[row][0]->SetNSizeX(1.0f);
	mItems[row][0]->SetSizeModificator(Vec2I(-4, 0));
	
	if (backColor && strlen(backColor) != 0)
	{
		mItems[row][0]->SetProperty(UIProperty::BACK_COLOR, backColor);
		mItems[row][0]->SetBackColor(backColor);
		mItems[row][0]->SetProperty(UIProperty::NO_BACKGROUND, "false");
		mItems[row][0]->SetNoBackground(false);
	}
		
	if (textColor && strlen(textColor) !=0)
		mItems[row][0]->SetProperty(UIProperty::TEXT_COLOR, textColor);

	if (noMouseEvent)
		mItems[row][0]->SetProperty(UIProperty::NO_MOUSE_EVENT, "true");	
	mItems[row][0]->SetProperty(UIProperty::TEXT_ALIGN, "center");

	return mItems[row][0];
}

void ListBox::SetRowId(unsigned row, unsigned id)
{
	while (row >= mRowIds.size())
	{
		mRowIds.push_back(-1);
	}
	mRowIds[row] = id;
}

void ListBox::SwapItems(unsigned row0, unsigned row1)
{
	if_assert_fail(row0 < mRowIds.size() && row1 < mRowIds.size())
		return;

	auto cols = mItems[row0].size();
	if_assert_fail( cols == mItems[row1].size())
		return;

	bool row0selected = !ValueNotExistInVector(mSelectedRows, row0);
	bool row1selected = !ValueNotExistInVector(mSelectedRows, row1);
	if (row0selected && !row1selected)
	{
		DeleteValuesInVector(mSelectedRows, row0);
		mSelectedRows.push_back(row1);
	}
	else if (!row0selected && row1selected)
	{
		DeleteValuesInVector(mSelectedRows, row1);
		mSelectedRows.push_back(row0);
	}
	for (unsigned col = 0; col < mItems[row0].size(); col++)
	{
		ListItem* item0 = mItems[row0][col];
		ListItem* item1 = mItems[row1][col];
		std::swap(mItems[row0][col], mItems[row1][col]);
		
		auto item0RowIndex = item0->GetRowIndex();
		auto item1RowIndex = item1->GetRowIndex();
		item0->SetRowIndex(item1RowIndex);
		item1->SetRowIndex(item0RowIndex);
		
		auto pos0 = item0->GetNPos();
		auto pos1 = item1->GetNPos();
		item0->SetNPos(pos1);
		item1->SetNPos(pos0);
	}
	std::swap(mRowIds[row0], mRowIds[row1]);

}

unsigned ListBox::GetRowId(unsigned row)
{
	while (row >= mRowIds.size())
	{
		mRowIds.push_back(-1);
	}
	return mRowIds[row];
}
unsigned ListBox::FindRowWithId(unsigned id)
{
	unsigned row = 0;
	for (auto rowId : mRowIds)
	{
		if (rowId == id)
			return row;

		++row;
	}

	return -1;
}

void ListBox::DeleteRow(unsigned targetRow)
{
	if (targetRow >= mItems.size())
		return;

	ClearSelection();
	unsigned num = mItems.size();
	for (unsigned i = num - 1; i > targetRow; --i)
	{
		auto toRow = mItems[i - 1];
		auto fromRow = mItems[i];
		unsigned numColToRow = toRow.size();
		unsigned numColFromRow = fromRow.size();
		for (unsigned c = 0; c < numColToRow && c < numColFromRow; ++c)
		{
			float y = toRow[c]->GetNPos().y;
			fromRow[c]->SetNPosY(y);
			fromRow[c]->SetRowIndex(toRow[c]->GetRowIndex());
		}
	}

	for (auto item : mItems[targetRow])
	{
		RemoveChild(item);
	}
	mItems.erase(mItems.begin() + targetRow);
	if (mRowIds.size() > targetRow)
	{
		mRowIds.erase(mRowIds.begin() + targetRow);
	}
}

void ListBox::GetSelectedRowIds(std::vector<unsigned>& ids) const
{
	ids.clear();
	for (unsigned rowIdx : mSelectedRows)
	{
		ids.push_back(mRowIds[rowIdx]);
	}
}

}