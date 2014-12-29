#include <UI/StdAfx.h>
#include <UI/WinBase.h>
#include <UI/IUIManager.h>
#include <UI/Container.h>
#include <UI/UIAnimation.h>
#include <UI/ImageBox.h>
#include <CommonLib/StringUtils.h>

namespace fastbird
{
const float WinBase::LEFT_GAP = 0.001f;
HCURSOR WinBase::mCursorOver = 0;
const float WinBase::NotDefined = 12345.6f;
const char* WinBase::sShowAnim = "_ShowAnim";
const char* WinBase::sHideAnim = "_HideAnim";
const float WinBase::sFadeInOutTime = 0.2f;

WinBase::WinBase()
: mVisible(false)
, mAlignH(ALIGNH::LEFT)
, mAlignV(ALIGNV::TOP)
, mParent(0)
, mNPosAligned(0, 0)
, mMouseIn(false)
, mMouseInPrev(false)
, mNext(0)
, mPrev(0)
, mTextColor(1.f, 1.f, 1.f, 1.f)
, mTextColorHover(1.f, 1.0f, 1.f, 1.f)
, mTextColorDown(1.0f, 1.0f, 1.0f, 1.f)
, mTextAlignH(ALIGNH::LEFT)
, mTextAlignV(ALIGNV::MIDDLE)
, mMatchSize(false)
, mSize(10, 10)
, mTextSize(30.0f)
, mFixedTextSize(false)
, mWNPosOffset(0, 0)
, mNPosOffset(0, 0)
, mMouseDragStartInHere(false)
, mDestNPos(0, 0)
, mSimplePosAnimEnabled(false)
, mAnimationSpeed(0.f)
, mAspectRatio(1.0)
, mNPos(0, 0)
, mUIObject(0)
, mNoMouseEvent(false)
, mUseScissor(true)
, mUseAbsoluteXPos(false)
, mUseAbsoluteYPos(false)
, mUseAbsoluteXSize(false)
, mUseAbsoluteYSize(false)
, mAbsTempLock(false)
, mAbsOffset(0, 0)
, mSizeMod(0, 0)
, mManualParent(0)
, mCustomContent(0)
, mLockTextSizeChange(false)
, mStopScissorParent(false)
, mSpecialOrder(0)
, mTextWidth(0)
, mNumTextLines(1)
, mInheritVisibleTrue(true)
, mInvalidateMouse(true)
, mNSize(NotDefined, NotDefined)
, mWNPos(NotDefined, NotDefined)
, mWNSize(NotDefined, NotDefined)
, mShowAnimation(false)
, mHideAnimation(false)
, mHideCounter(0), mPivot(false)
, mRender3D(false), mTextGap(0, 0)
{
}

WinBase::~WinBase()
{
	FB_FOREACH(it, mAnimations)
	{
		FB_SAFE_DEL(it->second);
	}

	for (auto ib : mBorders)
	{
		IUIManager::GetUIManager().DeleteComponent(ib);
	}
	
	FB_RELEASE(mUIObject);
}

//---------------------------------------------------------------------------
void WinBase::SetName(const char* name)
{
	mName = name;
}

const char* WinBase::GetName() const
{
	return mName.c_str();
}

void WinBase::ClearName()
{
	mName.clear();
}

//---------------------------------------------------------------------------
void WinBase::SetSize(const fastbird::Vec2I& size)
{
	mSize = size;
	mUseAbsoluteXSize = true;
	mUseAbsoluteYSize = true;
	auto rtSize = GetRenderTargetSize();
	
	if (mSize.x < 0)
		mSize.x = rtSize.x + mSize.x;

	if (mSize.y < 0)
		mSize.y = rtSize.y + mSize.y;

	if (mParent)
	{
		mNSize = mParent->PixelToLocalNSize(mSize);
	}
	else
	{
		mNSize = Vec2(mSize.x / (float)rtSize.x, mSize.y / (float)rtSize.y);
	}

	UpdateWorldSize(true);
	UpdateAlignedPos();
	OnSizeChanged();
}

//---------------------------------------------------------------------------
void WinBase::SetSizeModificator(const Vec2I& sizemod)
{
	Vec2 nsizeMod;
	mSizeMod = sizemod;
	auto rtSize = GetRenderTargetSize();
	if(mParent)
	{
		nsizeMod = mParent->PixelToLocalNSize(mSizeMod);
	}
	else
	{
		nsizeMod = Vec2(mSizeMod.x / (float)rtSize.x, mSizeMod.y / (float)rtSize.y);
	}
	SetNSize(mNSize + nsizeMod);
}

void WinBase::SetSizeX(int x)
{
	mUseAbsoluteXSize = true;
	mSize.x = x;
	auto rtSize = GetRenderTargetSize();
	if (mSize.x < 0)
		mSize.x = rtSize.x + mSize.x;

	if (mParent)
	{
		mNSize.x = mParent->PixelToLocalNWidth(mSize.x);
	}
	else
	{
		mNSize.x = mSize.x / (float)rtSize.x;
	}
	UpdateWorldSize(true);
	UpdateAlignedPos();
	OnSizeChanged();
}
void WinBase::SetSizeY(int y)
{
	mUseAbsoluteYSize = true;
	mSize.y = y;
	auto rtSize = GetRenderTargetSize();
	if (mSize.y < 0)
		mSize.y = rtSize.y + mSize.y;

	if (mParent)
	{
		mNSize.y = mParent->PixelToLocalNHeight(mSize.y);
	}
	else
	{
		mNSize.y = mSize.y / (float)rtSize.y;
	}
	UpdateWorldSize(true);
	UpdateAlignedPos();
	OnSizeChanged();
}

void WinBase::SetNSize(const fastbird::Vec2& size) // normalized size (0.0~1.0)
{
	if (!mAbsTempLock)
	{
		mUseAbsoluteXSize = false;
		mUseAbsoluteYSize = false;
	}
	mNSize = size;
	if (mParent)
	{
		mSize = mParent->LocalNSizeToPixel(mNSize);
	}
	else
	{
		auto rtSize = GetRenderTargetSize();
		mSize = Vec2I(Round(mNSize.x * rtSize.x), Round(mNSize.y*rtSize.y));
	}
	
	UpdateWorldSize(true);
	UpdateAlignedPos();
	OnSizeChanged();
}

void WinBase::SetNSizeX(float x) // normalized size (0.0~1.0)
{
	mUseAbsoluteXSize = false;
	mNSize.x = x;
	if (mParent)
		mSize.x = mParent->LocalNWidthToPixel(x);
	else
	{
		auto rtSize = GetRenderTargetSize();
		mSize.x = Round(x * rtSize.x);
	}

	UpdateWorldSize(true);
	UpdateAlignedPos();
	OnSizeChanged();
}

void WinBase::SetNSizeY(float y) // normalized size (0.0~1.0)
{
	if (mNSize.y == y)
		return;

	mUseAbsoluteYSize = false;
	mNSize.y = y;
	if (mParent)
		mSize.y = mParent->LocalNHeightToPixel(y);
	else
	{
		auto rtSize = GetRenderTargetSize();
		mSize.y = Round(y * rtSize.y);
	}

	UpdateWorldSize(true);
	UpdateAlignedPos();
	OnSizeChanged();
}

void WinBase::SetWNPos(const fastbird::Vec2& wnPos)
{
	Vec2 parentPos = wnPos;
	if (mParent)
		parentPos = mParent->ConvertWorldPosToParentCoord(parentPos);

	SetNPos(parentPos);
}

void WinBase::SetWNSize(const fastbird::Vec2& size)
{
	Vec2 parentSize = size;
	if (mParent)
		parentSize = mParent->ConvertWorldSizeToParentCoord(parentSize);
	SetNSize(parentSize);
}

// world coordinate
void WinBase::UpdateWorldSize(bool settingSize)
{
	if (!settingSize)
	{
		if (mUseAbsoluteXSize)
		{
			SetSizeX(mSize.x);
		}
		if (mUseAbsoluteYSize)
		{
			SetSizeY(mSize.y);
		}

		if (mParent)
		{
			mSize = mParent->LocalNSizeToPixel(mNSize);
		}
		else
		{
			auto rtSize = GetRenderTargetSize();
			mSize = Vec2I(Round(mNSize.x * rtSize.x), Round(mNSize.y*rtSize.y));
		}
	}
	
	mWNSize = mNSize;
	if (mParent)
		mWNSize = mParent->ConvertChildSizeToWorldCoord(mWNSize);
}

//---------------------------------------------------------------------------
void WinBase::SetPos(const fastbird::Vec2I& pos)
{
	mUseAbsoluteXPos = true;
	mUseAbsoluteYPos = true;
	mPos = pos;
	auto rtSize = GetRenderTargetSize();
	if (mPos.x < 0)
		mPos.x = rtSize.x + mPos.x;
	if (mPos.y < 0)
		mPos.y = rtSize.y + mPos.y;

	if (mParent)
	{
		mNPos = mParent->PixelToLocalNPos(mPos);
	}
	else
	{
		mNPos = Vec2(mPos.x / (float)rtSize.x, mPos.y / (float)rtSize.y);
	}
	UpdateWorldPos(true);
}

void WinBase::SetPosX(int x)
{
	mUseAbsoluteXPos = true;
	mPos.x = x;
	auto rtSize = GetRenderTargetSize();

	if (mPos.x < 0)
	{
		if (mParent)
			mPos.x = mParent->GetSize().x + mPos.x;
		else
			mPos.x = rtSize.x + mPos.x;
	}
	if (mParent)
	{
		mNPos.x = mParent->PixelToLocalNPosX(mPos.x);
	}
	else
	{
		mNPos.x = mPos.x / (float)rtSize.x;
	}
	UpdateWorldPos(true);
}

void WinBase::SetPosY(int y)
{
	mUseAbsoluteYPos = true;
	mPos.y = y;
	auto rtSize = GetRenderTargetSize();
	if (mPos.y < 0)
	{
		if (mParent)
			mPos.y = mParent->GetSize().y + mPos.y;
		else
			mPos.y = rtSize.y + mPos.y;
	}
	if (mParent)
	{
		mNPos.y = mParent->PixelToLocalNPosY(mPos.y);
	}
	else
	{
		mNPos.y = mPos.y / (float)rtSize.y;
	}
	UpdateWorldPos(true);
}

void WinBase::SetInitialOffset(Vec2I offset)
{
	Vec2 noffset;
	mAbsOffset = offset;
	if (mParent)
	{
		noffset = mParent->PixelToLocalNPos(offset);
	}
	else
	{
		auto rtSize = GetRenderTargetSize();
		noffset = Vec2(offset.x / (float)rtSize.x, offset.y / (float)rtSize.y);
	}
	mAbsTempLock = true;
	//SetNPos(mNPos + noffset);
	mAbsTempLock = false;
}


void WinBase::SetNPos(const fastbird::Vec2& pos) // normalized pos (0.0~1.0)
{	
	if (!mAbsTempLock)
	{
		mUseAbsoluteXPos = false;
		mUseAbsoluteYPos = false;
	}

	mNPos = pos;
	if (mParent)
		mPos = mParent->LocalNSizeToPixel(pos);
	else
	{
		auto rtSize = GetRenderTargetSize();
		mPos = Vec2I((int)(mNPos.x * rtSize.x), (int)(mNPos.y * rtSize.y));
	}
	UpdateWorldPos(true);
}

void WinBase::SetNPosX(float x)
{
	mUseAbsoluteXPos = false;
	mNPos.x = x;
	if (mParent)
	{
		mPos.x = mParent->LocalNWidthToPixel(x);
	}
	else
	{
		auto rtSize = GetRenderTargetSize();
		mPos.x = (int)(x * rtSize.x);
	}
	
	UpdateWorldPos(true);

}
void WinBase::SetNPosY(float y)
{
	mUseAbsoluteYPos = false;
	mNPos.y = y;
	if (mParent)
	{
		mPos.y = mParent->LocalNHeightToPixel(y);
	}
	else
	{
		auto rtSize = GetRenderTargetSize();
		mPos.y = (int)(y*rtSize.y);
	}
	
	UpdateWorldPos(true);
}

void WinBase::UpdateWorldPos(bool settingPos)
{

	if (!settingPos)
	{
		// not setting position.
		// maybe resolution is changed or parent pos changed.
		if (mUseAbsoluteXPos)
		{
			SetPosX(mPos.x);
		}
		if (mUseAbsoluteYPos)
		{
			SetPosY(mPos.y);
		}

		mAbsTempLock = true;
		SetNPos(mNPos);
		mAbsTempLock = false;
		return;
	}

	UpdateAlignedPos();
	// mNPos and mPos has updated value.
	mWNPos = mNPosAligned;
	if (mParent)
		mWNPos = mParent->ConvertChildPosToWorldCoord(mNPosAligned);

	/*if (GetType() != ComponentType::Scroller)
	{
		if (mParent && mParent->HasVScroll())
		{
			mWNPosOffset = mParent->GetScrollOffset();
		}
	}*/

	OnPosChanged();
}

//---------------------------------------------------------------------------
fastbird::Vec2I WinBase::ConvertToScreen(const fastbird::Vec2 npos) const
{
	auto rtSize = GetRenderTargetSize();
	Vec2I screenPos = { (int)(npos.x * rtSize.x), (int)((npos.y) * rtSize.y) };

	return screenPos;
}


fastbird::Vec2 WinBase::ConvertToNormalized(const fastbird::Vec2I pos) const
{
	auto rtSize = GetRenderTargetSize();
	Vec2 npos = { pos.x / (float)rtSize.x, pos.y / (float)rtSize.y };
	return npos;
}

//---------------------------------------------------------------------------
void WinBase::SetAlign(ALIGNH::Enum h, ALIGNV::Enum v)
{
	mAlignH = h;
	mAlignV = v;
	UpdateWorldPos(true);
}

void WinBase::UpdateAlignedPos()
{
	Vec2 noffset(0, 0);
	if (mAbsOffset != Vec2I::ZERO)
	{
		if (mParent)
		{
			noffset = mParent->PixelToLocalNPos(mAbsOffset);
		}
		else
		{
			auto rtSize = GetRenderTargetSize();
			noffset = Vec2(mAbsOffset.x / (float)rtSize.x, mAbsOffset.y / (float)rtSize.y);
		}
	}

	mNPosAligned = mNPos + noffset;
	switch(mAlignH)
	{
	case ALIGNH::LEFT: /*nothing todo*/ break;
	case ALIGNH::CENTER: mNPosAligned.x -= mNSize.x / 2; break;
	case ALIGNH::RIGHT: mNPosAligned.x -= mNSize.x; break;
	}
	switch(mAlignV)
	{
	case ALIGNV::TOP: /*nothing todo*/break;
	case ALIGNV::MIDDLE: mNPosAligned.y -= mNSize.y / 2;break;
	case ALIGNV::BOTTOM: mNPosAligned.y -= mNSize.y; break;
	}
}

//---------------------------------------------------------------------------
bool WinBase::SetVisible(bool show)
{
	if (mVisible == show && mHideCounter==0.f)
		return false;

	if (mHideCounter != 0.f)
	{
		if (show)
		{
			mHideCounter = 0.f;
			if (mHideAnimation)
			{
				mAnimations[sHideAnim]->SetActivated(false);
			}
		}
		else
		{
			return false;
		}
	}

	mVisible = show;

	if (mVisible)
	{
	
		if (mShowAnimation)
		{
			mAnimations[sShowAnim]->SetActivated(true);
		}

		if (!mBorders.empty())
		{
			for (auto var : mBorders)
			{
				var->SetVisible(show);
			}
		}

		OnEvent(IEventHandler::EVENT_ON_VISIBLE);
	}
	else
	{
		if (mHideAnimation)
		{
			auto anim = mAnimations[sHideAnim];
			
			mHideCounter = anim->GetLength();
			anim->SetActivated(true);
			mVisible = true;
			if (mShowAnimation)
			{
				mAnimations[sShowAnim]->SetActivated(false);
			}
			return false;
		}

		if (!mBorders.empty())
		{
			for (auto var : mBorders)
			{
				var->SetVisible(show);
			}
		}
		OnEvent(IEventHandler::EVENT_ON_HIDE);
	}

	IUIManager::GetUIManager().DirtyRenderList();
	return true;
}

void WinBase::SetVisibleInternal(bool visible)
{
	assert(!visible); // using only for hiding for now.
	mVisible = visible;
	IUIManager::GetUIManager().DirtyRenderList();
	if (!visible)
		OnEvent(IEventHandler::EVENT_ON_HIDE);
}

bool WinBase::GetVisible() const
{
	return mVisible;
}


bool WinBase::GetFocus(bool includeChildren /*= false*/) const
{
	return IUIManager::GetUIManager().IsFocused(this);
}

//---------------------------------------------------------------------------
void WinBase::SetParent(Container* parent)
{
	mParent = parent;
}

void WinBase::SetManualParent(WinBase* parent)
{
	mManualParent = parent;
}
//---------------------------------------------------------------------------
bool WinBase::IsIn(IMouse* mouse)
{
	assert(mouse);
	Vec2 mouseNormpos = mouse->GetNPos();
	long x, y;
	mouse->GetPos(x, y);
	float wx = mWNPos.x + mWNPosOffset.x;
	float wy = mWNPos.y + mWNPosOffset.y;
	bool inScissor = true;
	if (mUseScissor)
	{
		const RECT& rect = GetScissorRegion();
		inScissor = !(x < rect.left || x > rect.right || y < rect.top || y > rect.bottom);

	}
	return inScissor && !(mouseNormpos.x < wx ||
		mouseNormpos.x > wx + mWNSize.x ||
		mouseNormpos.y < wy ||
		mouseNormpos.y > wy + mWNSize.y);
}

//---------------------------------------------------------------------------
bool WinBase::OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard)
{
	if (!mVisible)
		return false;

	if (mNoMouseEvent)
	{
		return false;
	}

	mMouseIn = false;
	Vec2 mousepos = mouse->GetNPos();
	if (mouse->IsValid() && !mNoMouseEvent)
	{
		// hit test
		mMouseIn = mMouseDragStartInHere || IsIn(mouse);
		if (mMouseIn)
		{
			if ((mouse->IsLButtonDown() && !mouse->IsLButtonDownPrev() && mUIObject && mouse->IsDragStartIn(mUIObject->GetRegion()))
				|| (mMouseDragStartInHere && mouse->IsLButtonDown())
				)
			{
				mMouseDragStartInHere = true;
			}
			else
			{
				mMouseDragStartInHere = false;
			}
			if (mouse->IsLButtonDown() && mMouseDragStartInHere)
			{
				long x,  y;
				mouse->GetDeltaXY(x, y);
				if (x != 0 || y != 0)
				{
					if (OnEvent(IEventHandler::EVENT_MOUSE_DRAG))
						mouse->Invalidate();
				}
				else
				{
					if (OnEvent(IEventHandler::EVENT_MOUSE_DOWN))
						mouse->Invalidate();
				}
			}
			else if (mMouseInPrev)
			{
				if (OnEvent(IEventHandler::EVENT_MOUSE_HOVER))
					mouse->Invalidate();
				ToolTipEvent(IEventHandler::EVENT_MOUSE_HOVER, mousepos);
			}
			else if (mMouseIn)
			{
				if (OnEvent(IEventHandler::EVENT_MOUSE_IN))
					mouse->Invalidate();
				ToolTipEvent(IEventHandler::EVENT_MOUSE_IN, mousepos);
			}

			if (mouse->IsLButtonClicked())
			{
				if (OnEvent(EVENT_MOUSE_LEFT_CLICK))
				{
					mouse->Invalidate(GetType() == ComponentType::Button ? true : false);
				}
			}
			else if (mouse->IsLButtonDoubleClicked())
			{
				if (OnEvent(EVENT_MOUSE_LEFT_DOUBLE_CLICK))
				{
					mouse->Invalidate();
					IUIManager::GetUIManager().CleanTooltip();
				}

			}
			else if (mouse->IsRButtonClicked())
			{
				if (OnEvent(EVENT_MOUSE_RIGHT_CLICK))
					mouse->Invalidate();
			}
			if (mInvalidateMouse)
				mouse->Invalidate();
		}		
		else if (mMouseInPrev)
		{
			if (OnEvent(IEventHandler::EVENT_MOUSE_OUT))
				mouse->Invalidate();
			ToolTipEvent(IEventHandler::EVENT_MOUSE_OUT, mousepos);
		}

		mMouseInPrev = mMouseIn;
	}
	else if (mMouseInPrev)
	{
		if (OnEvent(IEventHandler::EVENT_MOUSE_OUT))
			mouse->Invalidate();
		ToolTipEvent(IEventHandler::EVENT_MOUSE_OUT, mousepos);
		mMouseInPrev = false;
	}

	if (!GetFocus())
		return mMouseIn;

	if (keyboard->IsValid())
	{
		char c = (char)keyboard->GetChar();

		switch(c)
		{
		case VK_RETURN:
			{
				OnEvent(EVENT_ENTER);
				keyboard->Invalidate();
			}
			break;
		}
	}

	return mMouseIn;
}

