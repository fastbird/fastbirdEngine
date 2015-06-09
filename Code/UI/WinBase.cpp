#include <UI/StdAfx.h>
#include <UI/WinBase.h>
#include <UI/IUIManager.h>
#include <UI/Container.h>
#include <UI/UIAnimation.h>
#include <UI/ImageBox.h>
#include <UI/IUIEditor.h>
#include <Engine/IRenderTarget.h>
#include <Engine/RendererStructs.h>
#include <CommonLib/StringUtils.h>

namespace fastbird
{
const int WinBase::LEFT_GAP = 2;
const int WinBase::BOTTOM_GAP = 2;
HCURSOR WinBase::sCursorOver = 0;
HCURSOR WinBase::sCursorAll = 0;
HCURSOR WinBase::sCursorNWSE = 0;
HCURSOR WinBase::sCursorNESW = 0;
HCURSOR WinBase::sCursorWE = 0;
HCURSOR WinBase::sCursorNS = 0;
const float WinBase::NotDefined = 12345.6f;
const char* WinBase::sShowAnim = "_ShowAnim";
const char* WinBase::sHideAnim = "_HideAnim";
const float WinBase::sFadeInOutTime = 0.2f;
Vec2I WinBase::sLastPos(0, 0);
Vec2I WinBase::OSWindowPos;

bool WinBase::sSuppressPropertyWarning = false;

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
, mSize(123, 321)
, mTextSize(22.f)
, mFixedTextSize(true)
, mWNScrollingOffset(0, 0)
, mMouseDragStartInHere(false)
, mDestNPos(0, 0)
, mSimplePosAnimEnabled(false)
, mAnimationSpeed(0.f)
, mAspectRatio(1.0)
, mNPos(0, 0)
, mUIObject(0)
, mNoMouseEvent(false), mNoMouseEventAlone(false), mVisualOnlyUI(false)
, mUseScissor(true)
, mUseAbsoluteXPos(true)
, mUseAbsoluteYPos(true)
, mUseAbsoluteXSize(true)
, mUseAbsoluteYSize(true)
, mAbsOffset(0, 0)
, mNOffset(0, 0)
, mManualParent(0)
, mCustomContent(0)
, mLockTextSizeChange(false)
, mStopScissorParent(false)
, mSpecialOrder(0)
, mTextWidth(0)
, mNumTextLines(1)
, mInheritVisibleTrue(true)
, mPos(0, 0)
, mWPos(0, 0)
, mNSize(NotDefined, NotDefined)
//, mWNSize(NotDefined, NotDefined)
, mHideCounter(0), mPivot(false)
, mRender3D(false), mTextGap(0, 0)
, mModal(false), mDragable(0, 0), mEnable(true)
, mCurHighlightTime(0)
, mHighlightSpeed(0.f)
, mGoingBright(true), mAlpha(1.f), mShowingTooltip(false), mTabOrder(-1), mSaveCheck(false)
, mFillX(false), mFillY(false), mRunTimeChild(false), mGhost(false), mAspectRatioSet(false)
, mAnimScale(1, 1), mAnimatedPos(0, 0), mAnimPos(0, 0)
, mGatheringException(false)
, mKeepUIRatio(false)
, mUpdateAlphaTexture(false)
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

void WinBase::SuppressPropertyWarning(bool warning){
	sSuppressPropertyWarning = warning;
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
// Sizing
//---------------------------------------------------------------------------
void WinBase::ChangeSize(const Vec2I& size){
	if (mSize == size)
		return;
	SetSize(size);
	OnSizeChanged();
}

//---------------------------------------------------------------------------
void WinBase::ChangeSizeX(int sizeX){
	if (mSize.x == sizeX)
		return;
	SetSizeX(sizeX);
	OnSizeChanged();
}

//---------------------------------------------------------------------------
void WinBase::ChangeSizeY(int sizeY){
	if (mSize.y == sizeY)
		return;
	SetSizeY(sizeY);
	OnSizeChanged();
}

//---------------------------------------------------------------------------
void WinBase::ChangeNSize(const Vec2& nsize){
	if (mNSize == nsize)
		return;
	SetNSize(nsize);
	OnSizeChanged();
}

void WinBase::ChangeNSizeX(float x){
	if (mNSize.x == x)
		return;
	SetNSizeX(x);
	OnSizeChanged();
}
void WinBase::ChangeNSizeY(float y){
	if (mNSize.y == y)
		return;
	SetNSizeY(y);
	OnSizeChanged();
}

//---------------------------------------------------------------------------
void WinBase::OnSizeChanged()
{
	if (mAnimScale != Vec2::ONE){
		mScaledSize = Round(mSize * mAnimScale);
	}
	else{
		mScaledSize = mSize;
	}

	if (mAlignH != ALIGNH::LEFT || mAlignV != ALIGNV::TOP)
	{
		OnPosChanged(false);
		//UpdateAlignedPos();
	}

	if (!mFixedTextSize && mTextSize != mScaledSize.y) {
		mTextSize = (float)mScaledSize.y;
		CalcTextWidth();
		AlignText();
		if (mUIObject)
			mUIObject->SetTextSize(mTextSize);
	}
	else{
		if (mTextAlignH != ALIGNH::LEFT && mTextAlignV != ALIGNV::TOP){
			AlignText();
		}
	}

	if (mUIObject){
		mUIObject->SetUISize(mScaledSize);
	}

	RefreshBorder();
	RefreshScissorRects();

	if (mParent)
		mParent->SetChildrenPosSizeChanged();
}

//---------------------------------------------------------------------------
void WinBase::SetSize(const fastbird::Vec2I& size){
	mSize = size;
	Vec2I localRT = GetParentSize();
	if (mSize.x < 0)
		mSize.x = localRT.x + mSize.x;
	if (mSize.y < 0)
		mSize.y = localRT.y + mSize.y;
	if (mSize.x == 0) mSize.x = 1;
	if (mSize.y == 0) mSize.y = 1;
	
	mNSize = mSize / Vec2(localRT);
}

//---------------------------------------------------------------------------
void WinBase::SetSizeX(int x){
	mSize.x = x;
	int rtX = GetParentSize().x;
	if (mSize.x < 0)
		mSize.x = rtX + mSize.x;
	if (mSize.x == 0) mSize.x = 1;
	mNSize.x = mSize.x / (float)rtX;
}

//---------------------------------------------------------------------------
void WinBase::SetSizeY(int y){
	mSize.y = y;
	int rtY = GetParentSize().y;
	if (mSize.y < 0)
		mSize.y = rtY + mSize.y;
	if (mSize.y == 0) mSize.y = 1;
	mNSize.y = mSize.y / (float)rtY;
}

//---------------------------------------------------------------------------
void WinBase::SetNSize(const fastbird::Vec2& size) // normalized size (0.0~1.0)
{
	mNSize = size;
	Vec2I localRT = GetParentSize();
	if (mNSize.x == 0.f) mNSize.x = 1.0f / (float)localRT.x;
	if (mNSize.y == 0.f) mNSize.y = 1.0f / (float)localRT.y;
	mSize = Round(localRT * mNSize);
}

//---------------------------------------------------------------------------
void WinBase::SetNSizeX(float x) // normalized size (0.0~1.0)
{
	mNSize.x = x;
	float rtX = (float)GetParentSize().x;
	if (mNSize.x == 0.f) mNSize.x = 1.0f / rtX;
	mSize.x = Round(rtX * mNSize.x);
}

//---------------------------------------------------------------------------
void WinBase::SetNSizeY(float y) // normalized size (0.0~1.0)
{
	mNSize.y = y;
	float rtY = (float)GetParentSize().y;
	if (mNSize.y == 0.f) mNSize.y = 1.0f / rtY;
	mSize.y = Round(rtY * mNSize.y);
}

//---------------------------------------------------------------------------
void WinBase::SetWNSize(const fastbird::Vec2& size)
{
	Vec2I isize = Round(GetRenderTargetSize() * size);
	SetSize(isize);
}

//---------------------------------------------------------------------------
void WinBase::OnParentSizeChanged() {
	assert(mParent);
	bool sizeChanged = false;
	if (!mUseAbsoluteXSize){
		mSize.x = Round(mParent->GetFinalSize().x * mNSize.x);
		sizeChanged = true;
	}
	else{
		mNSize.x = mSize.x / (float)mParent->GetFinalSize().x;
	}

	if (!mUseAbsoluteYSize) {
		mSize.y = Round(mParent->GetFinalSize().y * mNSize.y);
		sizeChanged = true;
	}
	else{
		mNSize.y = mSize.y / (float)mParent->GetFinalSize().y;
	}
	bool posChanged = false;
	if (!mUseAbsoluteXPos){
		mPos.x = Round(mParent->GetFinalSize().x * mNPos.x);
		posChanged = true;
	}
	if (!mUseAbsoluteYPos){
		mPos.y = Round(mParent->GetFinalSize().y * mNPos.y);
		posChanged = true;
	}

	if (sizeChanged){
		OnSizeChanged();
	}
	else{
		RefreshScissorRects();
	}

	if (posChanged || mAlignH!=ALIGNH::LEFT || mAlignV!=ALIGNV::MIDDLE)
		OnPosChanged(false);
}

//---------------------------------------------------------------------------
void WinBase::ModifySize(const Vec2I& sizemod)
{
	Vec2I newSize = mSize + sizemod;
	ChangeSize(newSize);
	//SetSize(newSize);
}

//---------------------------------------------------------------------------
// Positioning
//---------------------------------------------------------------------------
void WinBase::ChangePos(const Vec2I& pos){
	SetPos(pos);
	OnPosChanged(false);
}

void WinBase::ChangePosX(int posx){
	SetPosX(posx);
	OnPosChanged(false);
}

void WinBase::ChangePosY(int posy){
	SetPosY(posy);
	OnPosChanged(false);
}

//---------------------------------------------------------------------------
void WinBase::ChangeNPos(const Vec2& npos){
	SetNPos(npos);
	OnPosChanged(false);
}

void WinBase::ChangeNPosX(float xpos){
	SetNPosX(xpos);
	OnPosChanged(false);
}
void WinBase::ChangeNPosY(float ypos){
	SetNPosY(ypos);
	OnPosChanged(false);
}

//---------------------------------------------------------------------------
void WinBase::ChangeWPos(const Vec2I& wpos){
	SetWPos(wpos);
	OnPosChanged(false);
}

//---------------------------------------------------------------------------
void WinBase::OnPosChanged(bool anim)
{
	if (anim){
		UpdateAnimatedPos();
	}
	else{
		UpdateWorldPos();
	}

	if (mUIObject)
	{
		mUIObject->SetUIPos(mAnimatedPos);
	}

	RefreshBorder();
	RefreshScissorRects();

	if (mParent)
	{
		mParent->SetChildrenPosSizeChanged();
	}
}

//---------------------------------------------------------------------------
void WinBase::OnParentPosChanged(){
	OnPosChanged(false);
}

//---------------------------------------------------------------------------
void WinBase::UpdateWorldPos()
{
	mWPos = Vec2I::ZERO;
	if (mParent)
	{
		mWPos += mParent->GetFinalPos();
	}
	mWPos += mPos;

	UpdateAlignedPos();
	UpdateScrolledPos();
	UpdateAnimatedPos();
}

//---------------------------------------------------------------------------
void WinBase::UpdateAlignedPos()
{
	mWAlignedPos = mWPos + mAbsOffset;
	switch (mAlignH)
	{
	case ALIGNH::LEFT: /*nothing todo*/ break;
	case ALIGNH::CENTER: mWAlignedPos.x -= Round(mScaledSize.x / 2.f); break;
	case ALIGNH::RIGHT: mWAlignedPos.x -= mScaledSize.x; break;
	}
	switch (mAlignV)
	{
	case ALIGNV::TOP: /*nothing todo*/break;
	case ALIGNV::MIDDLE: mWAlignedPos.y -= Round(mScaledSize.y / 2.f); break;
	case ALIGNV::BOTTOM: mWAlignedPos.y -= mScaledSize.y; break;
	}

	const auto& rt = GetRenderTargetSize();
	mWNPos = mWAlignedPos / Vec2(rt);
}

const Vec2I WinBase::GetAlignedPos() const
{
	Vec2I alignedPos = mPos;
	switch (mAlignH)
	{
	case ALIGNH::LEFT: /*nothing todo*/ break;
	case ALIGNH::CENTER: alignedPos.x -= Round(mScaledSize.x / 2.f); break;
	case ALIGNH::RIGHT: alignedPos.x -= mScaledSize.x; break;
	}
	switch (mAlignV)
	{
	case ALIGNV::TOP: /*nothing todo*/break;
	case ALIGNV::MIDDLE: alignedPos.y -= Round(mScaledSize.y / 2.f); break;
	case ALIGNV::BOTTOM: alignedPos.y -= mScaledSize.y; break;
	}
	return alignedPos;
}

//---------------------------------------------------------------------------
void WinBase::UpdateScrolledPos(){
	const auto& rt = GetRenderTargetSize();
	mScrolledPos = mWAlignedPos + Round(mWNScrollingOffset*rt);
}

void WinBase::UpdateAnimatedPos(){
	mAnimatedPos = mScrolledPos + Round(mAnimPos * GetParentSize());
}

//---------------------------------------------------------------------------
void WinBase::SetPos(const fastbird::Vec2I& pos)
{
	mPos = pos;
	auto localRT = GetParentSize();
	mNPos = pos / Vec2(localRT);
}

//---------------------------------------------------------------------------
void WinBase::SetPosX(int x)
{
	mPos.x = x;
	auto rtX = GetParentSize().x;
	mNPos.x = x / (float)rtX;
}

//---------------------------------------------------------------------------
void WinBase::SetPosY(int y)
{
	mPos.y = y;
	auto rtY = GetParentSize().y;
	mNPos.y = y / (float)rtY;
}

//---------------------------------------------------------------------------
void WinBase::SetPosWithTranslator(const Vec2I& pos){
	Vec2I translatedPos = pos;
	const auto& rt = GetParentSize();
	if (pos.x < 0){
		translatedPos.x = rt.x + pos.x;
	}
	if (pos.y < 0){
		translatedPos.y = rt.y + pos.y;
	}
	SetPos(translatedPos);
}
void WinBase::SetPosWithTranslatorX(int x){
	int translatedX = x;
	const auto& rt = GetParentSize();
	if (x < 0){
		translatedX = rt.x + x;
	}
	SetPosX(translatedX);

}
void WinBase::SetPosWithTranslatorY(int y){
	int translatedY = y;
	const auto& rt = GetParentSize();
	if (y < 0){
		translatedY = rt.y + y;
	}
	SetPosY(translatedY);
}

//---------------------------------------------------------------------------
void WinBase::SetWPos(const Vec2I& wpos){
	Vec2I lpos = wpos;
	if (mParent)
	{
		lpos -= mParent->GetWPos();
	}
	SetPos(lpos);
}

//---------------------------------------------------------------------------
void WinBase::SetNPos(const fastbird::Vec2& pos) // normalized pos (0.0~1.0)
{
	mNPos = pos;
	mPos = Round(GetParentSize() * pos);
}

//---------------------------------------------------------------------------
void WinBase::SetNPosX(float x)
{
	mNPos.x = x;
	mPos.x = Round(GetParentSize().x * x);
}

//---------------------------------------------------------------------------
void WinBase::SetNPosY(float y)
{
	mNPos.y = y;
	mPos.y = Round(GetParentSize().y * y);
}

//---------------------------------------------------------------------------
void WinBase::SetWNPos(const fastbird::Vec2& wnPos)
{
	Vec2I wpos = Round(wnPos / GetRenderTargetSize());
	SetWPos(wpos);
}

//---------------------------------------------------------------------------
void WinBase::SetInitialOffset(Vec2I offset)
{
	mAbsOffset = offset;
	OnPosChanged(false);
}

//---------------------------------------------------------------------------
void WinBase::Move(Vec2I amount){
	Vec2I newPos = mPos + amount;
	ChangePos(newPos);
}

//---------------------------------------------------------------------------
fastbird::Vec2I WinBase::ConvertToScreen(const fastbird::Vec2 npos) const
{
	auto rtSize = GetRenderTargetSize();
	Vec2I screenPos = { Round(npos.x * rtSize.x), Round(npos.y * rtSize.y) };

	return screenPos;
}

//---------------------------------------------------------------------------
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
	OnPosChanged(false);
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
		OnEvent(UIEvents::EVENT_ON_VISIBLE);
		if (mModal)
		{
			gFBEnv->pUIManager->IgnoreInput(true, this);
		}
	}
	else
	{
		OnEvent(UIEvents::EVENT_ON_HIDE);
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
	Vec2I cursorPos = mouse->GetPos();
	bool inScissor = true;
	if (mUseScissor)
	{
		const RECT& rect = GetScissorRegion();
		inScissor = !(cursorPos.x < rect.left || cursorPos.x > rect.right || 
			cursorPos.y < rect.top || cursorPos.y > rect.bottom);

	}
	return inScissor && !(	cursorPos.x < mScrolledPos.x ||
							cursorPos.x > mScrolledPos.x + mSize.x ||
							cursorPos.y < mScrolledPos.y ||
							cursorPos.y > mScrolledPos.y + mSize.y);
}

bool WinBase::IsIn(const Vec2I& pt, bool ignoreScissor, Vec2I* expand) const
{
	if (mUseScissor && !ignoreScissor)
	{
		RECT rect = GetScissorRegion();
		if (expand)
		{
			rect.left -= expand->x / 2;
			rect.right += expand->x / 2;
			rect.top -= expand->y / 2;
			rect.bottom += expand->y / 2;
		}
		bool inScissor = !(pt.x < rect.left || pt.x > rect.right || pt.y < rect.top || pt.y > rect.bottom);
		if (!inScissor)
		{
			return false;
		}
	}

	auto finalPos = GetFinalPos();
	auto size = mSize;
	if (expand){
		finalPos.x -= expand->x / 2;
		size.x += expand->x;
		finalPos.y -= expand->y / 2;
		size.y += expand->y;
	}	

	bool in = !(
		finalPos.x > pt.x ||
		finalPos.x + size.x < pt.x ||
		finalPos.y > pt.y ||
		finalPos.y + size.y < pt.y
		);
	return in;
}

bool WinBase::IsPtOnLeft(const Vec2I& pt, int area) const{
	auto wpos = GetFinalPos();
	return pt.x <= wpos.x + area && pt.x >= wpos.x - area;
}
bool WinBase::IsPtOnRight(const Vec2I& pt, int area) const{
	auto wpos = GetFinalPos();
	int right = wpos.x + mSize.x;
	return pt.x <= right + area && pt.x >= right - area;
}
bool WinBase::IsPtOnTop(const Vec2I& pt, int area) const{
	auto wpos = GetFinalPos();
	int top = wpos.y;
	return pt.y <= top + area && pt.y >= top - area;
}
bool WinBase::IsPtOnBottom(const Vec2I& pt, int area) const{
	auto wpos = GetFinalPos();
	int bottom = wpos.y + mSize.y;
	return pt.y <= bottom + area && pt.y >= bottom - area;
}

//---------------------------------------------------------------------------
bool WinBase::OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard)
{
	if (!mVisibility.IsVisible())
		return false;

	if (mVisualOnlyUI)
		return false;

	if (mNoMouseEvent || mNoMouseEventAlone)
	{
		return false;
	}
	auto editor = gFBUIManager->GetUIEditor();
	if (editor)
	{		
		if (GetHwndId() != editor->GetHwndId())
		{
			if (!keyboard->IsKeyDown(VK_MENU))
				return false;
		}
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
				auto hwnd = gFBEnv->pEngine->GetWindowHandle(mHwndId);
				RECT rect;
				GetWindowRect(hwnd, &rect);
				OSWindowPos.x = rect.left;
				OSWindowPos.y = rect.top;
				//invalidate = true;
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
					if (OnEvent(UIEvents::EVENT_MOUSE_DRAG))
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
					if (OnEvent(UIEvents::EVENT_MOUSE_DOWN))
						mouse->Invalidate();
				}
			}
			else if (mMouseInPrev)
			{
				if (OnEvent(UIEvents::EVENT_MOUSE_HOVER))
					invalidate = true;
				ToolTipEvent(UIEvents::EVENT_MOUSE_HOVER, mousepos);
			}
			else if (mMouseIn)
			{
				if (OnEvent(UIEvents::EVENT_MOUSE_IN))
					invalidate = true;
				ToolTipEvent(UIEvents::EVENT_MOUSE_IN, mousepos);
			}

			if (mouse->IsLButtonClicked())
			{
				if (OnEvent(UIEvents::EVENT_MOUSE_LEFT_CLICK))
				{
					invalidate = true;
					mouse->Invalidate(GetType() == ComponentType::Button ? true : false);
				}
			}
			else if (mouse->IsLButtonDoubleClicked())
			{
				if (OnEvent(UIEvents::EVENT_MOUSE_LEFT_DOUBLE_CLICK))
				{
					invalidate = true;
					gFBEnv->pUIManager->CleanTooltip();
				}

			}
			else if (mouse->IsRButtonClicked())
			{
				if (OnEvent(UIEvents::EVENT_MOUSE_RIGHT_CLICK))
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
			if (OnEvent(UIEvents::EVENT_MOUSE_OUT))
			{
				mouse->Invalidate();
				TriggerRedraw();
			}
			ToolTipEvent(UIEvents::EVENT_MOUSE_OUT, mousepos);
		}

		mMouseInPrev = mMouseIn;
	}
	else if (mMouseInPrev)
	{
		if (OnEvent(UIEvents::EVENT_MOUSE_OUT))
		{
			mouse->Invalidate();
			TriggerRedraw();
		}
		ToolTipEvent(UIEvents::EVENT_MOUSE_OUT, mousepos);
		mMouseInPrev = false;
	}

	if (!GetFocus())
		return mMouseIn;

	if (keyboard->IsValid() && gFBUIManager->GetKeyboardFocusUI() == this)
	{

		char c = (char)keyboard->GetChar();

		switch (c)
		{
		case VK_RETURN:
		{
			if (OnEvent(UIEvents::EVENT_ENTER))
			{
				keyboard->PopChar();
				TriggerRedraw();
			}
			break;
		}
		/*case VK_TAB:
		{
			if (GetType() == ComponentType::TextField){
				if (OnEvent(UIEvents::EVENT_ENTER))
				{
					TriggerRedraw();
				}
				break;
			}
		}*/
			
		}
	}

	return mMouseIn;
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
	sCursorOver = LoadCursor(0, IDC_HAND);
	sCursorAll = LoadCursor(0, IDC_SIZEALL);
	sCursorNESW = LoadCursor(0, IDC_SIZENESW);
	sCursorNWSE = LoadCursor(0, IDC_SIZENWSE);
	sCursorWE = LoadCursor(0, IDC_SIZEWE);
	sCursorNS = LoadCursor(0, IDC_SIZENS);
}
// static
void WinBase::FinalizeMouseCursor()
{

}
//static 
HCURSOR WinBase::GetMouseCursorOver()
{
	return sCursorOver;
}

