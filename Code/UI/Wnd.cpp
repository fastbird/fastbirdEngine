#include <UI/StdAfx.h>
#include <UI/Wnd.h>
#include <UI/IUIManager.h>
#include <UI/Button.h>
#include <UI/ImageBox.h>
#include <UI/Scroller.h>
#include <Engine/GlobalEnv.h>

namespace fastbird
{

Wnd::Wnd()
:mTitlebar(0)
, mUseFrame(false)
, mBackgroundImage(0)
{
	mUIObject = IUIObject::CreateUIObject(false);
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	mStopScissorParent = true;
	mUIObject->SetNoDrawBackground(true);
	RegisterEventFunc(IEventHandler::EVENT_MOUSE_HOVER,
		std::bind(&Wnd::MouseConsumer, this, std::placeholders::_1));
}

Wnd::~Wnd()
{
	FB_DELETE(mBackgroundImage);
	for (auto var : mFrames)
	{
		IUIManager::GetUIManager().DeleteComponent(var);
	}
	mFrames.clear();
	IUIManager::GetUIManager().DeleteComponent(mTitlebar);
}

void Wnd::GatherVisit(std::vector<IUIObject*>& v)
{
	if (!mVisible)
		return;

	v.push_back(mUIObject);
	
	if (mBackgroundImage)
		mBackgroundImage->GatherVisit(v);

	__super::GatherVisit(v);

	for (auto var : mFrames)
	{
		var->GatherVisit(v);
	}
	if (mTitlebar)
		mTitlebar->GatherVisit(v);
	
}

bool Wnd::OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard)
{
	if (!mVisible)
		return false;

	if (!mouse->IsValid() && !keyboard->IsValid())
		return false;
	bool mouseIn = false;
	if (mTitlebar)
		mouseIn = mTitlebar->OnInputFromHandler(mouse, keyboard);

	return __super::OnInputFromHandler(mouse, keyboard) || mouseIn;
}

void Wnd::OnSizeChanged()
{
	__super::OnSizeChanged();
	RefreshFrame();
}

void Wnd::OnPosChanged()
{
	__super::OnPosChanged();
	RefreshFrame();
}