IWinBase* WinBase::FocusTest(IMouse* mouse)
{
	IWinBase* ret = 0;
	IsIn(mouse) ? ret = this : ret = 0;
	return ret;
}

void WinBase::SetNext(IWinBase* next)
{
	mNext = next;
	if (mNext && mNext->GetPrev() != this)
	{
		mNext->SetPrev(this);
	}
}

void WinBase::SetPrev(IWinBase* prev)
{
	mPrev = prev;
	if (mPrev && mPrev->GetNext() != this)
	{
		mPrev->SetNext(this);
	}
}

IWinBase* WinBase::GetNext() const
{
	return mNext;
}

IWinBase* WinBase::GetPrev() const
{
	return mPrev;
}

// static
void WinBase::InitMouseCursor()
{
	mCursorOver = LoadCursor(0, IDC_HAND);
}
// static
void WinBase::FinalizeMouseCursor()
{

}
//static 
HCURSOR WinBase::GetMouseCursorOver()
{
	return mCursorOver;
}

void WinBase::AlignText()
{
	if (mTextw.empty())
		return;
	
	float nwidth = ConvertToNormalized(Vec2I(mTextWidth, mTextWidth)).x;
	if (mMatchSize && mTextWidth != 0 && !mLockTextSizeChange)
	{ 
		mLockTextSizeChange = true;
		SetSize(Vec2I((int)(mTextWidth + 4), mSize.y));
		mLockTextSizeChange = false;
		RefreshScissorRects();
	}
	if (mUIObject)
	{
		Vec2 startPos = GetFinalPos();
		auto rtSize = GetRenderTargetSize();
		switch(mTextAlignH)
		{
		case ALIGNH::LEFT:
			{
							 startPos.x += LEFT_GAP + mTextGap.x / (float)rtSize.x;
			}
			break;

		case ALIGNH::CENTER:
			{
							   startPos.x += mWNSize.x*.5f - nwidth*.5f + mTextGap.x / (float)rtSize.x;
			}
			break;

		case ALIGNH::RIGHT:
			{
							  startPos.x += mWNSize.x - nwidth - mTextGap.y / (float)rtSize.x;
			}
			break;
		}
		switch (mTextAlignV)
		{
		case ALIGNV::TOP:
		{
							startPos.y += (mTextSize / (float)rtSize.y);
		}
			break;
		case ALIGNV::MIDDLE:
		{
							   startPos.y += mWNSize.y*.5f  + 
								   (
									   ((mTextSize * 0.5f) - (mTextSize * (mNumTextLines-1) * 0.5f))
										/ (float)rtSize.y
								   );
		}
			break;
		case ALIGNV::BOTTOM:
		{
							   startPos.y += mWNSize.y - GetTextBottomGap();// -mTextSize*(mNumTextLines - 1);
		}
			break;
		}
		mUIObject->SetTextStartNPos(startPos);
	}
}