void WinBase::AlignText()
{	
	if (mUIObject)
	{
		Vec2I offset(0, 0);
		switch(mTextAlignH)
		{
		case ALIGNH::LEFT:
			{
				offset.x += LEFT_GAP + mTextGap.x;
			}
			break;

		case ALIGNH::CENTER:
			{
				offset.x += Round(mSize.x * .5f - mTextWidth * .5f + mTextGap.x);
			}
			break;

		case ALIGNH::RIGHT:
			{
				offset.x += mSize.x - mTextWidth - mTextGap.y;
			}
			break;
		}

		switch (mTextAlignV)
		{
		case ALIGNV::TOP:
		{
			offset.y += Round(mTextSize);
		}
			break;
		case ALIGNV::MIDDLE:
		{
			offset.y += Round(mSize.y * .5f +
						((mTextSize * 0.5f) - (mTextSize * (mNumTextLines-1) * 0.5f)));
		}
			break;
		case ALIGNV::BOTTOM:
		{
			offset.y += mSize.y - BOTTOM_GAP;
		}
			break;
		}
		mUIObject->SetTextOffset(offset);
	}
}

void WinBase::CalcTextWidth()
{
	if (mTextw.empty())
	{
		mTextWidth = 0;
		return;
	}
	IFont* pFont = gFBEnv->pRenderer->GetFont();
	pFont->SetHeight(mTextSize);
	mTextWidth = (int)gFBEnv->pRenderer->GetFont()->GetTextWidth(
		(const char*)mTextw.c_str(), mTextw.size() * 2);
	pFont->SetBackToOrigHeight();

	if (mMatchSize && mTextWidth != 0)// && !mLockTextSizeChange)
	{
		//mLockTextSizeChange = true;
		ChangeSize(Vec2I((int)(mTextWidth + 4), mScaledSize.y));
		//mLockTextSizeChange = false;
	}
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
	{
		mUIObject->SetTextSize(mTextSize);
		mUIObject->SetText(szText);
	}

	CalcTextWidth();

	AlignText();
	TriggerRedraw();
}

