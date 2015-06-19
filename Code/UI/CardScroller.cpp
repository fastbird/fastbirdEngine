#include <UI/StdAfx.h>
#include <UI/CardScroller.h>
#include <UI/Scroller.h>

using namespace fastbird;

unsigned CardItem::NextCardId = 1;

CardScroller::CardScroller()
:mNextEmptySlot(-1)
, mRatio(1)
, mNYOffset(0.f)
, mCardOffsetY(2)
, mCardSizeY(100)
, mCardSizeX(200)
{
	mUIObject = gFBEnv->pEngine->CreateUIObject(false, GetRenderTargetSize());
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	mUseScrollerV = true;
	mUIObject->SetNoDrawBackground(true);
}

CardScroller::~CardScroller()
{

}

void CardScroller::OnSizeChanged()
{
	__super::OnSizeChanged();
	
	//mWidth = 1.0f - (4 / (float)GetParentSize().x);
	const auto& size = GetFinalSize();	
	mWidth = mCardSizeX / (float)size.x;
	mHeight = mCardSizeY / (float)size.y;
}

bool CardScroller::SetProperty(UIProperty::Enum prop, const char* val)
{
	switch (prop)
	{
	case UIProperty::CARD_SIZEX:
	{
		mCardSizeX = StringConverter::parseInt(val);
		SetCardSizeX(mCardSizeX);
		return true;
	}
	case UIProperty::CARD_SIZEY:
	{
		mCardSizeY = StringConverter::parseInt(val);
		SetCardSizeY(mCardSizeY);
		return true;
	}
	case UIProperty::CARD_OFFSETY:
	{
		mCardOffsetY = StringConverter::parseInt(val);
		SetCardOffset(mCardOffsetY);
		return true;
	}
	}
	return __super::SetProperty(prop, val);
}

bool CardScroller::GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly)
{
	switch (prop)
	{
	case UIProperty::CARD_SIZEX:
	{
		if (notDefaultOnly)
		{
			if (mCardSizeX == UIProperty::GetDefaultValueInt(prop))
				return false;
		}
		auto data = StringConverter::toString(mCardSizeX);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}
	case UIProperty::CARD_SIZEY:
	{
		if (notDefaultOnly)
		{
			if (mCardSizeY == UIProperty::GetDefaultValueInt(prop))
				return false;
		}
		auto data = StringConverter::toString(mCardSizeY);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}
	case UIProperty::CARD_OFFSETY:
	{
		if (notDefaultOnly)
		{
			if (mCardOffsetY == UIProperty::GetDefaultValueInt(prop))
				return false;
		}
		auto data = StringConverter::toString(mCardOffsetY);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}
	}
	return __super::GetProperty(prop, val, bufsize, notDefaultOnly);
}

void CardScroller::SetCardSize_Offset(const Vec2& x_ratio, int offset)
{
	mWidth = x_ratio.x;
	mRatio = x_ratio.y;
	mCardOffsetY = offset;
	auto localRT = GetParentSize();
	mNYOffset = offset / (float)localRT.y;

	float iWidth = localRT.x * mWidth;
	mCardSizeX = Round(iWidth);
	float iHeight = iWidth / mRatio;
	mCardSizeY = Round(iHeight);
	mHeight = iHeight / (float)localRT.y;
}

void CardScroller::SetCardSize(const Vec2I& size)
{
	auto localRT = GetParentSize();
	mWidth = size.x / (float)localRT.x;
	mHeight = size.y / (float)localRT.y;	
	
	mCardSizeX = size.x;
	mCardSizeY = size.y;
}

void CardScroller::SetCardSizeNX(float nx)
{
	mWidth = nx;
	mCardSizeX = Round(nx * GetParentSize().x);
}

void CardScroller::SetCardSizeX(int x)
{
	mCardSizeX = x;
	const auto& rtSize = GetFinalSize();
	mWidth = x / (float)rtSize.x;
}

void CardScroller::SetCardSizeY(int y)
{
	mCardSizeY = y;
	const auto& rtSize = GetFinalSize();
	mHeight = y / (float)rtSize.y;
}

void CardScroller::SetCardOffset(int offset)
{
	mCardOffsetY = offset;
	const auto& rtSize = GetFinalSize();
	mNYOffset = offset / (float)rtSize.y;
}