void WinBase::OnPosChanged()
{
	if (mWNPos.x == NotDefined || mWNPos.y == NotDefined)
		return;
	if (mUIObject)
	{
		mUIObject->SetNPos(GetFinalPos());
	}
	AlignText();
	if (mParent)
	{
		mParent->SetChildrenPosSizeChanged();		
	}
		
	RefreshBorder();
	RefreshScissorRects();
}

void WinBase::CalcTextWidth()
{
	if (mTextw.empty())
		return;
	IFont* pFont = gEnv->pRenderer->GetFont();
	pFont->SetHeight(mTextSize);
	mTextWidth = (int)gEnv->pRenderer->GetFont()->GetTextWidth(
		(const char*)mTextw.c_str(), mTextw.size() * 2);
	pFont->SetBackToOrigHeight();
}

void WinBase::OnSizeChanged()
{
	if (mNSize.x == NotDefined || mNSize.y == NotDefined)
		return;
	if (mUIObject)
	{
		mUIObject->SetNSize(mWNSize);
		const auto& region = mUIObject->GetRegion();
		assert(region.right - region.left == mSize.x);
		assert(region.bottom - region.top == mSize.y);

		if (!mFixedTextSize)
		{
			const RECT& region = mUIObject->GetRegion();
			mTextSize = (float)(region.bottom - region.top);
			CalcTextWidth();
			AlignText();
			mUIObject->SetTextSize(mTextSize);
		}
	}

	// we need this for alignment
	UpdateWorldPos(true);
	RefreshBorder();
}

