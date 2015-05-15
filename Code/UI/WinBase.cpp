#include <UI/StdAfx.h>
#include <UI/WinBase.h>
#include <UI/IUIManager.h>
#include <UI/Container.h>
#include <UI/UIAnimation.h>
#include <UI/ImageBox.h>
#include <Engine/IRenderTarget.h>
#include <CommonLib/StringUtils.h>

namespace fastbird
{
const float WinBase::LEFT_GAP = 0.001f;
HCURSOR WinBase::mCursorOver = 0;
const float WinBase::NotDefined = 12345.6f;
const char* WinBase::sShowAnim = "_ShowAnim";
const char* WinBase::sHideAnim = "_HideAnim";
const float WinBase::sFadeInOutTime = 0.2f;
Vec2I WinBase::sLastPos(0, 0);
Vec2I WinBase::OSWindowPos;

WinBase::WinBase()
: mHwndId(-1)
, mAlignH(ALIGNH::LEFT)
, mAlignV(ALIGNV::TOP)
, mParent(0)
, mNPosAligned(0, 0)
, mMouseIn(false)
, mMouseInPrev(false)
, mNext(0)
, mPrev(0)
, mTextColor(0.8f, 0.8f, 0.8f, 1.0f)
, mTextColorHover(1.f, 1.0f, 1.f, 1.f)
, mTextColorDown(1.0f, 1.0f, 1.0f, 1.f)
, mTextAlignH(ALIGNH::LEFT)
, mTextAlignV(ALIGNV::MIDDLE)
, mMatchSize(false)
, mSize(10, 10)
, mTextSize(24.f)
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
, mNoMouseEvent(false), mNoMouseEventAlone(false)
, mUseScissor(true)
, mUseAbsoluteXPos(false)
, mUseAbsoluteYPos(false)
, mUseAbsoluteXSize(false)
, mUseAbsoluteYSize(false)
, mAbsTempLock(false)
, mAbsOffset(0, 0)
, mNOffset(0, 0)
, mSizeMod(0, 0)
, mManualParent(0)
, mCustomContent(0)
, mLockTextSizeChange(false)
, mStopScissorParent(false)
, mSpecialOrder(0)
, mTextWidth(0)
, mNumTextLines(1)
, mInheritVisibleTrue(true)
, mNSize(NotDefined, NotDefined)
, mWNPos(NotDefined, NotDefined)
, mWNSize(NotDefined, NotDefined)
, mHideCounter(0), mPivot(false)
, mRender3D(false), mTextGap(0, 0)
, mModal(false), mDragable(0, 0), mEnable(true)
, mCurHighlightTime(0)
, mHighlightSpeed(0.f)
, mGoingBright(true), mAlpha(1.f), mShowingTooltip(false), mTabOrder(-1), mSaveCheck(false)
, mFillX(false), mFillY(false), mNSizeMod(0, 0), mRunTimeChild(false)
{
	mVisibility.SetWinBase(this);
}

WinBase::~WinBase()
{
	if (mShowingTooltip)
	{
		gFBEnv->pUIManager->CleanTooltip();
	}
	if (mModal)
	{
		if (mVisibility.IsVisible())
		{
			gFBEnv->pUIManager->IgnoreInput(false, this);
		}
	}
	FB_FOREACH(it, mAnimations)
	{
		FB_SAFE_DEL(it->second);
	}

	for (auto ib : mBorders)
	{
		gFBEnv->pUIManager->DeleteComponent(ib);
	}
	
	gFBEnv->pEngine->DeleteUIObject(mUIObject);
}

void WinBase::SetHwndId(HWND_ID hwndId)
{
	if (mHwndId != hwndId)
	{
		if (mUIObject)
		{
			mUIObject->SetRenderTargetSize(
				gFBEnv->pEngine->GetRequestedWndSize(hwndId)
				);
		}
	}
	mHwndId = hwndId;

	for (auto win : mBorders)
	{
		if (win)
		{
			win->SetHwndId(hwndId);
		}
	}
}

HWND_ID WinBase::GetHwndId() const
{
	auto root = GetRootWnd();
	assert(root);	
	if (root == this)
	{
		return mHwndId;
	}

	return root->GetHwndId();
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
	mSizeMod = sizemod;
	auto rtSize = GetRenderTargetSize();
	if(mParent)
	{
		mNSizeMod = mParent->PixelToLocalNSize(mSizeMod);
	}
	else
	{
		mNSizeMod = Vec2(mSizeMod.x / (float)rtSize.x, mSizeMod.y / (float)rtSize.y);
	}
	SetNSize(mNSize + mNSizeMod);
}

Vec2I WinBase::GetWPos() const
{
	Vec2I wpos(0, 0);
	if (mParent)
	{
		wpos += mParent->GetWPos();
	}
	wpos += mPos;
	wpos += Round(mWNPosOffset * GetRenderTargetSize());
	return wpos;
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
		if (mUseAbsoluteXSize && mUseAbsoluteYSize)
		{
			SetSize(mSize);
		}
		else
		{
			if (mUseAbsoluteXSize)
			{
				SetSizeX(mSize.x);
			}
			if (mUseAbsoluteYSize)
			{
				SetSizeY(mSize.y);
			}
		}
		
		if (mParent)
		{
			mSize = mParent->LocalNSizeToPixel(mNSize);
		}
		else
		{
			auto rtSize = GetRenderTargetSize();
			mSize = Vec2I(Round(mNSize.x *rtSize.x), Round(mNSize.y*rtSize.y));
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
	mAbsOffset = offset;
	if (mParent)
	{
		mNOffset = mParent->PixelToLocalNPos(offset);
	}
	else
	{
		auto rtSize = GetRenderTargetSize();
		mNOffset = Vec2(offset.x / (float)rtSize.x, offset.y / (float)rtSize.y);
	}
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
		if (mUseAbsoluteXPos && mUseAbsoluteYPos)
		{
			SetPos(mPos);
		}
		else
		{
			if (mUseAbsoluteXPos)
			{
				SetPosX(mPos.x);
			}
			if (mUseAbsoluteYPos)
			{
				SetPosY(mPos.y);
			}
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

void WinBase::UpdateNPos()
{
	if (mUseAbsoluteXPos && mUseAbsoluteYPos)
	{
		SetPos(mPos);
	}
	else
	{
		if (mUseAbsoluteXPos)
		{
			SetPosX(mPos.x);
		}
		if (mUseAbsoluteYPos)
		{
			SetPosY(mPos.y);
		}
	}

	mAbsTempLock = true;
	SetNPos(mNPos);
	mAbsTempLock = false;
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
	mNPosAligned = mNPos + mNOffset;
	switch(mAlignH)
	{
	case ALIGNH::LEFT: /*nothing todo*/ break;
	case ALIGNH::CENTER: mNPosAligned.x -= mNSize.x / 2; break;
	case ALIGNH::RIGHT: mNPosAligned.x -= mNSize.x; break;
	}
	switch(mAlignV)
	{
	case ALIGNV::TOP: /*nothing todo*/break;
	case ALIGNV::MIDDLE: mNPosAligned.y -= mNSize.y / 2; break;
	case ALIGNV::BOTTOM: mNPosAligned.y -= mNSize.y; break;
	}
}

//---------------------------------------------------------------------------
// returning true, when visibility changed.
bool WinBase::SetVisible(bool show)
{
	return mVisibility.SetVisible(show);
}

void WinBase::SetVisibleInternal(bool visible)
{
	gFBEnv->pUIManager->DirtyRenderList(GetHwndId());
	if (visible)
	{
		OnEvent(IEventHandler::EVENT_ON_VISIBLE);
		if (mModal)
		{
			gFBEnv->pUIManager->IgnoreInput(true, this);
		}
	}
	else
	{
		OnEvent(IEventHandler::EVENT_ON_HIDE);
		if (mModal)
		{
			gFBEnv->pUIManager->IgnoreInput(false, this);
		}
		mVisibility.Hided();
	}

	if (!mBorders.empty())
	{
		for (auto borderImage : mBorders)
		{
			borderImage->SetVisible(visible);
		}
	}
}

bool WinBase::GetVisible() const
{
	return mVisibility.IsVisible();
}


bool WinBase::GetFocus(bool includeChildren /*= false*/) const
{
	return gFBEnv->pUIManager->IsFocused(this);
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
bool WinBase::IsIn(IMouse* mouse) const
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

bool WinBase::IsIn(const Vec2I& pt) const
{
	if (mUseScissor)
	{
		const RECT& rect = GetScissorRegion();
		bool inScissor = !(pt.x < rect.left || pt.x > rect.right || pt.y < rect.top || pt.y > rect.bottom);
		if (!inScissor)
			return false;
	}

	auto wpos = GetWPos();
	bool in = !(
		wpos.x > pt.x ||
		wpos.x + mSize.x  < pt.x ||
		wpos.y > pt.y ||
		wpos.y + mSize.y < pt.y
		);
	return in;
}

//---------------------------------------------------------------------------
bool WinBase::OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard)
{
	if (!mVisibility.IsVisible())
		return false;

	if (mNoMouseEvent || mNoMouseEventAlone)
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
			bool invalidate = false;
			if ((mouse->IsLButtonDown() && !mouse->IsLButtonDownPrev() && mUIObject && mouse->IsDragStartIn(mUIObject->GetRegion()))
				|| (mMouseDragStartInHere && mouse->IsLButtonDown())
				)
			{
				mMouseDragStartInHere = true;
				gFBEnv->pUIManager->SetFocusUI(GetRootWnd());
				auto hwnd = gFBEnv->pEngine->GetWindowHandle(mHwndId);
				RECT rect;
				GetWindowRect(hwnd, &rect);
				OSWindowPos.x = rect.left;
				OSWindowPos.y = rect.top;
				invalidate = true;
			}
			else
			{
				mMouseDragStartInHere = false;
			}
			if (mouse->IsLButtonDown() && mMouseDragStartInHere)
			{
				invalidate = true;
				long x,  y;
				mouse->GetDeltaXY(x, y);
				if (x != 0 || y != 0)
				{
					if (OnEvent(IEventHandler::EVENT_MOUSE_DRAG))
					{
						mouse->Invalidate();
					}
					else if (mDragable != Vec2I::ZERO)
					{
						OnDrag(x, y);
					}
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
					invalidate = true;
				ToolTipEvent(IEventHandler::EVENT_MOUSE_HOVER, mousepos);
			}
			else if (mMouseIn)
			{
				if (OnEvent(IEventHandler::EVENT_MOUSE_IN))
					invalidate = true;
				ToolTipEvent(IEventHandler::EVENT_MOUSE_IN, mousepos);
			}

			if (mouse->IsLButtonClicked())
			{
				if (OnEvent(EVENT_MOUSE_LEFT_CLICK))
				{
					invalidate = true;
					mouse->Invalidate(GetType() == ComponentType::Button ? true : false);
				}
			}
			else if (mouse->IsLButtonDoubleClicked())
			{
				if (OnEvent(EVENT_MOUSE_LEFT_DOUBLE_CLICK))
				{
					invalidate = true;
					gFBEnv->pUIManager->CleanTooltip();
				}

			}
			else if (mouse->IsRButtonClicked())
			{
				if (OnEvent(EVENT_MOUSE_RIGHT_CLICK))
				{
					mouse->Invalidate();
					invalidate = true;
				}
			}
			if (invalidate)
			{
				mouse->Invalidate();
				TriggerRedraw();
			}
		}		
		else if (mMouseInPrev)
		{
			if (OnEvent(IEventHandler::EVENT_MOUSE_OUT))
			{
				mouse->Invalidate();
				TriggerRedraw();
			}
			ToolTipEvent(IEventHandler::EVENT_MOUSE_OUT, mousepos);
		}

		mMouseInPrev = mMouseIn;
	}
	else if (mMouseInPrev)
	{
		if (OnEvent(IEventHandler::EVENT_MOUSE_OUT))
		{
			mouse->Invalidate();
			TriggerRedraw();
		}
		ToolTipEvent(IEventHandler::EVENT_MOUSE_OUT, mousepos);
		mMouseInPrev = false;
	}

	if (!GetFocus())
		return mMouseIn;

	if (keyboard->IsValid() && gFBUIManager->GetKeyboardFocusUI() == this)
	{	

		char c = (char)keyboard->GetChar();

		switch(c)
		{
		case VK_RETURN:
			{
				if (OnEvent(EVENT_ENTER))
				{
					keyboard->PopChar();
					TriggerRedraw();
					keyboard->Invalidate();
				}
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
							startPos.x += mWNSize.x*.5f - nwidth*.5f + mTextGap.x / (float)rtSize.x;;
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
	IFont* pFont = gFBEnv->pRenderer->GetFont();
	pFont->SetHeight(mTextSize);
	mTextWidth = (int)gFBEnv->pRenderer->GetFont()->GetTextWidth(
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
		//assert(region.right - region.left == Round(mSize.x * mUIObject->GetAnimScale().x));
		assert(!mUseAbsoluteYSize || region.bottom - region.top == Round(mSize.y * mUIObject->GetAnimScale().y));

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
	RefreshScissorRects();
}

void WinBase::OnAlphaChanged()
{
	if (mUIObject)
	{
		mUIObject->SetAlpha(GetAlpha());
	}
}

std::string WinBase::TranslateText(const char* text)
{
	if (!text || strlen(text) == 0)
		return std::string();
	if (text[0] == '@')
	{
		char varName[255];
		const char* msgTranslationUnit = GetMsgTranslationUnit();
		sprintf_s(varName, "%s.%s", msgTranslationUnit, text + 1);
		auto var = GetLuaVar(gFBEnv->pUIManager->GetLuaState(), varName);
		if (var.IsString())
		{
			return var.GetString();
		}
	}
	return std::string();
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
	TriggerRedraw();
}

const wchar_t* WinBase::GetText() const
{
	return mTextw.c_str();
}

IUIAnimation* WinBase::GetOrCreateUIAnimation(const char* name)
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

IUIAnimation* WinBase::GetUIAnimation(const char* name)
{
	auto itfind = mAnimations.Find(name);
	if (itfind == mAnimations.end())
	{
		return 0;
	}
	return itfind->second;
}

void WinBase::SetUIAnimation(IUIAnimation* anim)
{
	const char* name = anim->GetName();
	assert(name && strlen(name) != 0);

	auto itfind = mAnimations.Find(name);
	if (itfind != mAnimations.end())
	{
		FB_DELETE(itfind->second);
		Log("(Info) Animation (%s) in Comp(%s) is replaced.", name, mName.c_str());
	}
	mAnimations[name] = anim;
	anim->SetTargetUI(this);
}

void WinBase::ClearAnimationResult()
{
	SetAnimNPosOffset(Vec2(0, 0));
}

bool WinBase::SetProperty(UIProperty::Enum prop, const char* val)
{
	assert(val);
	switch (prop)
	{
	case UIProperty::POS:
	{
							auto v = Split(val, ", ");
							assert(v.size() == 2);
							int x = ParseIntPosX(v[0]);
							int y = ParseIntPosY(v[1]);
							SetPos(Vec2I(x, y));
							return true;
	}
	case UIProperty::POSX:
	{
							int x = ParseIntPosX(val);
							 SetPosX(x);
							 return true;
	}
	case UIProperty::POSY:
	{
							int y = ParseIntPosY(val);
							 SetPosY(y);
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
								SetInitialOffset(mAbsOffset);
								UpdateWorldPos();
								return true;
	}
	case UIProperty::OFFSETY:
	{
								mAbsOffset.y = StringConverter::parseInt(val);
								SetInitialOffset(mAbsOffset);
								UpdateWorldPos();
								return true;
	}
	case UIProperty::SIZE:
	{
		SetSize(StringConverter::parseVec2I(val));
		return true;
	}
	case UIProperty::SIZEX:
	{
		SetSizeX(StringConverter::parseInt(val));
		return true;
	}
	case UIProperty::SIZEY:
	{
		SetSizeY(StringConverter::parseInt(val));
		return true;
	}
	case UIProperty::NSIZEX:
	{
		float size = 1.f;
		if (_stricmp(val, "fill") == 0)
		{
			size = 1.0f - mNPos.x;
			mFillX = true;
		}
		else
			size = StringConverter::parseReal(val);
		SetNSizeX(size);
		return true;
	}
	case UIProperty::NSIZEY:
	{
		float size = 1.f;
		if (_stricmp(val, "fill") == 0)
		{
			size = 1.0f - mNPos.y;
			mFillY = true;
		}
		else
			size = StringConverter::parseReal(val);

		SetNSizeY(size);
		return true;
	}

	case UIProperty::BACK_COLOR:
	{
								   if (mUIObject)
								   {
									   Color color(val);
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

	case UIProperty::TEXT_GAP:
	{
		int gap = StringConverter::parseInt(val);
		mTextGap.x = gap;
		mTextGap.y = gap;
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
									mTextColor = Color(val);								   
								   mUIObject->SetTextColor(mTextColor);
								   return true;
	}
	case UIProperty::TEXT_COLOR_HOVER:
	{
		mTextColorHover = Color(val);
										 return true;
	}
	case UIProperty::TEXT_COLOR_DOWN:
	{
		mTextColorDown = Color(val);
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
							 std::string translated = TranslateText(val);
							 if (translated != val)
							 {
								 mTextBeforeTranslated = val;
							 }

							 if (translated.empty())
							 {
								 SetText(AnsiToWide(val));
							 }
							 else
							 {
								 SetText(AnsiToWide(translated.c_str()));
							 }
							 return true;
	}
	case UIProperty::ALPHA:
	{
							  bool b = StringConverter::parseBool(val);
							  if (mUIObject)
								  mUIObject->SetAlphaBlending(b);
							  gFBEnv->pUIManager->DirtyRenderList(GetHwndId());
							  return true;
	}

	case UIProperty::TOOLTIP:
	{
								assert(val);
								auto msg = TranslateText(val);
								if (msg != val)
								{
									mTooltipTextBeforeT = val;
								}
								if (msg.empty())
									mTooltipText = AnsiToWide(val);
								else
									mTooltipText = AnsiToWide(msg.c_str());
								return true;
	}

	case UIProperty::NO_MOUSE_EVENT:
	{
									   mNoMouseEvent = StringConverter::parseBool(val);
									   return true;
	}

	case UIProperty::NO_MOUSE_EVENT_ALONE:
	{
											 mNoMouseEventAlone = StringConverter::parseBool(val);
											 return true;
	}

	case UIProperty::USE_SCISSOR:
		{
										mUseScissor = StringConverter::parseBool(val);
										RefreshScissorRects();
										/*if (!mUseScissor)
										{
											mUIObject->SetUseScissor(false, RECT());
										}
										else if (mParent && mUIObject)
											mUIObject->SetUseScissor(mUseScissor, mParent->GetRegion());*/
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
											auto anim = FB_NEW(UIAnimation);
											anim->SetName(sShowAnim);											   
											anim->SetTargetUI(this);
										   anim->ClearData();
										   anim->AddScale(0.0f, Vec2(0.1f, 0.1f));
										   anim->AddScale(sFadeInOutTime, Vec2(1.0f, 1.0f));
										   anim->AddAlpha(0.0f, 0.0f);
										   anim->AddAlpha(sFadeInOutTime, 1.0f);
										   anim->SetLength(sFadeInOutTime);
										   anim->SetLoop(false);
										   mVisibility.AddShowAnimation(anim);
										   mPivot = true;
										   return true;
		}

		case UIProperty::HIDE_ANIMATION:
		{
											auto anim = FB_NEW(UIAnimation);
											anim->SetName(sHideAnim);
											anim->SetTargetUI(this);
										   anim->ClearData();
										   anim->AddScale(0.0f, Vec2(1.0f, 1.0f));
										   anim->AddScale(sFadeInOutTime, Vec2(0.001f, 0.001f));
										   anim->AddAlpha(0.0f, 1.0f);
										   anim->AddAlpha(sFadeInOutTime, 0.0f);
										   anim->SetLength(sFadeInOutTime);
										   anim->SetLoop(false);
										   mVisibility.AddHideAnimation(anim);
										   mPivot = true;
										   return true;
		}

		case UIProperty::ENABLED:
		{
								   SetEnable(StringConverter::parseBool(val));
								   return true;
		}

		case UIProperty::MODAL:
		{
								  mModal = StringConverter::parseBool(val);
								  return true;
		}
		
		case UIProperty::DRAGABLE:
		{
			mDragable = StringConverter::parseVec2I(val);
			return true;
		}

		case UIProperty::TAB_ORDER:
		{
			mTabOrder = StringConverter::parseUnsignedInt(val);
			return true;
		}
	}
	Error(DEFAULT_DEBUG_ARG, FormatString("Not processed property(%s) found", UIProperty::ConvertToString(prop)));
	return false;
}

bool WinBase::GetProperty(UIProperty::Enum prop, char val[], bool notDefaultOnly)
{
	assert(val);
	switch (prop)
	{
	case UIProperty::POS:
	{
		auto data = StringConverter::toString(mPos);
		strcpy(val, data.c_str());
		return true;
	}
	case UIProperty::POSX:
	{
		auto data = StringConverter::toString(mPos.x);
		strcpy(val, data.c_str());
		return true;
	}
	case UIProperty::POSY:
	{
		auto data = StringConverter::toString(mPos.y);
		strcpy(val, data.c_str());
		return true;
	}
	case UIProperty::NPOS:
	{
		auto data = StringConverter::toString(mNPos);
		strcpy(val, data.c_str());
		return true;
	}
	case UIProperty::NPOSX:
	{
		auto data = StringConverter::toString(mNPos.x);
		strcpy(val, data.c_str());
		return true;
	}
	case UIProperty::NPOSY:
	{
		auto data = StringConverter::toString(mNPos.y);
		strcpy(val, data.c_str());
		return true;
	}
	case UIProperty::OFFSETX:
	{
		auto data = StringConverter::toString(mAbsOffset.x);
		strcpy(val, data.c_str());
		
		return true;
	}
	case UIProperty::OFFSETY:
	{
		auto data = StringConverter::toString(mAbsOffset.y);
		strcpy(val, data.c_str());		
		return true;
	}
	case UIProperty::SIZE:
	{
		auto data = StringConverter::toString(mSize);
		strcpy(val, data.c_str());
		
		return true;
	}
	case UIProperty::SIZEX:
	{
		auto data = StringConverter::toString(mSize.x);
		strcpy(val, data.c_str());
		
		return true;
	}
	case UIProperty::SIZEY:
	{
		auto data = StringConverter::toString(mSize.y);
		strcpy(val, data.c_str());
		
		return true;
	}
	case UIProperty::NSIZEX:
	{
		auto data = mFillX ? std::string("fill") : StringConverter::toString(mNSize.x);
		strcpy(val, data.c_str());
				
		return true;
	}
	case UIProperty::NSIZEY:
	{
		auto data = mFillY ? std::string("fill") : StringConverter::toString(mNSize.y);
		strcpy(val, data.c_str());
		
		return true;
	}
	case UIProperty::BACK_COLOR:
	{
		if (!mUIObject)
			return false;

		const auto& diffuseColor = mUIObject->GetMaterial()->GetDiffuseColor();
		if (notDefaultOnly) {
			if (diffuseColor == UIProperty::GetDefaultValueVec4(prop)) {
				return false;
			}
		}
		auto data = StringConverter::toString(diffuseColor);
		strcpy(val, data.c_str());
		
		return true;		

	}

	case UIProperty::TEXT_SIZE:
	{
		if (notDefaultOnly) {
			if (mTextSize == UIProperty::GetDefaultValueFloat(prop)) {
				return false;
			}
		}

		auto data = StringConverter::toString(mTextSize);
		strcpy(val, data.c_str());
	
		break;
	}

	case UIProperty::FIXED_TEXT_SIZE:
	{
		if (notDefaultOnly) {
			if (mFixedTextSize == UIProperty::GetDefaultValueBool(prop)) {
				return false;
			}
		}
		auto data = StringConverter::toString(mFixedTextSize);
		strcpy(val, data.c_str());

		return true;
	}

	case UIProperty::TEXT_ALIGN:
	{
		if (notDefaultOnly) {
			if (mTextAlignH == UIProperty::GetDefaultValueInt(prop)) {
				return false;
			}
		}

		strcpy(val, ALIGNH::ConvertToString(mTextAlignH));

		return true;
	}
	case UIProperty::TEXT_VALIGN:
	{
		if (notDefaultOnly) {
			if (mTextAlignV == UIProperty::GetDefaultValueInt(prop)) {
				return false;
			}
		}

		strcpy(val, ALIGNV::ConvertToString(mTextAlignV));
		
		return true;
	}
	case UIProperty::TEXT_LEFT_GAP:
	{
		if (notDefaultOnly) {
			if (mTextGap.x == UIProperty::GetDefaultValueInt(prop)) {
				return false;
			}
		}

		auto data = StringConverter::toString(mTextGap.x);
		strcpy(val, data.c_str());
		
		return true;
	}
	case UIProperty::TEXT_RIGHT_GAP:
	{
		if (notDefaultOnly) {
			if (mTextGap.y == UIProperty::GetDefaultValueInt(prop)) {
				return false;
			}
		}

		auto data = StringConverter::toString(mTextGap.y);
		strcpy(val, data.c_str());

		return true;
	}

	case UIProperty::TEXT_GAP:
	{
		if (notDefaultOnly) {
			if (mTextGap == UIProperty::GetDefaultValueVec2I(prop)) {
				return false;
			}
		}

		auto data = StringConverter::toString(mTextGap.x);
		strcpy(val, data.c_str());
		
		return true;
	}
	case UIProperty::MATCH_SIZE:
	{
		if (notDefaultOnly) {
			if (mMatchSize == UIProperty::GetDefaultValueBool(prop)) {
				return false;
			}
		}

		auto data = StringConverter::toString(mMatchSize);
		strcpy(val, data.c_str());
		return true;
	}
	case UIProperty::NO_BACKGROUND:
	{
		if (!mUIObject)
			return false;

		if (notDefaultOnly) {
			if (mUIObject->GetNoDrawBackground() == UIProperty::GetDefaultValueBool(prop)) {
				return false;
			}
		}

		auto data = StringConverter::toString(mUIObject->GetNoDrawBackground());
		strcpy(val, data.c_str());
		return true;
	}
	case UIProperty::TEXT_COLOR:
	{
		if (notDefaultOnly) {
			if (mTextColor == UIProperty::GetDefaultValueVec4(prop)) {
				return false;
			}
		}

		auto data = StringConverter::toString(mTextColor);
		strcpy(val, data.c_str());
		return true;
	}
	case UIProperty::TEXT_COLOR_HOVER:
	{
		if (notDefaultOnly) {
			if (mTextColorHover == UIProperty::GetDefaultValueVec4(prop)) {
				return false;
			}
		}

		auto data = StringConverter::toString(mTextColorHover);
		strcpy(val, data.c_str());
		return true;
	}
	case UIProperty::TEXT_COLOR_DOWN:
	{
		if (notDefaultOnly) {
			if (mTextColorDown == UIProperty::GetDefaultValueVec4(prop)) {
				return false;
			}
		}

		auto data = StringConverter::toString(mTextColorDown);
		strcpy(val, data.c_str());
		return true;
	}
	case UIProperty::ALIGNH:
	{
		if (notDefaultOnly) {
			if (mAlignH == UIProperty::GetDefaultValueInt(prop)) {
				return false;
			}
		}

		strcpy(val, ALIGNH::ConvertToString(mAlignH));
		
		return true;
	}
	case UIProperty::ALIGNV:
	{
		if (notDefaultOnly) {
			if (mAlignV == UIProperty::GetDefaultValueInt(prop)) {
				return false;
			}
		}

		strcpy(val, ALIGNV::ConvertToString(mAlignV));
		return true;
	}
	case UIProperty::TEXT:
	{
		if (notDefaultOnly) {
			if (mTextw.empty() && mTextBeforeTranslated.empty())
				return false;
		}

		if (!mTextBeforeTranslated.empty()){
			strcpy(val, mTextBeforeTranslated.c_str());
		}
		else
		{
			strcpy(val, WideToAnsi(mTextw.c_str()));
		}
		return true;
	}
	case UIProperty::ALPHA:
	{
		if (!mUIObject)
			return false;
		
		if (notDefaultOnly) {
			if (mUIObject->GetAlphaBlending() == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		auto data = StringConverter::toString(mUIObject->GetAlphaBlending());
		strcpy(val, data.c_str());
		return true;
		
		return false;
	}

	case UIProperty::TOOLTIP:
	{
		if (notDefaultOnly) {
			if (mTooltipTextBeforeT.empty() && mTooltipText.empty())
				return false;
		}

		if (!mTooltipTextBeforeT.empty())
		{
			strcpy(val, mTooltipTextBeforeT.c_str());
		}
		else
		{
			strcpy(val, WideToAnsi(mTooltipText.c_str()));
		}		
		return true;
	}

	case UIProperty::NO_MOUSE_EVENT:
	{
		if (notDefaultOnly) {
			if (mNoMouseEvent == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		auto data = StringConverter::toString(mNoMouseEvent);
		strcpy(val, data.c_str());
		
		return true;
	}

	case UIProperty::NO_MOUSE_EVENT_ALONE:
	{
		if (notDefaultOnly) {
			if (mNoMouseEventAlone == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		auto data = StringConverter::toString(mNoMouseEventAlone);
		strcpy(val, data.c_str());

		
		return true;
	}

	case UIProperty::USE_SCISSOR:
	{
		if (notDefaultOnly) {
			if (mUseScissor == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		auto data = StringConverter::toString(mUseScissor);
		strcpy(val, data.c_str());

		return true;
	}

	case UIProperty::USE_BORDER:
	{
		if (notDefaultOnly) {
			if (!mBorders.empty() == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		auto data = StringConverter::toString(!mBorders.empty());
		strcpy(val, data.c_str());
		
		return true;
	}

	case UIProperty::SPECIAL_ORDER:
	{
		if (notDefaultOnly) {
			if (mSpecialOrder == UIProperty::GetDefaultValueInt(prop))
				return false;
		}

		auto data = StringConverter::toString(mSpecialOrder);
		strcpy(val, data.c_str());

		return true;
	}

	case UIProperty::INHERIT_VISIBLE_TRUE:
	{
		if (notDefaultOnly) {
			if (mInheritVisibleTrue == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		auto data = StringConverter::toString(mInheritVisibleTrue);
		strcpy(val, data.c_str());
		return true;
	}

	case UIProperty::VISIBLE:
	{
		if (notDefaultOnly) {
			if (mVisibility.IsVisible() == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		auto data = StringConverter::toString(mVisibility.IsVisible());
		strcpy(val, data.c_str());
		return true;
	}

	case UIProperty::ENABLED:
	{
		if (notDefaultOnly) {
			if (mEnable == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		auto data = StringConverter::toString(mEnable);
		strcpy(val, data.c_str());
		
		return true;
	}

	case UIProperty::MODAL:
	{
		if (notDefaultOnly) {
			if (mModal == UIProperty::GetDefaultValueBool(prop))
				return false;
		}
		auto data = StringConverter::toString(mModal);
		strcpy(val, data.c_str());

		return true;
	}

	case UIProperty::DRAGABLE:
	{
		if (notDefaultOnly) {
			if (mDragable == UIProperty::GetDefaultValueVec2I(prop))
				return false;
		}

		auto data = StringConverter::toString(mDragable);
		strcpy(val, data.c_str());
		
		return true;
	}

	case UIProperty::TAB_ORDER:
	{
		if (notDefaultOnly) {
			if (mTabOrder == UIProperty::GetDefaultValueInt(prop))
				return false;
		}

		auto data = StringConverter::toString(mTabOrder);
		strcpy(val, data.c_str());

		return true;
	}
	}
	val = "";
	return false;
}

bool WinBase::GetPropertyAsBool(UIProperty::Enum prop, bool defaultVal)
{
	char buf[256];
	bool get = GetProperty(prop, buf, false);
	if (get)
	{
		return StringConverter::parseBool(buf);
	}
	return defaultVal;
}

float WinBase::GetPropertyAsFloat(UIProperty::Enum prop, float defaultVal)
{
	char buf[256];
	bool get = GetProperty(prop, buf, false);
	if (get)
	{
		return StringConverter::parseReal(buf);
	}
	return defaultVal;
}

int WinBase::GetPropertyAsInt(UIProperty::Enum prop, int defaultVal)
{
	char buf[256];
	bool get = GetProperty(prop, buf, false);
	if (get)
	{
		return StringConverter::parseInt(buf);
	}
	return defaultVal;
}

void WinBase::SetUseBorder(bool use)
{
	if (use && mBorders.empty())
	{
		ImageBox* T = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
		T->SetHwndId(GetHwndId());
		mBorders.push_back(T);
		T->SetRender3D(mRender3D, GetRenderTargetSize());
		T->SetManualParent(this);
		T->SetSizeY(3);
		T->SetTextureAtlasRegion("es/textures/ui.xml", "Box_T");	
		

		ImageBox* L = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
		L->SetHwndId(GetHwndId());
		mBorders.push_back(L);
		L->SetRender3D(mRender3D, GetRenderTargetSize());
		L->SetManualParent(this);
		L->SetSizeX(3);		
		L->SetTextureAtlasRegion("es/textures/ui.xml", "Box_L");		

		ImageBox* R = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
		R->SetHwndId(GetHwndId());
		mBorders.push_back(R);
		R->SetRender3D(mRender3D, GetRenderTargetSize());
		R->SetManualParent(this);
		R->SetAlign(ALIGNH::RIGHT, ALIGNV::TOP);
		R->SetSizeX(3);		
		R->SetTextureAtlasRegion("es/textures/ui.xml", "Box_R");
		

		ImageBox* B = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
		B->SetHwndId(GetHwndId());
		mBorders.push_back(B);
		B->SetRender3D(mRender3D, GetRenderTargetSize());
		B->SetManualParent(this);
		B->SetAlign(ALIGNH::LEFT, ALIGNV::BOTTOM);
		B->SetSizeY(3);
		B->SetTextureAtlasRegion("es/textures/ui.xml", "Box_B");

		ImageBox* LT = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
		LT->SetHwndId(GetHwndId());
		mBorders.push_back(LT);
		LT->SetRender3D(mRender3D, GetRenderTargetSize());
		LT->SetManualParent(this);
		LT->SetSize(Vec2I(6, 6));
		LT->SetTextureAtlasRegion("es/textures/ui.xml", "Box_LT");		

		ImageBox* RT = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
		RT->SetHwndId(GetHwndId());
		mBorders.push_back(RT);
		RT->SetRender3D(mRender3D, GetRenderTargetSize());
		RT->SetManualParent(this);
		RT->SetSize(Vec2I(6, 6));
		RT->SetAlign(ALIGNH::RIGHT, ALIGNV::TOP);
		RT->SetTextureAtlasRegion("es/textures/ui.xml", "Box_RT");		

		ImageBox* LB = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);;
		LB->SetHwndId(GetHwndId());
		mBorders.push_back(LB);
		LB->SetRender3D(mRender3D, GetRenderTargetSize());
		LB->SetManualParent(this);
		LB->SetSize(Vec2I(5, 5));
		LB->SetAlign(ALIGNH::LEFT, ALIGNV::BOTTOM);
		LB->SetTextureAtlasRegion("es/textures/ui.xml", "Box_LB");

		ImageBox* RB = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
		RB->SetHwndId(GetHwndId());
		mBorders.push_back(RB);
		RB->SetRender3D(mRender3D, GetRenderTargetSize());
		RB->SetManualParent(this);
		RB->SetSize(Vec2I(6, 6));
		RB->SetAlign(ALIGNH::RIGHT, ALIGNV::BOTTOM);
		RB->SetTextureAtlasRegion("es/textures/ui.xml", "Box_RB");
		
		RefreshBorder();
		RefreshScissorRects();
		gFBEnv->pUIManager->DirtyRenderList(GetHwndId());
	}
	else if (!use && !mBorders.empty())
	{
		for (auto ib : mBorders)
		{
			gFBEnv->pUIManager->DeleteComponent(ib);
		}
		mBorders.clear();
		gFBEnv->pUIManager->DirtyRenderList(GetHwndId());
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

	Vec2 offsetTwo = Vec2(3.0f, 3.0f) / (float)GetRenderTargetSize().x;
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
	/*mBorders[ORDER_T]->SetNSizeX(mWNSize.x + offsetTwo.x*2.f);
	mBorders[ORDER_T]->SetWNPos(Vec2(finalPos.x, finalPos.y - offsetTwo.y));

	mBorders[ORDER_L]->SetNSizeY(mWNSize.y + offsetTwo.y*2.f);
	mBorders[ORDER_L]->SetWNPos(Vec2(finalPos.x - offsetTwo.x, finalPos.y));

	mBorders[ORDER_R]->SetNSizeY(mWNSize.y + offsetTwo.y*2.f);
	Vec2 wnpos = finalPos;
	wnpos.x += mWNSize.x + offsetTwo.x;
	mBorders[ORDER_R]->SetWNPos(wnpos);

	mBorders[ORDER_B]->SetNSizeX(mWNSize.x + offsetTwo.x*2.f);
	wnpos = finalPos;
	wnpos.y += mWNSize.y + offsetTwo.y;
	mBorders[ORDER_B]->SetWNPos(wnpos);

	mBorders[ORDER_LT]->SetWNPos(finalPos - offsetTwo);

	wnpos = finalPos;
	wnpos.x += mWNSize.x + offsetTwo.x;
	wnpos.y -= offsetTwo.y;
	mBorders[ORDER_RT]->SetWNPos(wnpos);

	wnpos = finalPos;
	wnpos.y += mWNSize.y + offsetTwo.y;
	wnpos.x -= offsetTwo.x;
	mBorders[ORDER_LB]->SetWNPos(wnpos);

	wnpos = finalPos + mWNSize + offsetTwo;
	mBorders[ORDER_RB]->SetWNPos(wnpos);*/
}

void WinBase::SetNPosOffset(const Vec2& offset)
{
	/*char buf[255];
	sprintf_s(buf, "Winbase SetNPosOffset %s", ComponentType::ConvertToString(GetType()));
	Profiler p(buf);*/
	assert(GetType() != ComponentType::Scroller);
	mWNPosOffset = offset;
	mNPosOffset = offset;
	if (mParent)
		mNPosOffset = mParent->ConvertWorldSizeToParentCoord(offset);
	OnPosChanged();
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

void WinBase::ApplyAnim(IUIAnimation* anim, Vec2& pos, Vec2& scale, bool& hasPos, bool& hasScale)
{
	if (anim->HasPosAnim())
	{
		hasPos = true;
		pos += anim->GetCurrentPos();
	}
	if (anim->HasScaleAnim())
	{
		hasScale = true;
		scale *= anim->GetCurrentScale();
	}
	if (anim->HasBackColorAnim())
	{
		SetProperty(UIProperty::BACK_COLOR, StringConverter::toString(anim->GetCurrentBackColor()).c_str());
	}

	if (anim->HasTextColorAnim())
	{
		SetProperty(UIProperty::TEXT_COLOR, StringConverter::toString(anim->GetCurrentTextColor()).c_str());
	}
	if (anim->HasAlphaAnim())
	{
		if (mUIObject)
		{
			mAlpha = anim->GetCurrentAlpha();
			OnAlphaChanged();
		}
	}
}

void WinBase::OnStartUpdate(float elapsedTime)
{
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
	for(auto& it: mAnimations)
	{
		IUIAnimation* anim = it.second;
		if (anim->IsActivated())
		{			
			anim->Update(elapsedTime);
			ApplyAnim(anim, evaluatedPos, evaluatedScale, hasPos, hasScale);
		}
	}
	if (mVisibility.IsShowing())
	{
		for (auto anim : mVisibility.mShowAnimations)
		{
			if (anim->IsActivated())
			{
				anim->Update(elapsedTime);
				ApplyAnim(anim, evaluatedPos, evaluatedScale, hasPos, hasScale);
			}
		}
	}
	if (mVisibility.IsHiding())
	{
		for (auto anim : mVisibility.mHideAnimations)
		{
			if (anim->IsActivated())
			{
				anim->Update(elapsedTime);
				ApplyAnim(anim, evaluatedPos, evaluatedScale, hasPos, hasScale);
			}
		}
	}

	if (hasPos)
		SetAnimNPosOffset(evaluatedPos);
	if (hasScale)
		SetAnimScale(evaluatedScale, GetPivotWNPos());

	if (mHighlightSpeed > 0.f)
	{
		ProcessHighlight(elapsedTime);
	}

	mVisibility.Update(elapsedTime);
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
	if (mWNSize.x == 0.f)
		return 0.0f;
	return pixel / ((float)rtSize.x * mWNSize.x);
}
float WinBase::PixelToLocalNHeight(int pixel) const
{
	auto rtSize = GetRenderTargetSize();
	if (mWNSize.y == 0.0f)
		return 0.0f;
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
	if (mWNSize.x == 0.f)
		return 0.f;
	return (float)pixel / ((float)rtSize.x * mWNSize.x);
}
float WinBase::PixelToLocalNPosY(int pixel) const
{
	auto rtSize = GetRenderTargetSize();
	if (mWNSize.y == 0.0f)
	{
		return 0.0f;
	}
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
		StringVector vec = Split(sz);
		if (vec.size() < 2)
		{
			SetPos(Vec2I(0, 0));
		}
		else
		{
			Vec2I pos;
			pos.x = ParseIntPosX(vec[0]);
			pos.y = ParseIntPosY(vec[1]);
			SetPos(pos);
		}
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
		int x = ParseIntPosX(sz);
		SetPosX(x);
	}

	sz = pelem->Attribute("posY");
	if (sz)
	{
		int y = ParseIntPosY(sz);
		SetPosY(y);
	}

	sz = pelem->Attribute("offset");
	{
		if (sz)
		{
			Vec2I offset = StringConverter::parseVec2I(sz);
			SetInitialOffset(offset);
		}
	}
	sz = pelem->Attribute("offsetX");
	if (sz)
	{
		int x = StringConverter::parseInt(sz);
		mAbsOffset.x = x;
		SetInitialOffset(mAbsOffset);
	}

	sz = pelem->Attribute("offsetY");
	if (sz)
	{
		int y = StringConverter::parseInt(sz);
		mAbsOffset.y = y;
		SetInitialOffset(mAbsOffset);
	}

	//size	
	sz = pelem->Attribute("nsize");
	if (sz)
	{
		auto data = Split(sz, ",");
		data[0] = StripBoth(data[0].c_str());
		data[1] = StripBoth(data[1].c_str());
		float x, y;
		if (stricmp(data[0].c_str(), "fill") == 0){
			x = 1.0f - mNPos.x;
			mFillX = true;
		}
		else
			x = StringConverter::parseReal(data[0].c_str());

		if (stricmp(data[1].c_str(), "fill") == 0) {
			y = 1.0f - mNPos.y;
			mFillY = true;
		}
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
			mFillX = true;
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
			mFillY = true;
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
	sz = pelem->Attribute("sizeMod");
	{
		if (sz)
		{
			mSizeMod = StringConverter::parseVec2I(sz);
		}
	}
	sz = pelem->Attribute("sizeModX");
	if (sz)
	{
		int x = StringConverter::parseInt(sz);
		mSizeMod.x = x;
	}

	sz = pelem->Attribute("sizeModY");
	if (sz)
	{
		int y = StringConverter::parseInt(sz);
		mSizeMod.y = y;
	}

	if (mParent)
	{
		mNSizeMod = mParent->PixelToLocalNSize(mSizeMod);
	}
	else
	{
		mNSizeMod = Vec2(mSizeMod.x / (float)rtSize.x, mSizeMod.y / (float)rtSize.y);
	}

	mAbsTempLock = true;
	SetNSize(mNSize + mNSizeMod);
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
		mAnimations[pAnim->GetName()] = pAnim;
		
		pElem = pElem->NextSiblingElement("Animation");
	}

	auto eventsElem = pelem->FirstChildElement("Events");
	if (eventsElem)
	{
		auto eventElem = eventsElem->FirstChildElement();
		while (eventElem)
		{
			IEventHandler::EVENT e = IEventHandler::ConvertToEnum(eventElem->Name());
			if (e != IEventHandler::EVENT_NUM)
			{
				const char* funcName = eventElem->GetText();
				bool succ = RegisterEventLuaFunc(e, funcName);
				if (succ)
				{
					mEventFuncNames[e] = funcName;
				}
			}
			
			eventElem = eventElem->NextSiblingElement();
		}
	}


	return true;
}

void WinBase::Save(tinyxml2::XMLElement& elem)
{
	elem.SetAttribute("name", mName.c_str());
	if (mUseAbsoluteXSize && mUseAbsoluteYSize)
	{
		elem.SetAttribute("size", StringConverter::toString(mSize).c_str());
	}
	else if (!mUseAbsoluteXSize && !mUseAbsoluteYSize)
	{
		std::string  strX;
		if (mFillX)	{
			strX = "fill";
		}
		else {
			strX = StringConverter::toString(mNSize.x);
		}
		std::string strY;
		if (mFillY) {
			strY = "fill";
		}
		else{
			strY = StringConverter::toString(mNSize.y);
		}
		elem.SetAttribute("nsize", (strX + " " + strY).c_str());
	}
	else
	{
		if (mUseAbsoluteXSize)
		{
			elem.SetAttribute("sizeX", mSize.x);
		}
		else
		{
			elem.SetAttribute("nsizeX",  mFillX ? "fill" : StringConverter::toString(mNSize.x).c_str());
		}

		if (mUseAbsoluteYSize)
		{
			elem.SetAttribute("sizeY", mSize.y);
		}
		else
		{
			elem.SetAttribute("nsizeY", mFillY ?"fill": StringConverter::toString(mNSize.y).c_str());
		}
	}

	if (mUseAbsoluteXPos && mUseAbsoluteYPos)
	{
		elem.SetAttribute("pos", StringConverter::toString(mPos).c_str());
	}
	else if (!mUseAbsoluteXPos && !mUseAbsoluteYPos)
	{
		elem.SetAttribute("npos", StringConverter::toString(mNPos).c_str());
	}
	else
	{
		if (mUseAbsoluteXPos)
		{
			elem.SetAttribute("posX", mPos.x);
		}
		else
		{
			elem.SetAttribute("nposX", mNPos.x);
		}

		if (mUseAbsoluteYPos)
		{
			elem.SetAttribute("posY", mPos.y);
		}
		else
		{
			elem.SetAttribute("nposY", mNPos.y);
		}
	}

	if (mAbsOffset != Vec2I::ZERO)
	{
		elem.SetAttribute("offset", StringConverter::toString(mAbsOffset).c_str());
	}	

	// should not save. run time application.
	/*if (mSizeMod != Vec2I::ZERO)
	{
		elem.SetAttribute("sizeMod", StringConverter::toString(mSizeMod).c_str());
	}*/
		
	for (int i = UIProperty::BACK_COLOR; i < UIProperty::COUNT; ++i)
	{
		char buf[256];
		auto got = GetProperty(UIProperty::Enum(i), buf, true);
		if (got)
		{
			elem.SetAttribute(UIProperty::ConvertToString(i), buf);
		}
	}

	for (auto it : mAnimations)
	{
		auto animationElem = elem.GetDocument()->NewElement("Animation");
		elem.InsertEndChild(animationElem);
		auto name = it.first;
		auto anim = it.second;
		anim->Save(*animationElem);
	}

	if (!mEventFuncNames.empty())
	{
		auto eventsElem = elem.GetDocument()->NewElement("Events");
		elem.InsertEndChild(eventsElem);
		for (auto it : mEventFuncNames)
		{
			auto eElem = eventsElem->GetDocument()->NewElement(IEventHandler::ConvertToString(it.first));
			eventsElem->InsertEndChild(eElem);
			auto text = eElem->GetDocument()->NewText(it.second.c_str());
			eElem->InsertEndChild(text);
		}
	}
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
	{
		sLastPos = pos;
		SetPos(pos);
	}

	auto nPosX = compTable.GetField("nposX").GetFloat(success);
	if (success)
		SetNPosX(nPosX);

	auto nPosY = compTable.GetField("nposY").GetFloat(success);
	if (success)
		SetNPosY(nPosY);


	int x = compTable.GetField("posX").GetInt(success);
	if (success)
	{
		sLastPos.x = x;
		SetPosX(x);
	}

	int y = compTable.GetField("posY").GetInt(success);
	if (success)
	{
		sLastPos.y = y;
		SetPosY(y);
	}

	auto offset = compTable.GetField("offset").GetVec2I(success);
	if (success)
	{
		SetInitialOffset(offset);
	}
	
	{
		int x = compTable.GetField("offsetX").GetInt(success);
		if (success)
		{
			mAbsOffset.x = x;
			SetInitialOffset(mAbsOffset);
		}


		int y = compTable.GetField("offsetY").GetInt(success);
		if (success)
		{
			mAbsOffset.y = y;
			SetInitialOffset(mAbsOffset);
		}
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
				IEventHandler::EVENT e = IEventHandler::ConvertToEnum(eventName.c_str());
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
										  gFBEnv->pUIManager->SetTooltipString(mTooltipText);

										  gFBEnv->pUIManager->SetTooltipPos(mouseNPos);
										  mShowingTooltip = true;
	}
		break;

	case IEventHandler::EVENT_MOUSE_HOVER:
		gFBEnv->pUIManager->SetTooltipString(mTooltipText);
		gFBEnv->pUIManager->SetTooltipPos(mouseNPos);
		mShowingTooltip = true;
		break;

	case IEventHandler::EVENT_MOUSE_OUT:
		gFBEnv->pUIManager->SetTooltipString(std::wstring());
		mShowingTooltip = false;
		break;
	}
}

void WinBase::RefreshScissorRects()
{
	if (mUseScissor && mUIObject)
	{
		mUIObject->SetUseScissor(true, GetScissorRegion());
	}
	else
	{
		mUIObject->SetUseScissor(false, RECT());
	}
}

RECT WinBase::GetScissorRegion() const
{
	RECT scissor = GetRegion();
	/*if (!mBorders.empty())
	{
		ExpandRect(scissor, 3);
	}*/
	if (mUseScissor)
	{
		if (mParent)
		{			
			mParent->GetScissorIntersection(scissor);
			return scissor;
		}
		if (mManualParent)
		{
			mManualParent->GetScissorIntersection(scissor);
			return scissor;
		}
	}

	return scissor;
}

void WinBase::GetScissorIntersection(RECT& scissor)
{
	if (mUIObject)
	{
		const auto& myRegion = mUIObject->GetRegion();
		if (scissor.left < myRegion.left)
			scissor.left = myRegion.left;
		if (scissor.right > myRegion.right)
			scissor.right = myRegion.right;
		if (scissor.top < myRegion.top)
			scissor.top = myRegion.top;
		if (scissor.bottom > myRegion.bottom)
			scissor.bottom = myRegion.bottom;
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
		if (!mEnable)
			mUIObject->SetTextColor(mTextColor * .5f);
		else
			mUIObject->SetTextColor(mTextColor);

	}
	
	OnEnableChanged();
}

bool WinBase::GetEnable() const
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
	if (!mVisibility.IsVisible())
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
		auto root = GetRootWnd();
		assert(root);
		auto hwndId = root->GetHwndId();
		return gFBEnv->pEngine->GetRequestedWndSize(hwndId);
	}
}

IWinBase* WinBase::GetRootWnd() const
{
	if (mParent)
		return mParent->GetRootWnd();
	if (mManualParent)
		return mManualParent->GetRootWnd();
	return (IWinBase*)this;
}

void WinBase::OnDrag(int dx, int dy)
{
	auto rtSize = GetRenderTargetSize();
	Vec2 nposOffset = { dx / (float)rtSize.x, dy / (float)rtSize.y };
	if (mParent)
	{
		nposOffset = mParent->ConvertWorldSizeToParentCoord(nposOffset);
	}
	 
	mAbsTempLock = true;
	if (!mDragable.x)
		nposOffset.x = 0;
	if (!mDragable.y)
		nposOffset.y = 0;
	auto newPos = mNPos + nposOffset;
	newPos.x = std::max(0.f, newPos.x);
	newPos.x = std::min(1.f, newPos.x);
	newPos.y = std::max(0.f, newPos.y);
	newPos.y = std::min(1.f, newPos.y);
	
		
	SetNPos(newPos);
	mAbsTempLock = false;
	if (mParent)
	{
		mParent->OnChildHasDragged();
	}
}


int WinBase::ParseIntPosX(const std::string& posX)
{
	if_assert_fail(!posX.empty())
		return 0;

	if (posX[0] == '+')
	{
		int val = StringConverter::parseInt(&posX[1]);
		sLastPos.x = val + sLastPos.x;
		return sLastPos.x;
	}
	else
	{
		int val = StringConverter::parseInt(posX);
		sLastPos.x = val;
		return val;
	}
}

int WinBase::ParseIntPosY(const std::string& posY)
{
	if_assert_fail(!posY.empty())
		return 0;

	if (posY[0] == '+')
	{
		int val = StringConverter::parseInt(&posY[1]);
		sLastPos.y = val + sLastPos.y;
		return sLastPos.y;
	}
	else
	{
		int val = StringConverter::parseInt(posY);
		sLastPos.y = val;
		return val;
	}
}

void WinBase::StartHighlight(float speed)
{
	for (auto ib : mBorders)
	{
		ib->StartHighlight(speed);
	}

	mHighlightSpeed = speed;
	mCurHighlightTime = 0.f;
	mGoingBright = true;
}

void WinBase::StopHighlight()
{
	for (auto ib : mBorders)
	{
		ib->StopHighlight();
	}

	mHighlightSpeed = 0.f;
	mCurHighlightTime = 0.f;
	if (mUIObject)
	{
		auto mat = mUIObject->GetMaterial();
		if (mat)
			mat->SetAmbientColor(Vec4(0, 0, 0, 1));
	}
}

void WinBase::ProcessHighlight(float dt)
{
	if (mUIObject)
	{
		mCurHighlightTime += mGoingBright ? dt * mHighlightSpeed : -dt * mHighlightSpeed;
		if (mCurHighlightTime > 1.f)
		{
			mCurHighlightTime = 1.0f;
			mGoingBright = false;
		}
		else if (mCurHighlightTime < 0.f)
		{
			mCurHighlightTime = 0.f;
			mGoingBright = true;
		}

		auto mat = mUIObject->GetMaterial();
		if (mat)
		{
			mat->SetAmbientColor(Lerp(Vec4(0, 0, 0, 1), Vec4(0, 1, 1, 1), mCurHighlightTime));
		}
	}
}

bool WinBase::GetNoBackground() const
{
	if (mUIObject)
	{
		return mUIObject->GetNoDrawBackground();
	}
	return false;
}

const Color WinBase::GetBackColor()
{
	if (mUIObject)
	{
		auto mat = mUIObject->GetMaterial();
		if (mat)
		{
			return mat->GetDiffuseColor();
		}
	}

	return Color::Zero;
}

float WinBase::GetAlpha() const
{
	if (mParent)
	{
		return mParent->GetAlpha() * mAlpha;
	}
	return mAlpha;
}

const char* WinBase::GetMsgTranslationUnit() const
{
	auto root = GetRootWnd();
	if (root == this)
	{
		return "msg";
	}
	else
	{
		return root->GetMsgTranslationUnit();
	}
	
}

void WinBase::TriggerRedraw()
{
	if (mHwndId == 1)
		return;

	auto rt = gFBEnv->pRenderer->GetRenderTarget(mHwndId);
	if (rt)
	{
		rt->TriggerDrawEvent();
	}
}

IWinBase* WinBase::WinBaseWithPoint(const Vec2I& pt, bool container) const
{
	if (IsIn(pt))
		return (IWinBase*)this;
	return 0;
}

void WinBase::GatherTabOrder(VectorMap<unsigned, IWinBase*>& winbases) const
{
	if (!mEnable)
		return;

	if (mTabOrder != -1)
	{
		winbases[mTabOrder] = (IWinBase*)this;
	}
}

void WinBase::SetSaveNameCheck(bool set)
{
	mSaveCheck = set;
}

bool WinBase::GetSaveNameCheck() const
{
	return mSaveCheck;
}

}