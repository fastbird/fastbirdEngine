#include <UI/StdAfx.h>
#include <UI/CardScroller.h>
#include <UI/Scroller.h>

using namespace fastbird;

CardScroller::CardScroller()
:mNextEmptySlot(-1)
, mRatio(-1)
, mNYOffset(0.f)
{
	mUIObject = IUIObject::CreateUIObject(false);
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	mUIObject->GetMaterial()->SetDiffuseColor(Vec4(0.1f, 0.1f, 0, 1));
	mUseScrollerV = true;
}

CardScroller::~CardScroller()
{

}

void CardScroller::GatherVisit(std::vector<IUIObject*>& v)
{
	/*__super::GatherVisitAlpha(v);*/
	__super::GatherVisit(v);
}

void CardScroller::SetCardSize_Offset(const Vec2& x_ratio, int offset)
{
	mWidth = x_ratio.x;
	mRatio = x_ratio.y;
	mNYOffset = this->PixelToLocalNHeight(offset);

	Vec2 worldSize = ConvertChildSizeToWorldCoord(Vec2(mWidth, mWidth));
	float iWidth = gEnv->pRenderer->GetWidth() * worldSize.x;
	float iHeight = iWidth / mRatio;
	mHeight = iHeight / (gEnv->pRenderer->GetHeight() * mWNSize.y);
}

void CardScroller::SetCardSize(const Vec2I& size)
{
	if (mParent)
	{
		mWidth = mParent->PixelToLocalNWidth(size.x);
		mHeight = mParent->PixelToLocalNHeight(size.y);
	}
	else
	{
		mWidth = size.x / (float)gEnv->pRenderer->GetWidth();
		mHeight = size.y / (float)gEnv->pRenderer->GetHeight();
	}
	
}

void CardScroller::SetCardSizeNX(float nx)
{
	mWidth = nx;
}

void CardScroller::SetCardSizeY(int y)
{
	mHeight = y / (gEnv->pRenderer->GetHeight() * mWNSize.y);
}

void CardScroller::SetCardOffset(int offset)
{
	mNYOffset = this->PixelToLocalNHeight(offset);
}

IWinBase* CardScroller::AddCard()
{
	float posX = 0.f;
	float posY;
	Slot* pDestSlot = 0;
	if (mNextEmptySlot!=-1)
	{
		posY = mSlots[mNextEmptySlot].mNYPos;
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
	IWinBase* card = __super::AddChild(posX, posY, mWidth, mHeight, ComponentType::CardItem);
	assert(pDestSlot);
	pDestSlot->mCard = card;
	return card;
}

void CardScroller::DeleteCard(IWinBase* card)
{
	Profiler p("DeleteCard");
	unsigned num = mSlots.size();
	for (unsigned i = 0; i < num; ++i)
	{
		if (mSlots[i].mCard == card)
		{
			mSlots[i].mCard = 0;
			mSlots[i].mOccupied = false;
			RemoveChild(card);
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
	mUIObject = IUIObject::CreateUIObject(false);
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	mUIObject->GetMaterial()->SetDiffuseColor(Vec4(1, 0, 0, 1));
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