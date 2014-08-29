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
	: mIndex(INVALID_INDEX)
{
	mUIObject = IUIObject::CreateUIObject(false);
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	mUIObject->SetNoDrawBackground(true);
}

void ListItem::GatherVisit(std::vector<IUIObject*>& v)
{
	v.push_back(mUIObject);	
}

void ListItem::OnPosChanged()
{
	WinBase::OnPosChanged();
	mUIObject->SetTextStartNPos(Vec2(mWNPos.x, mWNPos.y + mWNSize.y - GetTextBottomGap()));
}

void ListItem::OnSizeChanged()
{
	WinBase::OnSizeChanged();
	mUIObject->SetTextStartNPos(Vec2(mWNPos.x, mWNPos.y + mWNSize.y - GetTextBottomGap()));
}

//-----------------------------------------------------------------------------
ListBox::ListBox()
	: mNextHeight(0.01f)
	, mScroller(0)
	, mCurSelected(ListItem::INVALID_INDEX)
{
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
}

void ListBox::GatherVisit(std::vector<IUIObject*>& v)
{
	__super::GatherVisit(v);
}

void ListBox::Scrolled()
{
	if (mScroller)
	{
		Vec2 offset = mScroller->GetOffset();
		for each(auto item in mItems)
		{
			item->SetNPosOffset(offset);
		}
	}
}

std::string ListBox::GetSelectedString()
{
	if (mCurSelected!=ListItem::INVALID_INDEX)
	{
		if (mItems[mCurSelected]->GetText())
			return WideToAnsi(mItems[mCurSelected]->GetText());
	}
	return std::string();
}

void ListBox::InsertItem(const wchar_t* szString)
{
	mItems.push_back(static_cast<ListItem*>(
		AddChild(0.01f, mNextHeight, 0.97f, 0.07f, ComponentType::ListItem)));
	ListItem* pAddedItem = mItems.back();
	const RECT& rect = mUIObject->GetRegion();
	pAddedItem->SetScissorRect(true, rect);
	pAddedItem->SetProperty(UIProperty::BACK_COLOR, "0.4, 0.4, 0.3, 1.0");
	pAddedItem->SetProperty(UIProperty::NO_BACKGROUND, "true");
	pAddedItem->SetIndex(mItems.size()-1);
	pAddedItem->RegisterEventFunc(IEventHandler::EVENT_MOUSE_CLICK,
		std::bind(&ListBox::OnItemClicked, this, std::placeholders::_1));
	pAddedItem->RegisterEventFunc(IEventHandler::EVENT_MOUSE_DOUBLE_CLICK,
		std::bind(&ListBox::OnItemDoubleClicked, this, std::placeholders::_1));
	pAddedItem->SetText(szString);

	float contentWNEnd = pAddedItem->GetWNPos().y + pAddedItem->GetWNSize().y;
	float boxWNEnd = mWNPos.y + mWNSize.y;	
	
	mNextHeight+=0.08f;

	if (contentWNEnd > boxWNEnd && !mScroller)
	{
		mScroller = static_cast<Scroller*>(AddChild(0.97f, 0.0f, 0.03f, 1.0f, ComponentType::Scroller));
		mScroller->SetOwner(this);
	}
	
	if (mScroller)
	{
		float length = contentWNEnd - boxWNEnd;
		assert(length>0);
		mScroller->SetMaxOffset(Vec2(0, length));
	}
}

void ListBox::OnItemClicked(void* arg)
{
	ListItem* pItem = (ListItem*)arg;
	size_t index = pItem->GetIndex();
	if (index!=ListItem::INVALID_INDEX)
	{
		if (mCurSelected != ListItem::INVALID_INDEX)
		{
			mItems[mCurSelected]->SetProperty(UIProperty::NO_BACKGROUND, "true");
		}
		mItems[index]->SetProperty(UIProperty::NO_BACKGROUND, "false");
		mCurSelected = index;
	}
	OnEvent(IEventHandler::EVENT_MOUSE_CLICK);
}

void ListBox::OnItemDoubleClicked(void* arg)
{
	ListItem* pItem = (ListItem*)arg;
	size_t index = pItem->GetIndex();
	if (index!=ListItem::INVALID_INDEX)
	{
		if (mCurSelected != ListItem::INVALID_INDEX)
		{
			mItems[mCurSelected]->SetProperty(UIProperty::NO_BACKGROUND, "true");
		}
		mItems[index]->SetProperty(UIProperty::NO_BACKGROUND, "false");
		mCurSelected = index;
	}
	OnEvent(IEventHandler::EVENT_MOUSE_DOUBLE_CLICK);
}

void ListBox::Clear()
{
	for each(auto item in mItems)
	{
		RemoveChild(item);
	}
	mItems.clear();
	mNextHeight = 0.01f;
	if (mScroller)
	{
		RemoveChild(mScroller);
		mScroller = 0;
	}
	mCurSelected = ListItem::INVALID_INDEX;
}

}