void WinBase::SetTextColor(const Color& c)
{
	mTextColor = c;
	if (mUIObject)
		mUIObject->SetTextColor(mTextColor);
}

void WinBase::SetText(const wchar_t* szText)
{
	mTextw = szText;
	if (mUIObject)
		mUIObject->SetText(szText);

	CalcTextWidth();

	AlignText();
}

const wchar_t* WinBase::GetText() const
{
	return mTextw.c_str();
}

IUIAnimation* WinBase::GetUIAnimation(const char* name)
{
	auto itfind = mAnimations.Find(name);
	if (itfind == mAnimations.end())
	{
		mAnimations[name] = FB_NEW(UIAnimation);
		mAnimations[name]->SetName(name);
		mAnimations[name]->SetTargetUI(this);
	}
	return mAnimations[name];
}

void WinBase::ClearAnimationResult()
{
	SetAnimNPosOffset(Vec2(0, 0));
}

bool WinBase::SetProperty(UIProperty::Enum prop, const char* val)
{
	assert(val);
	switch(prop)
	{
	case UIProperty::POS:
	{
							Vec2I pos = StringConverter::parseVec2I(val);
							SetPos(pos);
							return true;
	}
	case UIProperty::NPOS:
	{
							 Vec2 npos = StringConverter::parseVec2(val);
							 SetNPos(npos);
							 return true;
	}
	case UIProperty::NPOSX:
	{
							  float x = StringConverter::parseReal(val);
							  SetNPosX(x);
							  return true;
	}
	case UIProperty::NPOSY:
	{
							  float y = StringConverter::parseReal(val);
							  SetNPosY(y);
							  return true;
	}
	case UIProperty::OFFSETX:
	{
								mAbsOffset.x = StringConverter::parseInt(val);
								UpdateWorldPos();
							  return true;
	}
	case UIProperty::OFFSETY:
	{
							  mAbsOffset.y = StringConverter::parseInt(val);
							  UpdateWorldPos();
							  return true;
	}
	case UIProperty::BACK_COLOR:
		{
									if (mUIObject)
									{
										Color color;
										color = StringConverter::parseVec4(val);
										mUIObject->GetMaterial()->SetDiffuseColor(color.GetVec4());
										return true;
									}
									break;

		}

	case UIProperty::TEXT_SIZE:
		{
								   if (mUIObject)
								   {
									   mTextSize = StringConverter::parseReal(val, 20.0f);
									   CalcTextWidth();
									   mUIObject->SetTextSize(mTextSize);
									   mFixedTextSize = true;
									   return true;
								   }
								   break;
		}

	case UIProperty::FIXED_TEXT_SIZE:
		{
										 mFixedTextSize = stricmp("true", val) == 0;
										 return true;
		}

	case UIProperty::TEXT_ALIGN:
		{
								   mTextAlignH = ALIGNH::ConvertToEnum(val);
								   AlignText();
									return true;
		}
	case UIProperty::TEXT_VALIGN:
	{
									mTextAlignV = ALIGNV::ConvertToEnum(val);
									AlignText();
								   return true;
	}
	case UIProperty::TEXT_LEFT_GAP:
	{
									  mTextGap.x = StringConverter::parseInt(val);
									  AlignText();
									  return true;
	}
	case UIProperty::TEXT_RIGHT_GAP:
	{
									   mTextGap.y = StringConverter::parseInt(val);
									   AlignText();
									  return true;
	}
	case UIProperty::MATCH_SIZE:
		{
									mMatchSize = stricmp("true", val) == 0;
									return true;
		}
	case UIProperty::NO_BACKGROUND:
		{
									   if (stricmp("true", val) == 0)
									   {
										   if (mUIObject)
											   mUIObject->SetNoDrawBackground(true);
									   }
									   else
									   {
										   if (mUIObject)
											   mUIObject->SetNoDrawBackground(false);
									   }
									   return true;
		}
	case UIProperty::TEXT_COLOR:
		{
									mTextColor = StringConverter::parseVec4(val);
									mUIObject->SetTextColor(mTextColor);
									return true;
		}
	case UIProperty::TEXT_COLOR_HOVER:
		{
										  mTextColorHover = StringConverter::parseVec4(val);
										  return true;
		}
	case UIProperty::TEXT_COLOR_DOWN:
		{
										 mTextColorDown = StringConverter::parseVec4(val);
										 return true;
		}
		case UIProperty::ALIGNH:
		{
								   ALIGNH::Enum align = ALIGNH::ConvertToEnum(val);
								   SetAlign(align, mAlignV);
								   return true;
		}
		case UIProperty::ALIGNV:
		{
								   ALIGNV::Enum align = ALIGNV::ConvertToEnum(val);
								   SetAlign(mAlignH, align);
								   return true;
		}
		case UIProperty::TEXT:
		{
								 assert(val);
								 SetText(AnsiToWide(val, strlen(val)));
								 return true;
		}
		case UIProperty::ALPHA:
		{
								  bool b = StringConverter::parseBool(val);
								  if (mUIObject)
									  mUIObject->SetAlphaBlending(b);
								  IUIManager::GetUIManager().DirtyRenderList();
								  return true;
		}

		case UIProperty::TOOLTIP:
		{
									assert(val);
									mTooltipText = AnsiToWide(val, strlen(val));
									return true;
		}

		case UIProperty::NO_MOUSE_EVENT:
		{
										   mNoMouseEvent = StringConverter::parseBool(val);
										   return true;
		}

		case UIProperty::USE_SCISSOR:
		{
										mUseScissor = StringConverter::parseBool(val);
										if (!mUseScissor)
										{
											mUIObject->SetUseScissor(false, RECT());
										}
										else if (mParent && mUIObject)
											mUIObject->SetUseScissor(mUseScissor, mParent->GetRegion());
										return true;
		}

		case UIProperty::USE_BORDER:
		{
									   bool use = StringConverter::parseBool(val);
									   SetUseBorder(use);
									   return true;
		}

		case UIProperty::SPECIAL_ORDER:
		{
										  mSpecialOrder = StringConverter::parseInt(val);
										  mUIObject->SetSpecialOrder(mSpecialOrder);
										return true;
		}

		case UIProperty::INHERIT_VISIBLE_TRUE:
		{
									mInheritVisibleTrue = StringConverter::parseBool(val);
									return true;
		}

		case UIProperty::VISIBLE:
		{
									SetVisible(StringConverter::parseBool(val));
									return true;
		}

		case UIProperty::SHOW_ANIMATION:
		{
										   auto anim = GetUIAnimation(sShowAnim);
										   anim->ClearData();
										   anim->AddScale(0.0f, Vec2(0.001f, 0.001f));
										   anim->AddScale(sFadeInOutTime, Vec2(1.0f, 1.0f));
										   anim->SetLength(sFadeInOutTime);
										   anim->SetLoop(false);
										   mShowAnimation = true;
										   mPivot = true;
										   return true;
		}

		case UIProperty::HIDE_ANIMATION:
		{
										   auto anim = GetUIAnimation(sHideAnim);
										   anim->ClearData();
										   anim->AddScale(0.0f, Vec2(1.0f, 1.0f));
										   anim->AddScale(sFadeInOutTime, Vec2(0.001f, 0.001f));
										   anim->SetLength(sFadeInOutTime);
										   anim->SetLoop(false);
										   mHideAnimation = true;
										   mPivot = true;
										   return true;
		}

		case UIProperty::ENABLED:
		{
								   SetEnable(StringConverter::parseBool(val));
								   return true;
		}
	}

	assert(0 && "Not processed property found");
	return false;
}

