#include <UI/StdAfx.h>
#include <UI/CardScroller.h>
#include <UI/Scroller.h>

using namespace fastbird;

CardScroller::CardScroller()
:mNextEmptySlot(-1)
, mRatio(-1)
{
	mUIObject = IUIObject::CreateUIObject(false);
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	mUIObject->GetMaterial()->SetDiffuseColor(Vec4(0.1f, 0.1f, 0, 1));
}

CardScroller::~CardScroller()
{

}

void CardScroller::GatherVisit(std::vector<IUIObject*>& v)
{
	for each(IWinBase* win in mChildren)
	{
		win->SetScissorRect(true, this->GetRegion());
	}
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

IWinBase* CardScroller::AddCard()
{
	assert(mRatio != -1);
	float posX = 0.f;
	float posY;
	if (mNextEmptySlot!=-1)
	{
		posY = mSlots[mNextEmptySlot].mNYPos;
		mSlots[mNextEmptySlot].mOccupied = true;
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
	return __super::AddChild(posX, posY, mWidth, mHeight, ComponentType::CardItem);
}

void CardScroller::Sort()
{
	mChildren.sort();
	ArrangeSlots();
	
}

void CardScroller::ArrangeSlots()
{
	int i = 0;
	for each(WinBase* it in mChildren)
	{
		if (it->GetType() == ComponentType::CardItem)
		{
			Vec2 curPos = it->GetNPos();
			if ( curPos.y != mSlots[i].mNYPos)
			{
				it->PosAnimationTo(Vec2(curPos.x, mSlots[i].mNYPos), 0.01f);
			}
			mSlots[i].mOccupied = true;
			++i;
		}
	}
	if (i < (int)mSlots.size())
	{
		mNextEmptySlot = i;
		size_t numSlots = mSlots.size();
		for (; i < (int)numSlots; i++)
		{
			mSlots[i].mOccupied = false;
		}
	}
}



//---------------------------------------------------------------------------
CardItem::CardItem()
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
	bool ae = mAnimationEnabled;
	__super::OnStartUpdate(elapsedTime);

	if (ae)
	{
		SetScissorRect(true, mParent->GetRegion());
	}
}