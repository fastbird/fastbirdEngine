#include <UI/StdAfx.h>
#include <UI/WinBase.h>
#include <UI/IUIManager.h>
#include <UI/Container.h>
#include <CommonLib/StringUtils.h>

namespace fastbird
{
const float WinBase::LEFT_GAP = 0.001f;
const float WinBase::BOTTOM_GAP = 0.006f;
HCURSOR WinBase::mCursorOver = 0;

WinBase::WinBase()
	: mVisible(true)
	, mAlignH(ALIGNH::LEFT)
	, mAlignV(ALIGNV::TOP)
	, mParent(0)
	, mNPosAligned(0, 0)
	, mMouseIn(false)
	, mMouseInPrev(false)
	, mNext(0)
	, mPrev(0)
	, mTextColor(1.f, 1.f, 1.f, 1.f)
	, mTextColorHover(1.f, 1.0f, 0.f, 1.f)
	, mTextColorDown(1.0f, 0.5f, 0.0f, 1.f)
	, mTextAlignH(ALIGNH::LEFT)
	, mMatchSize(false)
	, mSize(10, 10)
	, mTextSize(30.0f)
	, mFixedTextSize(false)
	, mWNPosOffset(0, 0)
{
}

//---------------------------------------------------------------------------
void WinBase::SetName(const char* name)
{
	mName = name;
}

//---------------------------------------------------------------------------
void WinBase::SetSize(const fastbird::Vec2I& size)
{
	mSize = size;
	// internal calculation done always in normalized space
	SetNSize(ConvertToNormalized(size));
}

void WinBase::SetNSize(const fastbird::Vec2& size) // normalized size (0.0~1.0)
{
	mNSize = size;
	mSize = ConvertToScreen(size);
	UpdateWorldSize();
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
void WinBase::UpdateWorldSize()
{
	mWNSize = mNSize;
	if (mParent)
		mWNSize = mParent->ConvertChildSizeToWorldCoord(mWNSize);
}

//---------------------------------------------------------------------------
void WinBase::SetPos(const fastbird::Vec2I& pos)
{
	mPos = pos;
	// internal calculation done always in normalized space
	SetNPos(ConvertToNormalized(pos));
}

void WinBase::SetNPos(const fastbird::Vec2& pos) // normalized pos (0.0~1.0)
{	
	mNPos = pos;
	mPos = ConvertToScreen(pos);
	UpdateWorldPos();
}

void WinBase::UpdateWorldPos()
{
	UpdateAlignedPos();
	// mNPos and mPos has updated value.
	mWNPos = mNPosAligned;
	if (mParent)
		mWNPos = mParent->ConvertChildPosToWorldCoord(mNPosAligned);

	OnPosChanged();
}

//---------------------------------------------------------------------------
fastbird::Vec2I WinBase::ConvertToScreen(const fastbird::Vec2 npos) const
{
	Vec3 ndc( npos.x*2.f - 1.f,	npos.y*2.f - 1.f, 0.f);
	return gEnv->pRenderer->ToSreenPos(ndc);
}

fastbird::Vec2 WinBase::ConvertToNormalized(const fastbird::Vec2I pos) const
{
	Vec2 ndc = gEnv->pRenderer->ToNdcPos(pos);
	// convert to normalized space
	Vec2 npos((ndc.x + 1.f) * .5f, (-ndc.y + 1.f) * .5f);
	return npos;
}

//---------------------------------------------------------------------------
void WinBase::SetAlign(ALIGNH::Enum h, ALIGNV::Enum v)
{
	mAlignH = h;
	mAlignV = v;
	UpdateWorldPos();
}

void WinBase::UpdateAlignedPos()
{
	mNPosAligned = mNPos;
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
void WinBase::SetVisible(bool show)
{
	mVisible = show;
	IUIManager::GetUIManager().DirtyRenderList();
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

//---------------------------------------------------------------------------
void WinBase::OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard)
{
	if (!mVisible)
		return;
	if (mouse->IsValid())
	{
		// hit test
		
		Vec2 mousepos = mouse->GetNPos();
		float wx = mWNPos.x + mWNPosOffset.x;
		float wy = mWNPos.y + mWNPosOffset.y;
		mMouseIn = !(mousepos.x < wx ||
			mousepos.x > wx+mWNSize.x ||
			mousepos.y < wy ||
			mousepos.y > wy+mWNSize.y);

		if (mouse->IsValid())
		{
			if (mMouseIn)
			{
				if (mouse->IsLButtonDown() && mouse->IsDragStartIn(mUIObject->GetRegion()))
				{
					mouse->Invalidate();
					OnEvent(IEventHandler::EVENT_MOUSE_DOWN);
				}
				else if (mMouseInPrev)
				{
					OnEvent(IEventHandler::EVENT_MOUSE_HOVER);
				}
				else if (mMouseIn)
				{
					OnEvent(IEventHandler::EVENT_MOUSE_IN);
				}

				if (mouse->IsLButtonClicked())
				{
					mouse->Invalidate();
					OnEvent(EVENT_MOUSE_CLICK);
				}
				else if (mouse->IsLButtonDoubleClicked())
				{
					mouse->Invalidate();
					OnEvent(EVENT_MOUSE_DOUBLE_CLICK);
				}
			}
			else
			{
				if (mMouseInPrev)
				{
					mouse->Invalidate();
					OnEvent(IEventHandler::EVENT_MOUSE_OUT);
				}
			}
			

			mMouseInPrev = mMouseIn;
		}
	}

	if (!GetFocus())
		return;

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
}

IWinBase* WinBase::FocusTest(Vec2 normalizedMousePos)
{
	float wx = mWNPos.x + mWNPosOffset.x;
	float wy = mWNPos.y + mWNPosOffset.y;
	bool in = !(normalizedMousePos.x < wx ||
			normalizedMousePos.x > wx+mWNSize.x ||
			normalizedMousePos.y < wy ||
			normalizedMousePos.y > wy+mWNSize.y);
	IWinBase* ret = 0;
	in ? ret = this : ret = 0;
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
	IFont* pFont = gEnv->pRenderer->GetFont();
	if (mTextw.empty())
		return;

	pFont->SetHeight(mTextSize);
	int width = (int)gEnv->pRenderer->GetFont()->GetTextWidth(
		(const char*)mTextw.c_str(), mTextw.size()*2);
	pFont->SetBackToOrigHeight();
	float nwidth = ConvertToNormalized(Vec2I(width, width)).x;
	if (mMatchSize && width !=0)
	{ 
		SetSize(Vec2I((int)(width+LEFT_GAP*2.f), mSize.y));
	}
	if (mUIObject)
	{
		switch(mTextAlignH)
		{
		case ALIGNH::LEFT:
			{
				mUIObject->SetTextStartNPos(Vec2(mWNPos.x + LEFT_GAP, mWNPos.y + mWNSize.y - BOTTOM_GAP));
			}
			break;

		case ALIGNH::CENTER:
			{
				mUIObject->SetTextStartNPos(Vec2(mWNPos.x + mWNSize.x*.5f - nwidth*.5f, mWNPos.y + mWNSize.y - BOTTOM_GAP));
			}
			break;

		case ALIGNH::RIGHT:
			{
				mUIObject->SetTextStartNPos(Vec2(mWNPos.x + mWNSize.x - nwidth, mWNPos.y + mWNSize.y - BOTTOM_GAP));
			}
			break;
		}
	}
}

void WinBase::OnPosChanged()
{
	mUIObject->SetNPos(mWNPos);
	AlignText();
}

void WinBase::OnSizeChanged()
{
	mUIObject->SetNSize(mWNSize);

	if (!mFixedTextSize)
	{
		const RECT& region = mUIObject->GetRegion();
		mTextSize = (float)(region.bottom - region.top);
		AlignText();
		mUIObject->SetTextSize(mTextSize);
	}
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

	AlignText();
}

const wchar_t* WinBase::GetText() const
{
	return mTextw.c_str();
}

bool WinBase::SetProperty(Property prop, const char* val)
{
	if (prop == PROPERTY_BACK_COLOR) // for buttons handle themselves.
	{
		Color color;
		color = StringConverter::parseVec4(val);
		mUIObject->GetMaterial()->SetDiffuseColor(color.GetVec4());

		return true;
	}
	if (prop == PROPERTY_TEXT_SIZE)
	{
		mTextSize = StringConverter::parseReal(val, 20.0f);
		mUIObject->SetTextSize(mTextSize);
		return true;
	}

	if (prop==PROPERTY_FIXED_TEXT_SIZE)
	{
		mFixedTextSize = stricmp("true", val)==0;
		return true;
	}

	if (prop==PROPERTY_TEXT_ALIGN)
	{
		if (stricmp("left", val)==0)
		{
			mTextAlignH = ALIGNH::LEFT;
		}
		else if (stricmp("right", val)==0)
		{
			mTextAlignH = ALIGNH::RIGHT;
		}
		else // center
		{
			mTextAlignH = ALIGNH::CENTER;
		}
		return true;
	}

	if (prop==PROPERTY_MATCH_SIZE)
	{
		mMatchSize = stricmp("true", val)==0;
		return true;
	}

	if (prop==PROPERTY_NO_BACKGROUND)
	{
		if (stricmp("true", val)==0)
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

	if (prop==PROPERTY_TEXT_COLOR)
	{
		mTextColor = StringConverter::parseVec4(val);
		return true;
	}

	if (prop==PROPERTY_TEXT_COLOR_HOVER)
	{
		mTextColorHover = StringConverter::parseVec4(val);
		return true;
	}

	if (prop==PROPERTY_TEXT_COLOR_DOWN)
	{
		mTextColorDown = StringConverter::parseVec4(val);
		return true;
	}

	assert(0 && "Not processed property found");
	return false;
}

void WinBase::SetNPosOffset(const Vec2& offset)
{
	mWNPosOffset = offset;
	if (mUIObject)
		mUIObject->SetNPosOffset(offset);
}

void WinBase::SetScissorRect(bool use, const RECT& rect)
{
	if (mUIObject)
		mUIObject->SetUseScissor(use, rect);
}

const RECT& WinBase::GetRegion() const
{
	static RECT r;
	if (mUIObject)
		return mUIObject->GetRegion();

	assert(0);
	return r;
}

}