CardScroller::Slot* CardScroller::GetNextCardPos(Vec2& pos)
{
	float posX = 0.f;
	float posY;
	Slot* pDestSlot = 0;
	if (mNextEmptySlot != -1)
	{
		if (mNextEmptySlot > 0)
		{
			posY = mSlots[mNextEmptySlot].mNYPos =
				mSlots[mNextEmptySlot - 1].mNYPos + mHeight + mNYOffset;
		}
		else
		{
			posY = mSlots[mNextEmptySlot].mNYPos;
		}
		mSlots[mNextEmptySlot].mOccupied = true;
		pDestSlot = &mSlots[mNextEmptySlot];
		++mNextEmptySlot;
		if (mNextEmptySlot >= (int)mSlots.size())
		{
			mNextEmptySlot = -1;
		}
	}
	else
	{
		bool empty = mSlots.empty();
		mSlots.push_back(Slot());
		mSlots.back().mOccupied = true;
		pDestSlot = &mSlots.back();
		if (empty)
		{
			posY = mSlots.back().mNYPos = 0.0f;
		}
		else
		{
			auto prev = mSlots.end() - 2;
			posY = mSlots.back().mNYPos = prev->mNYPos + mHeight + mNYOffset;
		}
	}
	pos.x = posX;
	pos.y = posY;
	return pDestSlot;
}

IWinBase* CardScroller::AddCard()
{
	Vec2 pos;
	Slot* pDestSlot = GetNextCardPos(pos);
	assert(pDestSlot);
	IWinBase* card = __super::AddChild(pos.x, pos.y, mWidth, mHeight, ComponentType::CardItem);	
	card->SetRuntimeChildRecursive(true);
	pDestSlot->mCard = (CardItem*)card;
	return card;
}

void CardScroller::AddCard(LuaObject& obj)
{
	Vec2 pos;
	Slot* pDestSlot = GetNextCardPos(pos);
	assert(pDestSlot);
	IWinBase* card = __super::AddChild(pos.x, pos.y, mWidth, mHeight, ComponentType::CardItem);
	card->SetRuntimeChildRecursive(true);
	pDestSlot->mCard = (CardItem*)card;

	card->ParseLua(obj);
}

void CardScroller::DeleteCard(IWinBase* card)
{
	Profiler p("DeleteCard");
	unsigned num = mSlots.size();
	for (unsigned i = 0; i < num; ++i)
	{
		if (mSlots[i].mCard && mSlots[i].mCard == card)
		{
			mSlots[i].mCard->SetContent(0);
			mSlots[i].mCard = 0;
			mSlots[i].mOccupied = false;
			RemoveChild(card);
			Sort();
			return;
		}
	}
}

void CardScroller::DeleteCard(unsigned cardId){
	Profiler p("DeleteCard");
	unsigned num = mSlots.size();
	for (unsigned i = 0; i < num; ++i)
	{
		if (mSlots[i].mCard && mSlots[i].mCard->GetCardId() == cardId)
		{
			mSlots[i].mCard->SetContent(0);
			mSlots[i].mCard = 0;
			mSlots[i].mOccupied = false;
			RemoveChild(mSlots[i].mCard);
			Sort();
			return;
		}
	}
}

void CardScroller::Sort()
{
	Profiler p("DeleteCard");
	std::sort(mSlots.begin(), mSlots.end());
	ArrangeSlots();
	
}

bool CardScroller::Slot::operator<(const Slot& other) const
{
	if (!mOccupied && other.mOccupied)
		return false;
	if (mOccupied && !other.mOccupied)
		return true;
	if (!mOccupied && !other.mOccupied)
		return this < &other;

	CardItem* meCard = (CardItem*)mCard;
	CardItem* otherCard = (CardItem*)other.mCard;
	assert(meCard && otherCard);
	return meCard->operator<(*otherCard);
}

void CardScroller::ArrangeSlots()
{
	Profiler p("DeleteCard");
	size_t num = mSlots.size();
	mNextEmptySlot = -1;
	for (size_t i = 0; i < num; ++i)
	{
		float y = i*(mHeight + mNYOffset);
		Slot& slot = mSlots[i];
		if (!slot.mOccupied)
		{
			if (mNextEmptySlot == -1)
				mNextEmptySlot = i;
			continue;
		}
		Vec2 curpos = slot.mCard->GetNPos();
		assert(curpos.y == slot.mNYPos);
		if (slot.mNYPos == y)
			continue;
		slot.mNYPos = y;
		curpos.y = y;
		slot.mCard->SetNPos(curpos);
	}
}



//---------------------------------------------------------------------------
CardItem::CardItem()
: mCardData(0)
{
	mCardId = NextCardId++;
	mUIObject = gFBEnv->pEngine->CreateUIObject(false, GetRenderTargetSize());
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	mUIObject->SetNoDrawBackground(true);
}
CardItem::~CardItem()
{

}

// IWinBase
void CardItem::GatherVisit(std::vector<IUIObject*>& v)
{
	/*__super::GatherVisitAlpha(v);*/
	v.push_back(mUIObject);
	__super::GatherVisit(v);
}

bool CardItem::operator< (const CardItem& other) const
{
	if (mCardData && other.mCardData)
		return *mCardData < *other.mCardData;

	return false;
}

void CardItem::OnStartUpdate(float elapsedTime)
{
	bool ae = mSimplePosAnimEnabled;
	__super::OnStartUpdate(elapsedTime);
}