const wchar_t* WinBase::GetText() const
{
	return mTextw.c_str();
}

int WinBase::GetTextEndPosLocal() const
{
	if (mUIObject){
		return mUIObject->GetTextOffset().x +  mTextWidth;
	}
	return 0;
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
	SetAnimPos(Vec2(0, 0));
	SetAnimScale(Vec2(1, 1));
}

bool WinBase::SetProperty(UIProperty::Enum prop, const char* val)
{
	assert(val);
	switch (prop)
	{
	case UIProperty::POS:
	{
							auto v = Split(val);
							assert(v.size() == 2);
							int x = ParseIntPosX(v[0]);
							int y = ParseIntPosY(v[1]);
							ChangePos(Vec2I(x, y));
							return true;
	}
	case UIProperty::POSX:
	{
							int x = ParseIntPosX(val);
							ChangePosX(x);
							 return true;
	}
	case UIProperty::POSY:
	{
							int y = ParseIntPosY(val);
							ChangePosY(y);
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
							  OnSizeChanged();
							  return true;
	}
	case UIProperty::NPOSY:
	{
							  float y = StringConverter::parseReal(val);
							  SetNPosY(y);
							  OnSizeChanged();
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

	case UIProperty::USE_NSIZEX:{
		mUseAbsoluteXSize = !StringConverter::parseBool(val);
		return true;
	}

	case UIProperty::USE_NSIZEY:{
		mUseAbsoluteYSize = !StringConverter::parseBool(val);
		return true;
	}

	case UIProperty::USE_NPOSX:{
		mUseAbsoluteXPos = !StringConverter::parseBool(val);
		return true;
	}

	case UIProperty::USE_NPOSY:{
		mUseAbsoluteYPos = !StringConverter::parseBool(val);
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
									  mTextSize = StringConverter::parseReal(val, 22.0f);
									  CalcTextWidth();
									  AlignText();
									  mUIObject->SetTextSize(mTextSize);
									  mFixedTextSize = true;
									  return true;
								  }
								  break;
	}

	case UIProperty::FIXED_TEXT_SIZE:
	{
										mFixedTextSize = _stricmp("true", val) == 0;
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
		auto vals = Split(val);
		if (vals.size()== 2){
			mTextGap = StringConverter::parseVec2I(val);
		}
		else{
			mTextGap.x = StringConverter::parseInt(val);
			mTextGap.y = mTextGap.x;
		}
		
		AlignText();
		return true;
	}
	case UIProperty::MATCH_SIZE:
	{
								   mMatchSize = _stricmp("true", val) == 0;
								   return true;
	}
	case UIProperty::NO_BACKGROUND:
	{
									  if (_stricmp("true", val) == 0)
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
							 if (mTextBeforeTranslated == val)
								 return true;
							 mTextBeforeTranslated = val;
							 std::string translated = TranslateText(val);							 

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

	case UIProperty::KEEP_UI_RATIO:
	{
		mKeepUIRatio = StringConverter::parseBool(val);
		return true;
	}

	case UIProperty::UI_RATIO:
	{
		mAspectRatio = StringConverter::parseReal(val);
		return true;
	}

	case UIProperty::TOOLTIP:
	{
								assert(val);
								if (mTooltipTextBeforeT == val)
									return true;
								mTooltipTextBeforeT = val;								
								auto msg = TranslateText(val);								
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

	case UIProperty::VISUAL_ONLY_UI:
	{
		mVisualOnlyUI = StringConverter::parseBool(val);
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
			int specialOrder = StringConverter::parseInt(val);
			SetSpecialOrder(specialOrder);
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
	if (!sSuppressPropertyWarning)
		Error(DEFAULT_DEBUG_ARG, FormatString("Not processed property(%s) found", UIProperty::ConvertToString(prop)));
	return false;
}

bool WinBase::GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly)
{
	assert(val);
	switch (prop)
	{
	case UIProperty::POS:
	{
		auto data = StringConverter::toString(mPos);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}
	case UIProperty::POSX:
	{
		auto data = StringConverter::toString(mPos.x);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}
	case UIProperty::POSY:
	{
		auto data = StringConverter::toString(mPos.y);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}
	case UIProperty::NPOS:
	{
		auto data = StringConverter::toString(mNPos);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}
	case UIProperty::NPOSX:
	{
		auto data = StringConverter::toString(mNPos.x);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}
	case UIProperty::NPOSY:
	{
		auto data = StringConverter::toString(mNPos.y);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}
	case UIProperty::OFFSETX:
	{
		auto data = StringConverter::toString(mAbsOffset.x);
		strcpy_s(val, bufsize, data.c_str());
		
		return true;
	}
	case UIProperty::OFFSETY:
	{
		auto data = StringConverter::toString(mAbsOffset.y);
		strcpy_s(val, bufsize, data.c_str());		
		return true;
	}
	case UIProperty::SIZE:
	{
		auto data = StringConverter::toString(mSize);
		strcpy_s(val, bufsize, data.c_str());
		
		return true;
	}
	case UIProperty::SIZEX:
	{
		auto data = StringConverter::toString(mSize.x);
		strcpy_s(val, bufsize, data.c_str());
		
		return true;
	}
	case UIProperty::SIZEY:
	{
		auto data = StringConverter::toString(mSize.y);
		strcpy_s(val, bufsize, data.c_str());
		
		return true;
	}
	case UIProperty::NSIZEX:
	{
		auto data = mFillX ? std::string("fill") : StringConverter::toString(mNSize.x);
		strcpy_s(val, bufsize, data.c_str());
				
		return true;
	}
	case UIProperty::NSIZEY:
	{
		auto data = mFillY ? std::string("fill") : StringConverter::toString(mNSize.y);
		strcpy_s(val, bufsize, data.c_str());
		
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
		strcpy_s(val, bufsize, data.c_str());
		
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
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}

	case UIProperty::FIXED_TEXT_SIZE:
	{
		if (notDefaultOnly) {
			if (mFixedTextSize == UIProperty::GetDefaultValueBool(prop)) {
				return false;
			}
		}
		auto data = StringConverter::toString(mFixedTextSize);
		strcpy_s(val, bufsize, data.c_str());

		return true;
	}

	case UIProperty::TEXT_ALIGN:
	{
		if (notDefaultOnly) {
			if (mTextAlignH == UIProperty::GetDefaultValueInt(prop)) {
				return false;
			}
		}

		strcpy_s(val, bufsize, ALIGNH::ConvertToString(mTextAlignH));

		return true;
	}
	case UIProperty::TEXT_VALIGN:
	{
		if (notDefaultOnly) {
			if (mTextAlignV == UIProperty::GetDefaultValueInt(prop)) {
				return false;
			}
		}

		strcpy_s(val, bufsize, ALIGNV::ConvertToString(mTextAlignV));
		
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
		strcpy_s(val, bufsize, data.c_str());
		
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
		strcpy_s(val, bufsize, data.c_str());

		return true;
	}

	case UIProperty::TEXT_GAP:
	{
		if (notDefaultOnly) {
			return false;
		}

		auto data = StringConverter::toString(mTextGap);
		strcpy_s(val, bufsize, data.c_str());
		
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
		strcpy_s(val, bufsize, data.c_str());
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
		strcpy_s(val, bufsize, data.c_str());
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
		strcpy_s(val, bufsize, data.c_str());
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
		strcpy_s(val, bufsize, data.c_str());
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
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}
	case UIProperty::ALIGNH:
	{
		if (notDefaultOnly) {
			if (mAlignH == UIProperty::GetDefaultValueInt(prop)) {
				return false;
			}
		}

		strcpy_s(val, bufsize, ALIGNH::ConvertToString(mAlignH));
		
		return true;
	}
	case UIProperty::ALIGNV:
	{
		if (notDefaultOnly) {
			if (mAlignV == UIProperty::GetDefaultValueInt(prop)) {
				return false;
			}
		}

		strcpy_s(val, bufsize, ALIGNV::ConvertToString(mAlignV));
		return true;
	}
	case UIProperty::TEXT:
	{
		if (notDefaultOnly) {
			if (mTextw.empty() && mTextBeforeTranslated.empty())
				return false;
		}

		if (!mTextBeforeTranslated.empty()){
			strcpy_s(val, bufsize, mTextBeforeTranslated.c_str());
		}
		else
		{
			strcpy_s(val, bufsize, WideToAnsi(mTextw.c_str()));
		}
		return true;
	}

	case UIProperty::KEEP_UI_RATIO:
	{
		if (notDefaultOnly){
			if (mKeepUIRatio == UIProperty::GetDefaultValueBool(prop))
				return false;
		}
		auto data = StringConverter::toString(mKeepUIRatio);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}

	case UIProperty::UI_RATIO:
	{
		if (notDefaultOnly){
			if (mAspectRatio == UIProperty::GetDefaultValueFloat(prop))
				return false;
		}
		auto data = StringConverter::toString(mAspectRatio);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}

	case UIProperty::TOOLTIP:
	{
		if (notDefaultOnly) {
			if (mTooltipTextBeforeT.empty() && mTooltipText.empty())
				return false;
		}

		if (!mTooltipTextBeforeT.empty())
		{
			strcpy_s(val, bufsize, mTooltipTextBeforeT.c_str());
		}
		else
		{
			strcpy_s(val, bufsize, WideToAnsi(mTooltipText.c_str()));
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
		strcpy_s(val, bufsize, data.c_str());
		
		return true;
	}

	case UIProperty::NO_MOUSE_EVENT_ALONE:
	{
		if (notDefaultOnly) {
			if (mNoMouseEventAlone == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		auto data = StringConverter::toString(mNoMouseEventAlone);
		strcpy_s(val, bufsize, data.c_str());	
		return true;
	}

	case UIProperty::VISUAL_ONLY_UI:
	{
		if (notDefaultOnly) {
			if (mVisualOnlyUI == UIProperty::GetDefaultValueBool(prop))
				return false;
		}
		strcpy_s(val, bufsize, StringConverter::toString(mVisualOnlyUI).c_str());
		return true;
	}

	case UIProperty::USE_SCISSOR:
	{
		if (notDefaultOnly) {
			if (mUseScissor == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		auto data = StringConverter::toString(mUseScissor);
		strcpy_s(val, bufsize, data.c_str());

		return true;
	}

	case UIProperty::USE_BORDER:
	{
		if (notDefaultOnly) {
			if (!mBorders.empty() == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		auto data = StringConverter::toString(!mBorders.empty());
		strcpy_s(val, bufsize, data.c_str());
		
		return true;
	}

	case UIProperty::SPECIAL_ORDER:
	{
		if (notDefaultOnly) {
			if (mSpecialOrder == UIProperty::GetDefaultValueInt(prop))
				return false;
		}

		auto data = StringConverter::toString(mSpecialOrder);
		strcpy_s(val, bufsize, data.c_str());

		return true;
	}

	case UIProperty::INHERIT_VISIBLE_TRUE:
	{
		if (notDefaultOnly) {
			if (mInheritVisibleTrue == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		auto data = StringConverter::toString(mInheritVisibleTrue);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}

	case UIProperty::VISIBLE:
	{
		if (notDefaultOnly) {
			if (mVisibility.IsVisible() == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		auto data = StringConverter::toString(mVisibility.IsVisible());
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}

	case UIProperty::ENABLED:
	{
		if (notDefaultOnly) {
			if (mEnable == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		auto data = StringConverter::toString(mEnable);
		strcpy_s(val, bufsize, data.c_str());
		
		return true;
	}

	case UIProperty::MODAL:
	{
		if (notDefaultOnly) {
			if (mModal == UIProperty::GetDefaultValueBool(prop))
				return false;
		}
		auto data = StringConverter::toString(mModal);
		strcpy_s(val, bufsize, data.c_str());

		return true;
	}

	case UIProperty::DRAGABLE:
	{
		if (notDefaultOnly) {
			if (mDragable == UIProperty::GetDefaultValueVec2I(prop))
				return false;
		}

		auto data = StringConverter::toString(mDragable);
		strcpy_s(val, bufsize, data.c_str());
		
		return true;
	}

	case UIProperty::TAB_ORDER:
	{
		if (notDefaultOnly) {
			if (mTabOrder == UIProperty::GetDefaultValueInt(prop))
				return false;
		}

		auto data = StringConverter::toString(mTabOrder);
		strcpy_s(val, bufsize, data.c_str());

		return true;
	}
	}
	val = "";
	return false;
}

bool WinBase::GetPropertyAsBool(UIProperty::Enum prop, bool defaultVal)
{
	char buf[256];
	bool get = GetProperty(prop, buf, 256, false);
	if (get)
	{
		return StringConverter::parseBool(buf);
	}
	return defaultVal;
}

float WinBase::GetPropertyAsFloat(UIProperty::Enum prop, float defaultVal)
{
	char buf[256];
	bool get = GetProperty(prop, buf, 256, false);
	if (get)
	{
		return StringConverter::parseReal(buf);
	}
	return defaultVal;
}

int WinBase::GetPropertyAsInt(UIProperty::Enum prop, int defaultVal)
{
	char buf[256];
	bool get = GetProperty(prop, buf, 256, false);
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
		RecreateBorders();
	}
	else if (!use && !mBorders.empty())
	{
		for (auto ib : mBorders)
		{
			gFBEnv->pUIManager->DeleteComponent(ib);
		}
		mBorders.clear();
		
		if (mUIObject){
			auto mat = mUIObject->GetMaterial();
			if (mat && mat->RemoveShaderDefine("_ALPHA_TEXTURE"))
				mat->ApplyShaderDefines();
		}

		gFBEnv->pUIManager->DirtyRenderList(GetHwndId());
	}
}
void WinBase::RecreateBorders(){
	if (!mBorders.empty()){

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
		const char* atts[] = {
			"t", "l", "r", "b", "lt", "rt", "lb", "rb"
		};
		const char* uixmlPath = "es/textures/ui.xml";
		for (auto i = 0; i < ORDER_NUM; ++i){
			const char* borderRegion = gFBUIManager->GetBorderRegion(atts[i]);
			const auto& size = mBorders[i]->SetTextureAtlasRegion(uixmlPath, borderRegion);
			mBorders[i]->ChangeSize(size);
		}
	}
	else{
		const char* uixmlPath = "es/textures/ui.xml";
		ImageBox* T = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
		T->SetHwndId(GetHwndId());
		mBorders.push_back(T);
		T->SetRender3D(mRender3D, GetRenderTargetSize());
		T->SetManualParent(this);
		T->SetProperty(UIProperty::KEEP_IMAGE_RATIO, "false");
		const auto& sizeT = T->SetTextureAtlasRegion(uixmlPath,
			gFBUIManager->GetBorderRegion("t"));
		T->ChangeSizeY(sizeT.y);

		ImageBox* L = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
		L->SetHwndId(GetHwndId());
		mBorders.push_back(L);
		L->SetRender3D(mRender3D, GetRenderTargetSize());
		L->SetManualParent(this);
		L->SetProperty(UIProperty::KEEP_IMAGE_RATIO, "false");
		const auto& sizeL = L->SetTextureAtlasRegion(uixmlPath,
			gFBUIManager->GetBorderRegion("l"));
		L->ChangeSizeX(sizeL.x);


		ImageBox* R = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
		R->SetHwndId(GetHwndId());
		mBorders.push_back(R);
		R->SetRender3D(mRender3D, GetRenderTargetSize());
		R->SetManualParent(this);
		R->SetProperty(UIProperty::KEEP_IMAGE_RATIO, "false");
		R->SetAlign(ALIGNH::RIGHT, ALIGNV::TOP);
		const auto& sizeR = R->SetTextureAtlasRegion(uixmlPath,
			gFBUIManager->GetBorderRegion("r"));
		R->ChangeSizeX(sizeR.x);

		ImageBox* B = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
		B->SetHwndId(GetHwndId());
		mBorders.push_back(B);
		B->SetRender3D(mRender3D, GetRenderTargetSize());
		B->SetManualParent(this);
		B->SetProperty(UIProperty::KEEP_IMAGE_RATIO, "false");
		B->SetAlign(ALIGNH::LEFT, ALIGNV::BOTTOM);
		const auto& sizeB = B->SetTextureAtlasRegion(uixmlPath,
			gFBUIManager->GetBorderRegion("b"));
		B->ChangeSizeY(sizeB.y);

		ImageBox* LT = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
		LT->SetHwndId(GetHwndId());
		mBorders.push_back(LT);
		LT->SetRender3D(mRender3D, GetRenderTargetSize());
		LT->SetManualParent(this);
		const auto& sizeLt = LT->SetTextureAtlasRegion(uixmlPath,
			gFBUIManager->GetBorderRegion("lt"));
		LT->ChangeSize(sizeLt);

		ImageBox* RT = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
		RT->SetHwndId(GetHwndId());
		mBorders.push_back(RT);
		RT->SetRender3D(mRender3D, GetRenderTargetSize());
		RT->SetManualParent(this);
		RT->SetAlign(ALIGNH::RIGHT, ALIGNV::TOP);
		auto sizeRT = RT->SetTextureAtlasRegion(uixmlPath,
			gFBUIManager->GetBorderRegion("rt"));
		RT->ChangeSize(sizeRT);

		ImageBox* LB = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);;
		LB->SetHwndId(GetHwndId());
		mBorders.push_back(LB);
		LB->SetRender3D(mRender3D, GetRenderTargetSize());
		LB->SetManualParent(this);
		LB->SetAlign(ALIGNH::LEFT, ALIGNV::BOTTOM);
		const auto& sizeLB = LB->SetTextureAtlasRegion(uixmlPath,
			gFBUIManager->GetBorderRegion("lb"));
		LB->ChangeSize(sizeLB);

		ImageBox* RB = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
		RB->SetHwndId(GetHwndId());
		mBorders.push_back(RB);
		RB->SetRender3D(mRender3D, GetRenderTargetSize());
		RB->SetManualParent(this);
		RB->SetAlign(ALIGNH::RIGHT, ALIGNV::BOTTOM);
		const auto& sizeRB = RB->SetTextureAtlasRegion(uixmlPath,
			gFBUIManager->GetBorderRegion("rb"));
		RB->ChangeSize(sizeRB);
	}
	RefreshBorder();
	RefreshScissorRects();
	gFBEnv->pUIManager->DirtyRenderList(GetHwndId());

	auto visible = mVisibility.IsVisible();
	for (auto borderImage : mBorders)
	{
		borderImage->SetVisible(visible);
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

	const Vec2I& finalPos = GetFinalPos();
	const Vec2I& finalSize = GetFinalSize();
	const auto& ltsize = mBorders[ORDER_LT]->GetSize();
	mBorders[ORDER_T]->ChangeSizeX(std::max(0, finalSize.x - ltsize.x* 2));
	mBorders[ORDER_T]->ChangeWPos(Vec2I(finalPos.x + ltsize.x, finalPos.y));

	mBorders[ORDER_L]->ChangeSizeY(std::max(0, finalSize.y - ltsize.y*2));
	mBorders[ORDER_L]->ChangeWPos(Vec2I(finalPos.x, finalPos.y + ltsize.y));

	const auto& rtsize = mBorders[ORDER_RT]->GetSize();
	mBorders[ORDER_R]->ChangeSizeY(std::max(0, finalSize.y - rtsize.y * 2));
	Vec2I wpos = finalPos;
	wpos.x += finalSize.x;
	wpos.y += rtsize.y;
	mBorders[ORDER_R]->ChangeWPos(wpos);

	auto lbsize = mBorders[ORDER_LB]->GetSize();
	mBorders[ORDER_B]->ChangeSizeX(std::max(0, finalSize.x - lbsize.x*2));
	wpos = finalPos;
	wpos.y += finalSize.y;
	wpos.x += lbsize.x;
	mBorders[ORDER_B]->ChangeWPos(wpos);

	mBorders[ORDER_LT]->ChangeWPos(finalPos);

	wpos = finalPos;
	wpos.x += finalSize.x;
	mBorders[ORDER_RT]->ChangeWPos(wpos);

	wpos = finalPos;
	wpos.y += finalSize.y;
	mBorders[ORDER_LB]->ChangeWPos(wpos);

	wpos = finalPos + finalSize;
	mBorders[ORDER_RB]->ChangeWPos(wpos);

	UpdateAlphaTexture();
	
	
}

void WinBase::UpdateAlphaTexture(){
	mUpdateAlphaTexture = false;

	const Vec2I& finalSize = GetFinalSize();
	if (mUIObject){
		auto mat = mUIObject->GetMaterial();
		bool callmeLater = false;
		auto texture = gFBUIManager->GetBorderAlphaInfoTexture(finalSize, callmeLater);
		if (texture && mat){
			mat->SetTexture(texture, BINDING_SHADER_PS, 1);
			if (mat->AddShaderDefine("_ALPHA_TEXTURE", "1"))
				mat->ApplyShaderDefines();

			if (!mUIObject->HasTexCoord()){
				Vec2 texcoords[4] = {
					Vec2(0.f, 1.f),
					Vec2(0.f, 0.f),
					Vec2(1.f, 1.f),
					Vec2(1.f, 0.f)
				};
				mUIObject->SetTexCoord(texcoords, 4);
				INPUT_ELEMENT_DESCS descs;
				descs.push_back(INPUT_ELEMENT_DESC("POSITION", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0));
				descs.push_back(INPUT_ELEMENT_DESC("COLOR", 0, INPUT_ELEMENT_FORMAT_UBYTE4, 1, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0));
				descs.push_back(INPUT_ELEMENT_DESC("TEXCOORD", 0, INPUT_ELEMENT_FORMAT_FLOAT2, 2, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0));
				mat->SetInputLayout(descs);
			}
		}
		else{
			if (mat->RemoveShaderDefine("_ALPHA_TEXTURE"))
				mat->ApplyShaderDefines();

			if (!texture && callmeLater){
				mUpdateAlphaTexture = true;
			}
		}
	}
}

void WinBase::SetWNScollingOffset(const Vec2& offset)
{
	assert(GetType() != ComponentType::Scroller);
	mWNScrollingOffset = offset;
	OnPosChanged(false);
}

void WinBase::SetAnimScale(const Vec2& scale)
{
	mAnimScale = scale;
	OnSizeChanged();
}

void WinBase::SetAnimPos(const Vec2& pos){
	mAnimPos = pos;
	OnPosChanged(true);
}

//Vec2 WinBase::GetPivotWNPos()
//{
//	if (!mPivot && mParent)
//	{
//		return mParent->GetPivotWNPos();
//	}
//
//	assert(mUIObject);
//	return mUIObject->GetNPos() + mUIObject->GetAnimNPosOffset() + mUIObject->GetNSize()*0.5f;
//}

//void WinBase::SetPivotToUIObject(const Vec2& pivot)
//{
//	if(mUIObject)
//		mUIObject->SetPivot(pivot);
//}

const RECT& WinBase::GetRegion() const
{
	static RECT r;
	const auto& finalPos = GetFinalPos();
	const auto& finalSize = GetFinalSize();
	r.left = finalPos.x;
	r.top = finalPos.y;
	r.right = r.left + finalSize.x;
	r.bottom = r.top + finalSize.y;

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
	if (mUpdateAlphaTexture){
		UpdateAlphaTexture();
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
		SetAnimPos(evaluatedPos);
	if (hasScale)
		SetAnimScale(evaluatedScale);

	if (mHighlightSpeed > 0.f)
	{
		ProcessHighlight(elapsedTime);
	}

	mVisibility.Update(elapsedTime);
}

bool WinBase::ParseXML(tinyxml2::XMLElement* pelem)
{
	assert(pelem);
	auto rtSize = GetRenderTargetSize();
	const char* sz = pelem->Attribute("name");
	if (sz)
		SetName(sz);

	sz = pelem->Attribute("useNPosX");
	if (sz){
		mUseAbsoluteXPos = !StringConverter::parseBool(sz);
	}
	sz = pelem->Attribute("useNPosY");
	if (sz){
		mUseAbsoluteYPos = !StringConverter::parseBool(sz);
	}
	sz = pelem->Attribute("useNSizeX");
	if (sz){
		mUseAbsoluteXSize = !StringConverter::parseBool(sz);
	}
	sz = pelem->Attribute("useNSizeY");
	if (sz){
		mUseAbsoluteYSize = !StringConverter::parseBool(sz);
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
			SetPosWithTranslator(pos);
		}
	}		
	sz = pelem->Attribute("posX");
	if (sz)
	{
		int x = ParseIntPosX(sz);
		SetPosWithTranslatorX(x);
	}

	sz = pelem->Attribute("posY");
	if (sz)
	{
		int y = ParseIntPosY(sz);
		SetPosWithTranslatorY(y);
	}

	sz = pelem->Attribute("npos");
	if (sz)
	{
		Vec2 npos = StringConverter::parseVec2(sz);
		SetNPos(npos);
	}
	sz = pelem->Attribute("nposX");
	if (sz)
		SetNPosX(StringConverter::parseReal(sz));

	sz = pelem->Attribute("nposY");
	if (sz)
		SetNPosY(StringConverter::parseReal(sz));

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
		auto data = Split(sz);
		data[0] = StripBoth(data[0].c_str());
		data[1] = StripBoth(data[1].c_str());
		float x, y;
		if (_stricmp(data[0].c_str(), "fill") == 0){
			x = 1.0f - mNPos.x;
			mFillX = true;
		}
		else
			x = StringConverter::parseReal(data[0].c_str());

		if (_stricmp(data[1].c_str(), "fill") == 0) {
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
		if (_stricmp(sz, "fill") == 0)
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
		if (_stricmp(sz, "fill") == 0)
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
		mAspectRatioSet = true;
		mAspectRatio = StringConverter::parseReal(sz);
		int sizeY = (int)(mSize.x / mAspectRatio);
		SetSizeY(sizeY);
	}

	// size mod
	sz = pelem->Attribute("sizeMod");
	if (sz)
	{
		Vec2I sizeMod = StringConverter::parseVec2I(sz);
		ModifySize(sizeMod);
	}
	sz = pelem->Attribute("sizeModX");
	if (sz)
	{
		int x = StringConverter::parseInt(sz);
		ModifySize(Vec2I(x, 0));
	}

	sz = pelem->Attribute("sizeModY");
	if (sz)
	{
		int y = StringConverter::parseInt(sz);
		ModifySize(Vec2I(0, y));
	}

	OnSizeChanged();
	OnPosChanged(false);

	for (int i = UIProperty::BACK_COLOR; i < UIProperty::COUNT; ++i)
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
			UIEvents::Enum e = UIEvents::ConvertToEnum(eventElem->Name());
			if (e != UIEvents::EVENT_NUM)
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

//---------------------------------------------------------------------------
bool WinBase::ParseLua(const fastbird::LuaObject& compTable)
{
	assert(compTable.IsTable());
	bool success;
	auto rtSize = GetRenderTargetSize();
	std::string name = compTable.GetField("name").GetString(success);
	if (success)
		SetName(name.c_str());

	bool b = compTable.GetField("useNPosX").GetBool(success);
	if (success){
		mUseAbsoluteXPos = !b;
	}
	b = compTable.GetField("useNPosY").GetBool(success);
	if (success){
		mUseAbsoluteYPos = !b;
	}
	b = compTable.GetField("useNSizeX").GetBool(success);
	if (success){
		mUseAbsoluteXSize = !b;
	}
	b = compTable.GetField("useNSizeY").GetBool(success);
	if (success){
		mUseAbsoluteYSize = !b;
	}

	auto npos = compTable.GetField("npos").GetVec2(success);
	if (success)
		SetNPos(npos);

	// positions
	auto pos = compTable.GetField("pos").GetVec2I(success);
	if (success)
	{
		sLastPos = pos;
		SetPosWithTranslator(pos);
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
		SetPosWithTranslatorX(x);
	}

	int y = compTable.GetField("posY").GetInt(success);
	if (success)
	{
		sLastPos.y = y;
		SetPosWithTranslatorY(y);
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
			if (success && _stricmp(str.c_str(), "fill") == 0)
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
			if (_stricmp(str.c_str(), "fill") == 0)
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
		mAspectRatioSet = true;
		int sizeY = Round(mSize.x / mAspectRatio);
		SetSizeY(sizeY);
	}

	// size mod
	Vec2I sizeMod = compTable.GetField("sizeMod").GetVec2I(success);
	if (success)
	{
		ModifySize(sizeMod);
	}
	{
		int x = compTable.GetField("sizeModX").GetInt(success);
		if (success) {
			ModifySize(Vec2I(x, 0));
		}

		int y = compTable.GetField("sizeModY").GetInt(success);
		if (success) {
			ModifySize(Vec2I(0, y));
		}
	}

	OnSizeChanged();
	OnPosChanged(false);

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
				UIEvents::Enum e = UIEvents::ConvertToEnum(eventName.c_str());
				RegisterEventLuaFunc(e, funcName.c_str());
			}
		}
	}

	return true;
}

void WinBase::Save(tinyxml2::XMLElement& elem)
{
	elem.SetAttribute("name", mName.c_str());
	elem.SetAttribute("type", ComponentType::ConvertToString(GetType()));
	elem.SetAttribute("useNPosX", !mUseAbsoluteXPos);
	elem.SetAttribute("useNPosY", !mUseAbsoluteYPos);
	elem.SetAttribute("useNSizeX", !mUseAbsoluteXSize);
	elem.SetAttribute("useNSizeY", !mUseAbsoluteYSize);

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
		
	for (int i = UIProperty::BACK_COLOR; i < UIProperty::COUNT; ++i)
	{
		char buf[256];
		auto got = GetProperty(UIProperty::Enum(i), buf, 256, true);
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
			auto eElem = eventsElem->GetDocument()->NewElement(UIEvents::ConvertToString(it.first));
			eventsElem->InsertEndChild(eElem);
			auto text = eElem->GetDocument()->NewText(it.second.c_str());
			eElem->InsertEndChild(text);
		}
	}
}



float WinBase::GetTextBottomGap() const
{
	auto rtSize = GetRenderTargetSize();
	float yGap = 2.0f / (float)rtSize.y;
	return yGap;
}


void WinBase::ToolTipEvent(UIEvents::Enum evt, const Vec2& mouseNPos)
{
	if (mTooltipText.empty())
		return;

	switch (evt)
	{
	case UIEvents::EVENT_MOUSE_IN:
	{
										  gFBEnv->pUIManager->SetTooltipString(mTooltipText);

										  gFBEnv->pUIManager->SetTooltipPos(mouseNPos);
										  mShowingTooltip = true;
	}
		break;

	case UIEvents::EVENT_MOUSE_HOVER:
		gFBEnv->pUIManager->SetTooltipString(mTooltipText);
		gFBEnv->pUIManager->SetTooltipPos(mouseNPos);
		mShowingTooltip = true;
		break;

	case UIEvents::EVENT_MOUSE_OUT:
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
		const auto& myRegion = GetRegion();
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

	//UpdateWorldSize();
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

Vec2I WinBase::GetParentSize() const{
	if (mParent){
		return mParent->GetSize();
	}
	else{
		return GetRenderTargetSize();
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
	Move(Vec2I(dx, dy));
	
	// for color ramp
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

IWinBase* WinBase::WinBaseWithPoint(const Vec2I& pt, const RegionTestParam& param) const
{
	if (param.mNoRuntimeComp && mRunTimeChild)
		return 0;

	if (IsIn(pt, param.mIgnoreScissor))
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

void WinBase::GetBiggestTabOrder(int& curBiggest) const{
	if (curBiggest < mTabOrder){
		curBiggest = mTabOrder;
	}
}

void WinBase::TabPressed(){
	if (mParent){
		mParent->TabPressed();
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

float WinBase::GetContentHeight() const
{
	return GetFinalSize().y / (float)GetRenderTargetSize().y;
}

bool WinBase::IsKeyboardFocused() const
{
	return gFBUIManager->GetKeyboardFocusUI() == this;
}


void WinBase::SetEvent(UIEvents::Enum e, const char* luaFuncName){
	assert(luaFuncName);
	if (e != UIEvents::EVENT_NUM)
	{
		if (strlen(luaFuncName) == 0){
			UnregisterEventLuaFunc(e);
			auto it = mEventFuncNames.Find(e);
			if (it != mEventFuncNames.end()){
				mEventFuncNames.erase(it);
			}
		}
		else{
			RegisterEventLuaFunc(e, luaFuncName);
			mEventFuncNames[e] = luaFuncName;
		}
	}
}

const char* WinBase::GetEvent(UIEvents::Enum e){
	assert(e < UIEvents::EVENT_NUM);
	auto it = mEventFuncNames.Find(e);
	if (it != mEventFuncNames.end()){
		return it->second.c_str();
	}
	return "";
}

void WinBase::SetSpecialOrder(int specialOrder){
	mSpecialOrder = specialOrder;
	if (mUIObject){
		mUIObject->SetSpecialOrder(specialOrder);
	}
	gFBUIManager->DirtyRenderList(GetHwndId());
}
}