void Wnd::RefreshFrame()
{
	if (mTitlebar)
	{
		mTitlebar->SetNSizeX(mWNSize.x);
		mTitlebar->SetNPos(mWNPos);
	}
	if (mUseFrame)
	{
		if (mFrames.empty())
		{
			ImageBox* T = (ImageBox*)IUIManager::GetUIManager().CreateComponent(ComponentType::ImageBox);
			T->SetSizeY(16);
			T->SetTextureAtlasRegion("es/textures/ui.xml", "Pane_T");
			T->SetManualParent(this);
			T->SetProperty(UIProperty::SPECIAL_ORDER, "1");
			T->SetVisible(true);
			mFrames.push_back(T);			

			ImageBox* L = (ImageBox*)IUIManager::GetUIManager().CreateComponent(ComponentType::ImageBox);
			L->SetSizeX(16);
			L->SetTextureAtlasRegion("es/textures/ui.xml", "Pane_L");
			L->SetManualParent(this);
			L->SetProperty(UIProperty::SPECIAL_ORDER, "1");
			L->SetVisible(true);
			mFrames.push_back(L);			

			ImageBox* R = (ImageBox*)IUIManager::GetUIManager().CreateComponent(ComponentType::ImageBox);
			R->SetSizeX(16);
			R->SetAlign(ALIGNH::RIGHT, ALIGNV::TOP);
			R->SetTextureAtlasRegion("es/textures/ui.xml", "Pane_R");
			R->SetManualParent(this);
			R->SetProperty(UIProperty::SPECIAL_ORDER, "1");
			R->SetVisible(true);
			mFrames.push_back(R);

			ImageBox* B = (ImageBox*)IUIManager::GetUIManager().CreateComponent(ComponentType::ImageBox);
			B->SetAlign(ALIGNH::LEFT, ALIGNV::BOTTOM);
			B->SetSizeY(20);
			B->SetTextureAtlasRegion("es/textures/ui.xml", "Pane_B");
			B->SetManualParent(this);
			B->SetProperty(UIProperty::SPECIAL_ORDER, "1");
			B->SetVisible(true);
			mFrames.push_back(B);

			ImageBox* LT = (ImageBox*)IUIManager::GetUIManager().CreateComponent(ComponentType::ImageBox);
			LT->SetSize(Vec2I(40, 44));
			LT->SetTextureAtlasRegion("es/textures/ui.xml", "Pane_LT");
			LT->SetManualParent(this);
			LT->SetProperty(UIProperty::SPECIAL_ORDER, "2");
			LT->SetVisible(true);
			mFrames.push_back(LT);

			ImageBox* RT = (ImageBox*)IUIManager::GetUIManager().CreateComponent(ComponentType::ImageBox);
			RT->SetSize(Vec2I(40, 44));
			RT->SetAlign(ALIGNH::RIGHT, ALIGNV::TOP);
			RT->SetTextureAtlasRegion("es/textures/ui.xml", "Pane_RT");
			RT->SetManualParent(this);
			RT->SetProperty(UIProperty::SPECIAL_ORDER, "2");
			RT->SetVisible(true);
			mFrames.push_back(RT);

			ImageBox* MT = (ImageBox*)IUIManager::GetUIManager().CreateComponent(ComponentType::ImageBox);
			MT->SetSize(Vec2I(302, 44));
			MT->SetAlign(ALIGNH::CENTER, ALIGNV::TOP);
			MT->SetTextureAtlasRegion("es/textures/ui.xml", "Pane_MT");
			MT->SetManualParent(this);
			MT->SetProperty(UIProperty::SPECIAL_ORDER, "2");
			MT->SetVisible(true);
			mFrames.push_back(MT);

			ImageBox* LB = (ImageBox*)IUIManager::GetUIManager().CreateComponent(ComponentType::ImageBox);
			LB->SetSize(Vec2I(40, 44));
			LB->SetAlign(ALIGNH::LEFT, ALIGNV::BOTTOM);
			LB->SetTextureAtlasRegion("es/textures/ui.xml", "Pane_LB");
			LB->SetManualParent(this);
			LB->SetProperty(UIProperty::SPECIAL_ORDER, "2");
			LB->SetVisible(true);
			mFrames.push_back(LB);

			ImageBox* RB = (ImageBox*)IUIManager::GetUIManager().CreateComponent(ComponentType::ImageBox);
			RB->SetSize(Vec2I(40, 44));
			RB->SetAlign(ALIGNH::RIGHT, ALIGNV::BOTTOM);
			RB->SetTextureAtlasRegion("es/textures/ui.xml", "Pane_RB");
			RB->SetManualParent(this);
			RB->SetProperty(UIProperty::SPECIAL_ORDER, "2");
			RB->SetVisible(true);
			mFrames.push_back(RB);
		}
		enum FRAME_ORDER
		{
			FRAME_T,
			FRAME_L,
			FRAME_R,
			FRAME_B,

			FRAME_LT,
			FRAME_RT,
			FRAME_MT,

			FRAME_LB,
			FRAME_RB,
		};
		mFrames[FRAME_T]->SetNSizeX(mWNSize.x);
		mFrames[FRAME_T]->SetWNPos(mWNPos);

		mFrames[FRAME_L]->SetNSizeY(mWNSize.y);
		mFrames[FRAME_L]->SetWNPos(mWNPos);

		mFrames[FRAME_R]->SetNSizeY(mWNSize.y);
		Vec2 wnpos = mWNPos;
		wnpos.x += mWNSize.x;
		mFrames[FRAME_R]->SetWNPos(wnpos);

		mFrames[FRAME_B]->SetNSizeX(mWNSize.x);
		wnpos = mWNPos;
		wnpos.y += mWNSize.y;
		mFrames[FRAME_B]->SetWNPos(wnpos);

		mFrames[FRAME_LT]->SetWNPos(mWNPos);

		wnpos = mWNPos;
		wnpos.x += mWNSize.x;
		mFrames[FRAME_RT]->SetWNPos(wnpos);

		wnpos = mWNPos;
		wnpos.x += mWNSize.x*.5f;
		mFrames[FRAME_MT]->SetWNPos(wnpos);

		wnpos = mWNPos;
		wnpos.y += mWNSize.y;
		mFrames[FRAME_LB]->SetWNPos(wnpos);

		wnpos = mWNPos + mWNSize;
		mFrames[FRAME_RB]->SetWNPos(wnpos);
	}
	else
	{
		if (!mFrames.empty())
		{
			for (auto var : mFrames)
			{
				IUIManager::GetUIManager().DeleteComponent(var);
			}
			mFrames.clear();
		}
	}
}