void WinBase::SetUseBorder(bool use)
{
	if (use && mBorders.empty())
	{
		ImageBox* T = (ImageBox*)IUIManager::GetUIManager().CreateComponent(ComponentType::ImageBox);
		mBorders.push_back(T);
		T->SetRender3D(mRender3D, GetRenderTargetSize());
		T->SetManualParent(this);
		T->SetSizeY(3);
		T->SetTextureAtlasRegion("es/textures/ui.xml", "Box_T");	
		

		ImageBox* L = (ImageBox*)IUIManager::GetUIManager().CreateComponent(ComponentType::ImageBox);
		mBorders.push_back(L);
		L->SetRender3D(mRender3D, GetRenderTargetSize());
		L->SetManualParent(this);
		L->SetSizeX(3);		
		L->SetTextureAtlasRegion("es/textures/ui.xml", "Box_L");		

		ImageBox* R = (ImageBox*)IUIManager::GetUIManager().CreateComponent(ComponentType::ImageBox);
		mBorders.push_back(R);
		R->SetRender3D(mRender3D, GetRenderTargetSize());
		R->SetManualParent(this);
		R->SetAlign(ALIGNH::RIGHT, ALIGNV::TOP);
		R->SetSizeX(3);		
		R->SetTextureAtlasRegion("es/textures/ui.xml", "Box_R");
		

		ImageBox* B = (ImageBox*)IUIManager::GetUIManager().CreateComponent(ComponentType::ImageBox);
		mBorders.push_back(B);
		B->SetRender3D(mRender3D, GetRenderTargetSize());
		B->SetManualParent(this);
		B->SetAlign(ALIGNH::LEFT, ALIGNV::BOTTOM);
		B->SetSizeY(3);
		B->SetTextureAtlasRegion("es/textures/ui.xml", "Box_B");

		ImageBox* LT = (ImageBox*)IUIManager::GetUIManager().CreateComponent(ComponentType::ImageBox);
		mBorders.push_back(LT);
		LT->SetRender3D(mRender3D, GetRenderTargetSize());
		LT->SetManualParent(this);
		LT->SetSize(Vec2I(6, 6));
		LT->SetTextureAtlasRegion("es/textures/ui.xml", "Box_LT");		

		ImageBox* RT = (ImageBox*)IUIManager::GetUIManager().CreateComponent(ComponentType::ImageBox);
		mBorders.push_back(RT);
		RT->SetRender3D(mRender3D, GetRenderTargetSize());
		RT->SetManualParent(this);
		RT->SetSize(Vec2I(6, 6));
		RT->SetAlign(ALIGNH::RIGHT, ALIGNV::TOP);
		RT->SetTextureAtlasRegion("es/textures/ui.xml", "Box_RT");		

		ImageBox* LB = (ImageBox*)IUIManager::GetUIManager().CreateComponent(ComponentType::ImageBox);;
		mBorders.push_back(LB);
		LB->SetRender3D(mRender3D, GetRenderTargetSize());
		LB->SetManualParent(this);
		LB->SetSize(Vec2I(5, 5));
		LB->SetAlign(ALIGNH::LEFT, ALIGNV::BOTTOM);
		LB->SetTextureAtlasRegion("es/textures/ui.xml", "Box_LB");

		ImageBox* RB = (ImageBox*)IUIManager::GetUIManager().CreateComponent(ComponentType::ImageBox);
		mBorders.push_back(RB);
		RB->SetRender3D(mRender3D, GetRenderTargetSize());
		RB->SetManualParent(this);
		RB->SetSize(Vec2I(6, 6));
		RB->SetAlign(ALIGNH::RIGHT, ALIGNV::BOTTOM);
		RB->SetTextureAtlasRegion("es/textures/ui.xml", "Box_RB");
		
		RefreshBorder();
		IUIManager::GetUIManager().DirtyRenderList();
	}
	else if (!use && !mBorders.empty())
	{
		for (auto ib : mBorders)
		{
			IUIManager::GetUIManager().DeleteComponent(ib);
		}
		mBorders.clear();
		IUIManager::GetUIManager().DirtyRenderList();
	}
}
void WinBase::RefreshBorder()
{
	if (mBorders.empty())
		return;

	enum ORDER{
		ORDER_T,
		ORDER_L,
		ORDER_R,
		ORDER_B,
		ORDER_LT,
		ORDER_RT,
		ORDER_LB,
		ORDER_RB,
		
		ORDER_NUM
	};

	assert(mBorders.size() == ORDER_NUM);

	const Vec2 finalPos = GetFinalPos();
	mBorders[ORDER_T]->SetNSizeX(mWNSize.x);
	mBorders[ORDER_T]->SetWNPos(finalPos);

	mBorders[ORDER_L]->SetNSizeY(mWNSize.y);
	mBorders[ORDER_L]->SetWNPos(finalPos);

	mBorders[ORDER_R]->SetNSizeY(mWNSize.y);
	Vec2 wnpos = finalPos;
	wnpos.x += mWNSize.x;
	mBorders[ORDER_R]->SetWNPos(wnpos);

	mBorders[ORDER_B]->SetNSizeX(mWNSize.x);
	wnpos = finalPos;
	wnpos.y += mWNSize.y;
	mBorders[ORDER_B]->SetWNPos(wnpos);

	mBorders[ORDER_LT]->SetWNPos(finalPos);

	wnpos = finalPos;
	wnpos.x += mWNSize.x;
	mBorders[ORDER_RT]->SetWNPos(wnpos);

	wnpos = finalPos;
	wnpos.y += mWNSize.y;
	mBorders[ORDER_LB]->SetWNPos(wnpos);

	wnpos = finalPos + mWNSize;
	mBorders[ORDER_RB]->SetWNPos(wnpos);
}

void WinBase::SetNPosOffset(const Vec2& offset)
{
	assert(GetType() != ComponentType::Scroller);
	mWNPosOffset = offset;
	mNPosOffset = offset;
	if (mParent)
		mNPosOffset = mParent->ConvertWorldSizeToParentCoord(offset);
	OnPosChanged();
	//if (mUIObject)
		//mUIObject->SetNPosOffset(offset);
	//RefreshScissorRects();

	/*for (auto var : mBorders)
	{
		var->SetNPosOffset(offset);
	}*/
}

void WinBase::SetAnimNPosOffset(const Vec2& offset)
{
	if (mUIObject)
		mUIObject->SetAnimNPosOffset(offset);
	RefreshScissorRects();
}

void WinBase::SetAnimScale(const Vec2& scale, const Vec2& pivot)
{
	if (mUIObject)
	{
		mUIObject->SetAnimScale(scale, pivot);
	}

	if (!mBorders.empty())
	{
		for (auto var : mBorders)
		{
			var->SetAnimScale(scale, pivot);
		}
	}
	RefreshScissorRects();
}

Vec2 WinBase::GetPivotWNPos()
{
	if (!mPivot && mParent)
	{
		return mParent->GetPivotWNPos();
	}

	assert(mUIObject);
	return mUIObject->GetNPos() + mUIObject->GetAnimNPosOffset() + mUIObject->GetNSize()*0.5f;
}

