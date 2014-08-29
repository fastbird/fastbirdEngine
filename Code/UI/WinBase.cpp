#include <UI/StdAfx.h>
#include <UI/WinBase.h>
#include <UI/IUIManager.h>
#include <UI/Container.h>
#include <UI/UIAnimation.h>
#include <CommonLib/StringUtils.h>

namespace fastbird
{
const float WinBase::LEFT_GAP = 0.001f;
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
	, mTextAlignV(ALIGNV::BOTTOM)
	, mMatchSize(false)
	, mSize(10, 10)
	, mTextSize(30.0f)
	, mFixedTextSize(false)
	, mWNPosOffset(0, 0)
	, mMouseDragStartInHere(false)
	, mDestNPos(0, 0)
	, mAnimationEnabled(false)
	, mAnimationSpeed(0.f)
	, mAspectRatio(0)
	, mNPos(0, 0), mWNPos(0, 0)
	, mAnimation(0)
	, mUIObject(0)
{
}

WinBase::~WinBase()
{
	FB_SAFE_DEL(mAnimation);
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
	if (mVisible == show)
		return;
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
bool WinBase::IsIn(const Vec2& mouseNormpos)
{
	float wx = mWNPos.x + mWNPosOffset.x;
	float wy = mWNPos.y + mWNPosOffset.y;
	return !(mouseNormpos.x < wx ||
		mouseNormpos.x > wx + mWNSize.x ||
		mouseNormpos.y < wy ||
		mouseNormpos.y > wy + mWNSize.y);
}

//---------------------------------------------------------------------------
bool WinBase::OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard)
{
	if (!mVisible)
		return false;
	mMouseIn = false;
	if (mouse->IsValid())
	{
		// hit test
		
		Vec2 mousepos = mouse->GetNPos();
		mMouseIn = IsIn(mousepos);

		if (mouse->IsValid())
		{
			if (mMouseIn)
			{
				if (mouse->IsLButtonDown() && !mouse->IsLButtonDownPrev() && mUIObject && mouse->IsDragStartIn(mUIObject->GetRegion()))
				{
					mMouseDragStartInHere = true;
				}
				else
				{
					mMouseDragStartInHere = false;
				}
				if (mouse->IsLButtonDown() && mMouseDragStartInHere)
				{
					if (OnEvent(IEventHandler::EVENT_MOUSE_DOWN))
						mouse->Invalidate();
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
					if (OnEvent(EVENT_MOUSE_CLICK))
						mouse->Invalidate();
				}
				else if (mouse->IsLButtonDoubleClicked())
				{
					if (OnEvent(EVENT_MOUSE_DOUBLE_CLICK))
						mouse->Invalidate();
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

IWinBase* WinBase::FocusTest(Vec2 normalizedMousePos)
{
	IWinBase* ret = 0;
	IsIn(normalizedMousePos) ? ret = this : ret = 0;
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
		Vec2 startPos = mWNPos;
		switch(mTextAlignH)
		{
		case ALIGNH::LEFT:
			{
							 startPos.x = mWNPos.x + LEFT_GAP;				
			}
			break;

		case ALIGNH::CENTER:
			{
							   startPos.x = mWNPos.x + mWNSize.x*.5f - nwidth*.5f;
			}
			break;

		case ALIGNH::RIGHT:
			{
							  startPos.x = mWNPos.x + mWNSize.x - nwidth;
			}
			break;
		}

		switch (mTextAlignV)
		{
		case ALIGNV::TOP:
		{
							startPos.y = mWNPos.y + (mTextSize / (float)gEnv->pRenderer->GetHeight());
		}
			break;
		case ALIGNV::MIDDLE:
		{
							   startPos.y = mWNPos.y + mWNSize.y*.5f + (mTextSize / (float)gEnv->pRenderer->GetHeight())*.4f;
		}
			break;
		case ALIGNV::BOTTOM:
		{
							   startPos.y = mWNPos.y + mWNSize.y - GetTextBottomGap();
		}
			break;
		}
		mUIObject->SetTextStartNPos(startPos);
	}
}

void WinBase::OnPosChanged()
{
	if (mUIObject)
		mUIObject->SetNPos(mWNPos);
	AlignText();

	if (mParent)
		mParent->OnChildPosSizeChanged(this);
}

void WinBase::OnSizeChanged()
{
	if (mUIObject)
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

	if (mParent)
		mParent->OnChildPosSizeChanged(this);
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

IUIAnimation* WinBase::GetUIAnimation()
{
	if (!mAnimation)
	{
		mAnimation = FB_NEW(UIAnimation);
	}
	return mAnimation;		
}

bool WinBase::SetProperty(UIProperty::Enum prop, const char* val)
{
	assert(val);
	switch(prop)
	{
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
									   mUIObject->SetTextSize(mTextSize);
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
									return true;
		}
	case UIProperty::TEXT_VALIGN:
	{
									mTextAlignV = ALIGNV::ConvertToEnum(val);
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
								  if (mUIObject)
									  mUIObject->SetAlphaBlending(true);
								  return true;
		}
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

void WinBase::SetAnimNPosOffset(const Vec2& offset)
{
	mWNAnimPosOffset = offset;
	if (mUIObject)
		mUIObject->SetAnimNPosOffset(offset);
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

void WinBase::PosAnimationTo(const Vec2& destNPos, float speed)
{
	if (destNPos != mNPos)
	{
		mAnimationEnabled = true;
		mDestNPos = destNPos;
		mAnimationSpeed = speed;
	}
}

void WinBase::OnStartUpdate(float elapsedTime)
{
	// Static Animation.
	if (mAnimationEnabled)
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
			mAnimationEnabled = false;
		}			
		SetNPos(mNPos + to*movement);
	}

	// Key Frame Animation
	if (mAnimation && mUIObject)
	{
		mAnimation->Update(elapsedTime);
		SetAnimNPosOffset(mAnimation->GetCurrentPos());
	}
}

void WinBase::SetAlphaBlending(bool set)
{
	if (mUIObject)
		mUIObject->SetAlphaBlending(set);
}

float WinBase::PixelToLocalNWidth(int pixel) const
{
	return pixel / ((float)gEnv->pRenderer->GetWidth() * mWNSize.x);
}
float WinBase::PixelToLocalNHeight(int pixel) const
{
	return pixel / ((float)gEnv->pRenderer->GetHeight() * mWNSize.y);
}

Vec2 WinBase::PixelToLocalNSize(const Vec2I& pixel) const
{
	return Vec2(PixelToLocalNWidth(pixel.x), PixelToLocalNWidth(pixel.y));
}

float WinBase::PixelToLocalNPosX(int pixel) const
{
	return pixel / (float)gEnv->pRenderer->GetWidth() * mWNSize.x;
}
float WinBase::PixelToLocalNPosY(int pixel) const
{
	return pixel / (float)gEnv->pRenderer->GetHeight() * mWNSize.y;
}

Vec2 WinBase::PixelToLocalNPos(const Vec2I& pixel) const
{
	return Vec2(PixelToLocalNPosX(pixel.x), PixelToLocalNPosX(pixel.y));
}

float WinBase::GetTextWidthLocal() const
{
	IFont* pFont = gEnv->pRenderer->GetFont();
	if (mTextw.empty())
		return 0.f;

	pFont->SetHeight(mTextSize);
	int width = (int)gEnv->pRenderer->GetFont()->GetTextWidth(
		(const char*)mTextw.c_str(), mTextw.size() * 2);
	pFont->SetBackToOrigHeight();
	if (mParent)
		return mParent->PixelToLocalNWidth(width);

	return width / (float)gEnv->pRenderer->GetWidth();
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
	Vec2 npos(0, 0);
	const char* sz = pelem->Attribute("npos");
	if (sz)
		npos = StringConverter::parseVec2(sz);

	// positions
	sz = pelem->Attribute("pos");
	if (sz)
	{
		Vec2I pos = StringConverter::parseVec2I(sz);
		if (mParent)
		{
			npos = mParent->PixelToLocalNPos(pos);
		}
		else
		{
			npos = Vec2(pos.x / (float)gEnv->pRenderer->GetWidth(), pos.y / (float)gEnv->pRenderer->GetHeight());
		}
	}		

	sz = pelem->Attribute("nposX");
	if (sz)
		npos.x = StringConverter::parseReal(sz);

	sz = pelem->Attribute("nposY");
	if (sz)
		npos.y = StringConverter::parseReal(sz);

	sz = pelem->Attribute("posX");
	if (sz)
	{
		int x = StringConverter::parseInt(sz);
		if (mParent)
			npos.x = mParent->PixelToLocalNPosX(x);
		else
			npos.x = (float)x / (float)gEnv->pRenderer->GetWidth();
	}

	sz = pelem->Attribute("posY");
	if (sz)
	{
		int y = StringConverter::parseInt(sz);
		if (mParent)
			npos.y = mParent->PixelToLocalNPosY(y);
		else
			npos.y = y / (float)gEnv->pRenderer->GetHeight();
	}

	//size
	Vec2 nsize(1.0f, 1.0f);
	sz = pelem->Attribute("nsize");
	if (sz)
	{
		nsize = StringConverter::parseVec2(sz);
	}

	sz = pelem->Attribute("nsizeX");
	if (sz)
	{
		if (stricmp(sz, "fill") == 0)
		{
			nsize.x = 1.0f - npos.x;
		}
		else
		{
			nsize.x = StringConverter::parseReal(sz);
		}
	}

	sz = pelem->Attribute("nsizeY");
	if (sz)
	{
		if (stricmp(sz, "fill") == 0)
		{
			nsize.y = 1.0f - npos.y;
		}
		else
		{
			nsize.y = StringConverter::parseReal(sz);
		}
	}

	sz = pelem->Attribute("sizeX");
	if (sz)
	{
		int x = StringConverter::parseInt(sz);
		if (mParent)
		{
			nsize.x = mParent->PixelToLocalNWidth(x);
		}
		else
		{
			nsize.x = x / (float)gEnv->pRenderer->GetWidth();
		}
	}

	sz = pelem->Attribute("sizeY");
	if (sz)
	{
		int y = StringConverter::parseInt(sz);
		if (mParent)
		{
			nsize.y = mParent->PixelToLocalNHeight(y);
		}
		else
		{
			nsize.y = y / (float)gEnv->pRenderer->GetHeight();
		}
	}

	float aspectRatio = 0.f;
	sz = pelem->Attribute("aspectRatio");
	if (sz)
	{
		aspectRatio = StringConverter::parseReal(sz);
		Vec2 worldSize = nsize;
		if (mParent)
		{
			float width = nsize.x;
			worldSize = mParent->ConvertChildSizeToWorldCoord(Vec2(width, width));
		}
		float iWidth = gEnv->pRenderer->GetWidth() * worldSize.x;
		float iHeight = iWidth / aspectRatio;
		float height = iHeight / (gEnv->pRenderer->GetHeight() * mWNSize.y);
		nsize.y = height;
	}

	SetNSize(nsize);
	SetNPos(npos);
	sz = pelem->Attribute("name");
	if (sz)
		SetName(sz);

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
	return true;
}

float WinBase::GetTextBottomGap() const
{
	float yGap = 2.0f / (float)gEnv->pRenderer->GetHeight();
	return yGap;
}

}