bool Wnd::SetProperty(UIProperty::Enum prop, const char* val)
{
	switch (prop)
	{
	case UIProperty::USE_WND_FRAME:
	{
									  mUseFrame = StringConverter::parseBool(val);
									  RefreshFrame();
									  return true;
	}
	case UIProperty::TITLEBAR:
	{
								 mTitlebar = (Button*)IUIManager::GetUIManager().CreateComponent(ComponentType::Button);
								 mTitlebar->RegisterEventFunc(IEventHandler::EVENT_MOUSE_DRAG,
									 std::bind(&Wnd::OnTitlebarDrag, this, std::placeholders::_1));
								 mTitlebar->SetManualParent(this);
								 mTitlebar->SetNSizeX(mWNSize.x);
								 mTitlebar->SetSizeY(44);
								 mTitlebar->SetNPos(mWNPos);
								 mTitlebar->SetProperty(UIProperty::TEXT_ALIGN, "center");
								 mTitlebar->SetProperty(UIProperty::TEXT_VALIGN, "middle");
								 mTitlebar->SetProperty(UIProperty::NO_BACKGROUND, "true");
								 mTitlebar->SetProperty(UIProperty::TEXT_SIZE, "24");
								 mTitlebar->SetProperty(UIProperty::FIXED_TEXT_SIZE, "true");
								 mTitlebar->SetProperty(UIProperty::SPECIAL_ORDER, "3");
								 mTitlebar->SetText(AnsiToWide(val, strlen(val)));
								 mTitlebar->SetName("_@TitleBar");
								 mWndContentUI = (Wnd*)AddChild(0.0f, 0.0f, 1.0f, 1.0f, ComponentType::Window);
								 Vec2I sizeMod = {
									 mUseFrame ? -26 : 0,
									 mUseFrame  ? -64 : -44,
								 };								 
								 mWndContentUI->SetSizeModificator(sizeMod);
								 mWndContentUI->SetUseAbsYSize(true);
								 mWndContentUI->SetPos(Vec2I(20, 44));
								 mWndContentUI->SetSizeModificator(Vec2I(-20, 0));

								 mWndContentUI->SetProperty(UIProperty::NO_BACKGROUND, "true");
								 if (mUseScrollerV)
								 {
									 mPendingDelete.push_back(mScrollerV);
									 mUseScrollerV = false;
									 mWndContentUI->SetProperty(UIProperty::SCROLLERV, "true");
								 }
								 return true;
	}
	case UIProperty::BACKGROUND_IMAGE_NOATLAS:
	{
												 if (!mBackgroundImage)
												 {
													 mBackgroundImage = FB_NEW(ImageBox);
													 mBackgroundImage->SetParent(this);
													 mBackgroundImage->SetWNPos(mWNPos);
													 mBackgroundImage->SetWNSize(mWNSize);
													 mBackgroundImage->SetProperty(UIProperty::NO_MOUSE_EVENT, "true");
												 }
												 IUIManager::GetUIManager().DirtyRenderList();

												 mBackgroundImage->SetTexture(val);
												 return true;
	}

	case UIProperty::KEEP_IMAGE_RATIO:
	{
										 if (!mBackgroundImage)
										 {
											 mBackgroundImage = FB_NEW(ImageBox);
											 mBackgroundImage->SetParent(this);
											 mBackgroundImage->SetWNPos(mWNPos);
											 mBackgroundImage->SetWNSize(mWNSize);
											 mBackgroundImage->SetProperty(UIProperty::NO_MOUSE_EVENT, "true");
										 }
										 IUIManager::GetUIManager().DirtyRenderList();
										 mBackgroundImage->SetKeepImageRatio(StringConverter::parseBool(val, true));
										 return true;
	}

	}

	return __super::SetProperty(prop, val);
}

void Wnd::OnTitlebarDrag(void *arg)
{
	long x, y;
	gEnv->pEngine->GetMouse()->GetDeltaXY(x, y);
	Vec2 nposOffset = { x / (float)gEnv->pRenderer->GetWidth(), y / (float)gEnv->pRenderer->GetHeight() };
	mAbsTempLock = true;
	SetNPos(GetNPos() + nposOffset);
	mAbsTempLock = false;
}

void Wnd::SetVisible(bool show)
{
	__super::SetVisible(show);
	IUIManager::GetUIManager().SetFocusUI(this);
}

}