void WinBase::SetPivotToUIObject(const Vec2& pivot)
{
	if(mUIObject)
		mUIObject->SetPivot(pivot);
}

void WinBase::SetScissorRect(bool use, const RECT& rect)
{
	mUseScissor = use;
	if (mUIObject)
	{
		mUIObject->SetUseScissor(use, rect);
	}
}

const RECT& WinBase::GetRegion() const
{
	static RECT r;
	if (mUIObject)
		return mUIObject->GetRegion();

	assert(0);
	return r;
}

void WinBase::PosAnimationTo(const Vec2& destNPos, float speed)
{
	if (destNPos != mNPos)
	{
		mSimplePosAnimEnabled = true;
		mDestNPos = destNPos;
		mAnimationSpeed = speed;
	}
}

void WinBase::OnStartUpdate(float elapsedTime)
{
	if (mHideCounter > 0.f)
	{
		mHideCounter -= elapsedTime;
		if (mHideCounter <= 0.f)
		{
			mHideCounter = 0.f;
			SetVisibleInternal(false);
		}
	}
	// Static Animation.
	if (mSimplePosAnimEnabled)
	{
		Vec2 to = mDestNPos - mNPos;
		float length = to.Normalize();
		float delta = mAnimationSpeed*elapsedTime;
		float movement;
		if (length > delta)
		{
			movement = delta;
		}
		else
		{
			movement = length;
			mSimplePosAnimEnabled = false;
		}
		SetNPos(mNPos + to*movement);
	}

	// Key Frame Animation
	Vec2 evaluatedPos(0, 0);
	Vec2 evaluatedScale(1, 1);
	bool hasPos = false;
	bool hasScale = false;
	FB_FOREACH(it, mAnimations)
	{
		if (it->second->IsActivated())
		{
			IUIAnimation* anim = it->second;
			anim->Update(elapsedTime);
			if (anim->HasPosAnim())
			{
				evaluatedPos += anim->GetCurrentPos();
				hasPos = true;
			}

			if (anim->HasScaleAnim())
			{
				evaluatedScale = evaluatedScale * anim->GetCurrentScale();
				hasScale = true;
			}
			if (anim->HasBackColorAnim())
				SetProperty(UIProperty::BACK_COLOR, StringConverter::toString(anim->GetCurrentBackColor()).c_str());
			if (anim->HasTextColorAnim())
				SetProperty(UIProperty::TEXT_COLOR, StringConverter::toString(anim->GetCurrentTextColor()).c_str());
		}
	}
	if (hasPos)
		SetAnimNPosOffset(evaluatedPos);
	if (hasScale)
		SetAnimScale(evaluatedScale, GetPivotWNPos());
}

void WinBase::SetAlphaBlending(bool set)
{
	if (mUIObject)
		mUIObject->SetAlphaBlending(set);
}

//bool WinBase::GetAlphaBlending() const
//{
//	if (mUIObject)
//		return mUIObject->GetAlphaBlending();
//
//	return false;
//}

float WinBase::PixelToLocalNWidth(int pixel) const
{
	auto rtSize = GetRenderTargetSize();
	return pixel / ((float)rtSize.x * mWNSize.x);
}
float WinBase::PixelToLocalNHeight(int pixel) const
{
	auto rtSize = GetRenderTargetSize();
	return pixel / ((float)rtSize.y * mWNSize.y);
}

Vec2 WinBase::PixelToLocalNSize(const Vec2I& pixel) const
{
	return Vec2(PixelToLocalNWidth(pixel.x), PixelToLocalNHeight(pixel.y));
}

int WinBase::LocalNWidthToPixel(float nwidth) const
{
	auto rtSize = GetRenderTargetSize();
	return Round(nwidth * ((float)rtSize.x * mWNSize.x));
}
int WinBase::LocalNHeightToPixel(float nheight) const
{
	auto rtSize = GetRenderTargetSize();
	return Round(nheight * ((float)rtSize.y * mWNSize.y));
}
Vec2I WinBase::LocalNSizeToPixel(const Vec2& nsize) const
{
	return Vec2I(LocalNWidthToPixel(nsize.x), LocalNHeightToPixel(nsize.y));
}

float WinBase::PixelToLocalNPosX(int pixel) const
{
	auto rtSize = GetRenderTargetSize();
	return (float)pixel / ((float)rtSize.x * mWNSize.x);
}
float WinBase::PixelToLocalNPosY(int pixel) const
{
	auto rtSize = GetRenderTargetSize();
	return (float)pixel / ((float)rtSize.y * mWNSize.y);
}

Vec2 WinBase::PixelToLocalNPos(const Vec2I& pixel) const
{
	return Vec2(PixelToLocalNPosX(pixel.x), PixelToLocalNPosY(pixel.y));
}

int WinBase::LocalNPosXToPixel(float nposx) const
{
	auto rtSize = GetRenderTargetSize();
	return Round(nposx * ((float)rtSize.x * mWNSize.x));
}
int WinBase::LocalNPosYToPixel(float nposy) const
{
	auto rtSize = GetRenderTargetSize();
	return Round(nposy * ((float)rtSize.y * mWNSize.y));
}
Vec2I WinBase::LocalNPosToPixel(const Vec2& npos) const
{
	return Vec2I(LocalNPosXToPixel(npos.x), LocalNPosYToPixel(npos.y));
}


float WinBase::GetTextWidthLocal() const
{
	if (mTextw.empty())
		return 0.f;

	if (mParent)
		return mParent->PixelToLocalNWidth(mTextWidth);
	auto rtSize = GetRenderTargetSize();
	return mTextWidth / (float)rtSize.x;
}

float WinBase::GetTextEndWLocal() const
{
	if (mUIObject)
	{
		Vec2 startPos = mUIObject->GetTextStarNPos();
		
		if (mParent)
			startPos = mParent->ConvertWorldPosToParentCoord(startPos);

		return startPos.x + GetTextWidthLocal();
	}

	return 0.f;
}

bool WinBase::ParseXML(tinyxml2::XMLElement* pelem)
{
	assert(pelem);
	auto rtSize = GetRenderTargetSize();
	const char* sz = pelem->Attribute("name");
	if (sz)
		SetName(sz);

	sz = pelem->Attribute("npos");
	if (sz)
	{
		Vec2 npos = StringConverter::parseVec2(sz);
		SetNPos(npos);
	}

	// positions
	sz = pelem->Attribute("pos");
	if (sz)
	{
		Vec2I pos = StringConverter::parseVec2I(sz);
		SetPos(pos);
	}		

	sz = pelem->Attribute("nposX");
	if (sz)
		SetNPosX(StringConverter::parseReal(sz));

	sz = pelem->Attribute("nposY");
	if (sz)
		SetNPosY(StringConverter::parseReal(sz));

	sz = pelem->Attribute("posX");
	if (sz)
	{
		int x = StringConverter::parseInt(sz);
		SetPosX(x);
	}

	sz = pelem->Attribute("posY");
	if (sz)
	{
		int y = StringConverter::parseInt(sz);
		SetPosY(y);
	}


	Vec2 noffset(0, 0);
	sz = pelem->Attribute("offset");
	{
		if (sz)
		{
			Vec2I offset = StringConverter::parseVec2I(sz);
			mAbsOffset = offset;
			if (mParent)
			{
				noffset = mParent->PixelToLocalNPos(offset);
			}
			else
			{
				noffset = Vec2(offset.x / (float)rtSize.x, offset.y / (float)rtSize.y);
			}
		}
	}
	sz = pelem->Attribute("offsetX");
	if (sz)
	{
		int x = StringConverter::parseInt(sz);
		mAbsOffset.x = x;
		if (mParent)
			noffset.x = mParent->PixelToLocalNPosX(x);
		else
			noffset.x = (float)x / (float)rtSize.x;
	}

	sz = pelem->Attribute("offsetY");
	if (sz)
	{
		int y = StringConverter::parseInt(sz);
		mAbsOffset.y = y;
		if (mParent)
			noffset.y = mParent->PixelToLocalNPosY(y);
		else
			noffset.y = y / (float)rtSize.y;
	}
	//mNPos += noffset;

	//size
	
	sz = pelem->Attribute("nsize");
	if (sz)
	{
		auto data = Split(sz, ",");
		data[0] = StripBoth(data[0].c_str());
		data[1] = StripBoth(data[1].c_str());
		float x, y;
		if (stricmp(data[0].c_str(), "fill") == 0)
			x = 1.0f - mNPos.x;
		else
			x = StringConverter::parseReal(data[0].c_str());

		if (stricmp(data[1].c_str(), "fill") == 0)
			y = 1.0f - mNPos.y;
		else
			y = StringConverter::parseReal(data[1].c_str());

		Vec2 nsize(x, y);
		SetNSize(nsize);
	}

	sz = pelem->Attribute("size");
	if (sz)
	{
		Vec2I v = StringConverter::parseVec2I(sz);
		SetSize(v);
	}

	sz = pelem->Attribute("nsizeX");
	if (sz)
	{
		float x;
		if (stricmp(sz, "fill") == 0)
		{
			x = 1.0f - mNPos.x;
		}
		else
		{
			x = StringConverter::parseReal(sz);
		}
		SetNSizeX(x);
	}

	sz = pelem->Attribute("nsizeY");
	if (sz)
	{
		float y;
		if (stricmp(sz, "fill") == 0)
		{
			y = 1.0f - mNPos.y;
		}
		else
		{
			y = StringConverter::parseReal(sz);
		}
		SetNSizeY(y);
	}

	sz = pelem->Attribute("sizeX");
	if (sz)
	{		
		int x = StringConverter::parseInt(sz);
		SetSizeX(x);
	}

	sz = pelem->Attribute("sizeY");
	if (sz)
	{
		int y = StringConverter::parseInt(sz);
		SetSizeY(y);
	}

	float mAspectRatio = 1.f;
	sz = pelem->Attribute("aspectRatio");
	if (sz)
	{
		mAspectRatio = StringConverter::parseReal(sz);
		mSize.y = (int)(mSize.x * mAspectRatio);
		if (mParent)
		{
			mNSize.y = mParent->PixelToLocalNHeight(mSize.y);
		}
		else
		{
			mNSize.y = mSize.y / (float)rtSize.y;
		}		
	}

	// size mod
	Vec2 nsizeMod(0, 0);
	sz = pelem->Attribute("sizeMod");
	{
		if (sz)
		{
			mSizeMod = StringConverter::parseVec2I(sz);
			if (mParent)
			{
				nsizeMod = mParent->PixelToLocalNSize(mSizeMod);
			}
			else
			{
				nsizeMod = Vec2(mSizeMod.x / (float)rtSize.x, mSizeMod.y / (float)rtSize.y);
			}
		}
	}
	sz = pelem->Attribute("sizeModX");
	if (sz)
	{
		int x = StringConverter::parseInt(sz);
		mSizeMod.x = x;
		if (mParent)
			nsizeMod.x = mParent->PixelToLocalNWidth(x);
		else
			nsizeMod.x = (float)x / (float)rtSize.x;
	}

	sz = pelem->Attribute("sizeModY");
	if (sz)
	{
		int y = StringConverter::parseInt(sz);
		mSizeMod.y = y;
		if (mParent)
			nsizeMod.y = mParent->PixelToLocalNHeight(y);
		else
			nsizeMod.y = y / (float)rtSize.y;
	}
	mNSize += nsizeMod;

	mAbsTempLock = true;
	SetNSize(mNSize);
	SetNPos(mNPos);
	mAbsTempLock = false;

	//ALIGNH::Enum alignh = ALIGNH::LEFT;
	//ALIGNV::Enum alignv = ALIGNV::TOP;
	//sz =pelem->Attribute("AlignH");
	//if (sz)
	//{
	//	alignh = ALIGNH::ConvertToEnum(sz);
	//}

	//sz = pelem->Attribute("AlignV");
	//if (sz)
	//{
	//	alignv = ALIGNV::ConvertToEnum(sz);
	//}
	//SetAlign(alignh, alignv);

	for (int i = 0; i < UIProperty::COUNT; ++i)
	{
		sz = pelem->Attribute(UIProperty::ConvertToString((UIProperty::Enum)i));
		if (sz)
		{
			SetProperty((UIProperty::Enum)i, sz);
		}
	}

	tinyxml2::XMLElement* pElem = pelem->FirstChildElement("Animation");
	while (pElem)
	{
		// this will create mAnimation
		IUIAnimation* pAnim = FB_NEW(UIAnimation);
		pAnim->LoadFromXML(pElem);
		std::string name = pAnim->GetName();
		if (mAnimations.Find(name) != mAnimations.end())
		{
			FB_DELETE(pAnim);
		}
		else
		{
			mAnimations[pAnim->GetName()] = pAnim;
		}
		pElem = pElem->NextSiblingElement("Animation");
	}

	auto eventsElem = pelem->FirstChildElement("Events");
	if (eventsElem)
	{
		auto eventElem = eventsElem->FirstChildElement();
		while (eventElem)
		{
			IEventHandler::EVENT e = ConvertToEventEnum(eventElem->Name());
			if (e != IEventHandler::EVENT_NUM)
			{
				RegisterEventLuaFunc(e, eventElem->GetText());
			}
			
			eventElem = eventElem->NextSiblingElement();
		}
	}


	return true;
}

bool WinBase::ParseLua(const fastbird::LuaObject& compTable)
{
	assert(compTable.IsTable());
	bool success;
	auto rtSize = GetRenderTargetSize();
	std::string name = compTable.GetField("name").GetString(success);
	if (success)
		SetName(name.c_str());

	auto npos = compTable.GetField("npos").GetVec2(success);
	if (success)
		SetNPos(npos);

	// positions
	auto pos = compTable.GetField("pos").GetVec2I(success);
	if (success)
		SetPos(pos);

	auto nPosX = compTable.GetField("nposX").GetFloat(success);
	if (success)
		SetNPosX(nPosX);

	auto nPosY = compTable.GetField("nposY").GetFloat(success);
	if (success)
		SetNPosY(nPosY);


	int x = compTable.GetField("posX").GetInt(success);
	if (success)
		SetPosX(x);

	int y = compTable.GetField("posY").GetInt(success);
	if (success)
		SetPosY(y);

	Vec2 noffset(0, 0);
	auto offset = compTable.GetField("offset").GetVec2I(success);
	if (success)
	{
		mAbsOffset = offset;
		if (mParent)
		{
			noffset = mParent->PixelToLocalNPos(offset);
		}
		else
		{
			noffset = Vec2(offset.x / (float)rtSize.x, offset.y / (float)rtSize.y);
		}
	}
	
	{
		int x = compTable.GetField("offsetX").GetInt(success);
		if (success)
		{
			mAbsOffset.x = x;
			if (mParent)
				noffset.x = mParent->PixelToLocalNPosX(x);
			else
				noffset.x = (float)x / (float)rtSize.x;
		}


		int y = compTable.GetField("offsetY").GetInt(success);
		if (success)
		{
			mAbsOffset.y = y;
			if (mParent)
				noffset.y = mParent->PixelToLocalNPosY(y);
			else
				noffset.y = y / (float)rtSize.y;
		}
		//mNPos += noffset;
	}

	//size

	Vec2 nsize = compTable.GetField("nsize").GetVec2(success);
	if (success)
	{
		SetNSize(nsize);
	}

	Vec2I v = compTable.GetField("size").GetVec2I(success);
	if (success)
	{		
		SetSize(v);
	}

	{
		float x = compTable.GetField("nsizeX").GetFloat(success);
		if (success)
		{
			SetNSizeX(x);
		}
		else
		{
			std::string str = compTable.GetField("nsizeX").GetString(success);
			if (success && stricmp(str.c_str(), "fill") == 0)
			{
				SetNSizeX(1.0f - mNPos.x);
			}
		}
	}

	{
		float y = compTable.GetField("nsizeY").GetFloat(success);
		if (success)
		{
			SetNSizeY(y);
		}
		else
		{
			std::string str = compTable.GetField("nsizeY").GetString(success);
			if (stricmp(str.c_str(), "fill") == 0)
			{
				SetNSizeY(1.0f - mNPos.y);
			}
		}
	}

	{
		int x = compTable.GetField("sizeX").GetInt(success);
		if (success)
		{
			SetSizeX(x);
		}

		int y = compTable.GetField("sizeY").GetInt(success);
		if (success)
		{
			SetSizeY(y);
		}
	}
	{
		// rel
		float fillUntilNX = compTable.GetField("fillUntilNX").GetFloat(success);
		if (success)
		{
			SetNSizeX(fillUntilNX - mNPos.x);
		}

		float fillUntilNY = compTable.GetField("fillUntilNY").GetFloat(success);
		if (success)
		{
			SetNSizeY(fillUntilNY - mNPos.y);
		}
	}

	float mAspectRatio = 1.f;
	mAspectRatio = compTable.GetField("aspectRatio").GetFloat(success);
	if (success)
	{		
		float sizex;
		if (mUseAbsoluteXSize)
		{
			if (mParent)
				sizex = mParent->PixelToLocalNWidth(mSize.x);
			else
				sizex = mSize.x / (float)rtSize.x;
		}
		else
		{
			sizex = mNSize.x;
		}
		Vec2 worldSize;

		if (mParent)
		{
			worldSize = mParent->ConvertChildSizeToWorldCoord(Vec2(sizex, sizex));
		}
		float iWidth = rtSize.x * worldSize.x;
		float iHeight = iWidth / mAspectRatio;
		float height = iHeight / (rtSize.y*mWNSize.y);
		mNSize.y = height;
	}

	// size mod
	Vec2 nsizeMod(0, 0);
	mSizeMod = compTable.GetField("sizeMod").GetVec2I(success);
	{
		if (success)
		{
			if (mParent)
			{
				nsizeMod = mParent->PixelToLocalNSize(mSizeMod);
			}
			else
			{
				nsizeMod = Vec2(mSizeMod.x / (float)rtSize.x, mSizeMod.y / (float)rtSize.y);
			}
		}
	}

	{
		int x = compTable.GetField("sizeModX").GetInt(success);
		if (success)
		{
			mSizeMod.x = x;
			if (mParent)
				nsizeMod.x = mParent->PixelToLocalNWidth(x);
			else
				nsizeMod.x = (float)x / (float)rtSize.x;
		}

		int y = compTable.GetField("sizeModY").GetInt(success);
		if (success)
		{
			mSizeMod.y = y;
			if (mParent)
				nsizeMod.y = mParent->PixelToLocalNHeight(y);
			else
				nsizeMod.y = y / (float)rtSize.y;
		}
		mNSize += nsizeMod;
	}

	mAbsTempLock = true;
	SetNSize(mNSize);
	SetNPos(mNPos);
	mAbsTempLock = false;

	for (int i = 0; i < UIProperty::COUNT; ++i)
	{
		auto field = compTable.GetField(UIProperty::ConvertToString((UIProperty::Enum)i));
		if (field.IsValid())
		{
			std::string v = field.GetString(success);
			if (success)
			{
				SetProperty((UIProperty::Enum)i, v.c_str());
			}
		}
	}

	auto animsTable = compTable.GetField("Animations");
	if (animsTable.IsValid())
	{
		auto it = animsTable.GetSequenceIterator();
		LuaObject animTable;
		while (it.GetNext(animTable))
		{
			IUIAnimation* pAnim = FB_NEW(UIAnimation);
			pAnim->ParseLua(animTable);
			std::string name = pAnim->GetName();
			if (mAnimations.Find(name) != mAnimations.end())
			{
				FB_DELETE(pAnim);
			}
			else
			{
				mAnimations[pAnim->GetName()] = pAnim;
			}
		}
	}	

	auto eventsTable = compTable.GetField("Events");
	if (eventsTable.IsValid())
	{
		auto ti = eventsTable.GetTableIterator();
		LuaTableIterator::KeyValue kv;
		while (ti.GetNext(kv))
		{
			bool s1, s2;
			std::string eventName = kv.first.GetString(s1);
			std::string funcName = kv.second.GetString(s2);
			if (s1 && s2)
			{
				IEventHandler::EVENT e = ConvertToEventEnum(eventName.c_str());
				RegisterEventLuaFunc(e, funcName.c_str());
			}
		}
	}

	return true;
}

float WinBase::GetTextBottomGap() const
{
	auto rtSize = GetRenderTargetSize();
	float yGap = 2.0f / (float)rtSize.y;
	return yGap;
}


void WinBase::ToolTipEvent(IEventHandler::EVENT evt, const Vec2& mouseNPos)
{
	if (mTooltipText.empty())
		return;

	switch (evt)
	{
	case IEventHandler::EVENT_MOUSE_IN:
	{
										  IUIManager::GetUIManager().SetTooltipString(mTooltipText);

										  IUIManager::GetUIManager().SetTooltipPos(mouseNPos);
	}
		break;

	case IEventHandler::EVENT_MOUSE_HOVER:
		IUIManager::GetUIManager().SetTooltipString(mTooltipText);
		IUIManager::GetUIManager().SetTooltipPos(mouseNPos);
		break;

	case IEventHandler::EVENT_MOUSE_OUT:
		IUIManager::GetUIManager().SetTooltipString(std::wstring());
		break;
	}
}

void WinBase::RefreshScissorRects()
{
	if (mUseScissor && mUIObject)
	{
		mUIObject->SetUseScissor(true, GetScissorRegion());
	}
}

RECT WinBase::GetScissorRegion()
{
	if (mUseScissor)
	{
		if (mParent)
		{
			RECT scissor = mParent->GetRegion();
			mParent->GetScissorIntersection(scissor);
			return scissor;
		}
		if (mManualParent)
		{
			RECT scissor = mManualParent->GetRegion();
			mManualParent->GetScissorIntersection(scissor);
			return scissor;
		}
	}

	return mUIObject->GetRegion();
}

void WinBase::GetScissorIntersection(RECT& scissor)
{
	if (mUIObject)
	{
		const auto& parentRegion = mUIObject->GetRegion();
		if (scissor.left < parentRegion.left)
			scissor.left = parentRegion.left;
		if (scissor.right > parentRegion.right)
			scissor.right = parentRegion.right;
		if (scissor.top < parentRegion.top)
			scissor.top = parentRegion.top;
		if (scissor.bottom > parentRegion.bottom)
			scissor.bottom = parentRegion.bottom;
	}
	if (mParent)
		mParent->GetScissorIntersection(scissor);
	if (mManualParent)
		mManualParent->GetScissorIntersection(scissor);
}

void WinBase::SetEnable(bool enable)
{
	if (mEnable == enable)
		return;

	mEnable = enable;
	IEventHandler* pevent = dynamic_cast<IEventHandler*>(this);
	if (pevent)
		pevent->SetEnableEvent(enable);

	if (mUIObject)
	{
		mUIObject->SetTextColor(mTextColor * .5f);
	}
	OnEnableChanged();
}

bool WinBase::GetEnable(bool enable) const
{
	return mEnable;
}

void WinBase::RemoveAllEvents(bool includeChildren)
{
	EventHandler* eventHandler = dynamic_cast<EventHandler*>(this);
	assert(eventHandler);
	eventHandler->UnregisterAllEventFunc();
}

void WinBase::GatherVisit(std::vector<IUIObject*>& v)
{
	if (!mVisible)
		return;
	if (!mBorders.empty())
	{
		for (auto var : mBorders)
		{
			var->GatherVisit(v);
		}
	}
}

void WinBase::SetRender3D(bool render3D, const Vec2I& renderTargetSize)
{
	mRender3D = render3D;
	mRenderTargetSize = renderTargetSize;
	if (mUIObject)
		mUIObject->SetRenderTargetSize(mRenderTargetSize);

	UpdateWorldSize();
}

Vec2I WinBase::GetRenderTargetSize() const
{
	if (mRender3D)
	{
		return mRenderTargetSize;
	}
	else
	{
		return Vec2I(gEnv->pRenderer->GetWidth(), gEnv->pRenderer->GetHeight());
	}
}
}
