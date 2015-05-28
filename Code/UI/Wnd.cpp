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
, mAlwaysOnTop(false)
, mCloseByEsc(false)
, mSyncWindowPos(false)
{
	mUIObject = gFBEnv->pEngine->CreateUIObject(false, GetRenderTargetSize());
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	//RegisterEventFunc(IEventHandler::EVENT_MOUSE_HOVER,
		//std::bind(&Wnd::MouseConsumer, this, std::placeholders::_1));
}

Wnd::~Wnd()
{
	if (mAlwaysOnTop)
	{
		gFBEnv->pUIManager->UnRegisterAlwaysOnTopWnd(this);
	}
	FB_DELETE(mBackgroundImage);
	for (auto var : mFrames)
	{
		gFBEnv->pUIManager->DeleteComponent(var);
	}
	mFrames.clear();
	gFBEnv->pUIManager->DeleteComponent(mTitlebar);
	mTitlebar = 0;
}

void Wnd::GatherVisit(std::vector<IUIObject*>& v)
{
	if (!mVisibility.IsVisible())
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
	if (!mVisibility.IsVisible())
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

	if (mTitlebar){
		mTitlebar->ChangeSizeX(GetFinalSize().x);
	}
	if (mBackgroundImage){
		mBackgroundImage->ChangeSize(GetFinalSize());
	}
}

void Wnd::OnPosChanged(bool anim)
{
	__super::OnPosChanged(anim);
	RefreshFrame();
	if (mBackgroundImage){
		mBackgroundImage->ChangePos(GetFinalPos());
	}
}

void Wnd::RefreshFrame()
{
	if (mTitlebar)
	{
		mTitlebar->ChangeSizeX(GetFinalSize().x);
		mTitlebar->ChangePos(GetFinalPos());
	}
	if (mUseFrame)
	{
		if (mFrames.empty())
		{
			ImageBox* T = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
			T->SetHwndId(GetHwndId());
			T->SetRender3D(mRender3D, GetRenderTargetSize());
			T->ChangeSizeY(16);
			T->SetTextureAtlasRegion("es/textures/ui.xml", "Pane_T");
			T->SetManualParent(this);
			T->SetProperty(UIProperty::SPECIAL_ORDER, "1");
			T->SetProperty(UIProperty::KEEP_IMAGE_RATIO, "false");
			T->SetVisible(true);
			mFrames.push_back(T);			

			ImageBox* L = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
			L->SetHwndId(GetHwndId());
			L->SetRender3D(mRender3D, GetRenderTargetSize());
			L->ChangeSizeX(16);
			L->SetTextureAtlasRegion("es/textures/ui.xml", "Pane_L");
			L->SetManualParent(this);
			L->SetProperty(UIProperty::SPECIAL_ORDER, "1");
			L->SetProperty(UIProperty::KEEP_IMAGE_RATIO, "false");
			L->SetVisible(true);
			mFrames.push_back(L);			

			ImageBox* R = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
			R->SetHwndId(GetHwndId());
			R->SetRender3D(mRender3D, GetRenderTargetSize());
			R->ChangeSizeX(16);
			R->SetAlign(ALIGNH::RIGHT, ALIGNV::TOP);
			R->SetTextureAtlasRegion("es/textures/ui.xml", "Pane_R");
			R->SetManualParent(this);
			R->SetProperty(UIProperty::SPECIAL_ORDER, "1");
			R->SetProperty(UIProperty::KEEP_IMAGE_RATIO, "false");
			R->SetVisible(true);
			mFrames.push_back(R);

			ImageBox* B = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
			B->SetHwndId(GetHwndId());
			B->SetRender3D(mRender3D, GetRenderTargetSize());
			B->SetAlign(ALIGNH::LEFT, ALIGNV::BOTTOM);
			B->ChangeSizeY(20);
			B->SetTextureAtlasRegion("es/textures/ui.xml", "Pane_B");
			B->SetManualParent(this);
			B->SetProperty(UIProperty::SPECIAL_ORDER, "1");
			B->SetProperty(UIProperty::KEEP_IMAGE_RATIO, "false");
			B->SetVisible(true);
			mFrames.push_back(B);

			ImageBox* LT = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
			LT->SetHwndId(GetHwndId());
			LT->SetRender3D(mRender3D, GetRenderTargetSize());
			LT->ChangeSize(Vec2I(40, 44));
			LT->SetTextureAtlasRegion("es/textures/ui.xml", "Pane_LT");
			LT->SetManualParent(this);
			LT->SetProperty(UIProperty::SPECIAL_ORDER, "2");
			LT->SetVisible(true);
			mFrames.push_back(LT);

			ImageBox* RT = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
			RT->SetHwndId(GetHwndId());
			RT->SetRender3D(mRender3D, GetRenderTargetSize());
			RT->ChangeSize(Vec2I(40, 44));
			RT->SetAlign(ALIGNH::RIGHT, ALIGNV::TOP);
			RT->SetTextureAtlasRegion("es/textures/ui.xml", "Pane_RT");
			RT->SetManualParent(this);
			RT->SetProperty(UIProperty::SPECIAL_ORDER, "2");
			RT->SetVisible(true);
			mFrames.push_back(RT);

			ImageBox* MT = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
			MT->SetHwndId(GetHwndId());
			MT->SetRender3D(mRender3D, GetRenderTargetSize());
			MT->ChangeSize(Vec2I(302, 44));
			MT->SetAlign(ALIGNH::CENTER, ALIGNV::TOP);
			MT->SetTextureAtlasRegion("es/textures/ui.xml", "Pane_MT");
			MT->SetManualParent(this);
			MT->SetProperty(UIProperty::SPECIAL_ORDER, "2");
			MT->SetVisible(true);
			mFrames.push_back(MT);

			ImageBox* LB = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
			LB->SetHwndId(GetHwndId());
			LB->SetRender3D(mRender3D, GetRenderTargetSize());
			LB->ChangeSize(Vec2I(40, 44));
			LB->SetAlign(ALIGNH::LEFT, ALIGNV::BOTTOM);
			LB->SetTextureAtlasRegion("es/textures/ui.xml", "Pane_LB");
			LB->SetManualParent(this);
			LB->SetProperty(UIProperty::SPECIAL_ORDER, "2");
			LB->SetVisible(true);
			mFrames.push_back(LB);

			ImageBox* RB = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
			RB->SetHwndId(GetHwndId());
			RB->SetRender3D(mRender3D, GetRenderTargetSize());
			RB->ChangeSize(Vec2I(40, 44));
			RB->SetAlign(ALIGNH::RIGHT, ALIGNV::BOTTOM);
			RB->SetTextureAtlasRegion("es/textures/ui.xml", "Pane_RB");
			RB->SetManualParent(this);
			RB->SetProperty(UIProperty::SPECIAL_ORDER, "2");
			RB->SetVisible(true);
			mFrames.push_back(RB);
		}

		if (!mWndContentUI){
			mWndContentUI = (Wnd*)AddChild(0.f, 0.f, 1.0f, 1.0f, ComponentType::Window);
			mWndContentUI->SetName("_@ContentWindow");
			mWndContentUI->SetGhost(true);
			mWndContentUI->SetRuntimeChild(true);
			mWndContentUI->SetRender3D(mRender3D, GetRenderTargetSize());
			Vec2I sizeMod = {
				mUseFrame ? -26 : 0,
				mUseFrame ? -64 : -44,
			};
			mWndContentUI->ModifySize(sizeMod);
			mWndContentUI->ChangePos(Vec2I(20, 44));
			mWndContentUI->ModifySize(Vec2I(-20, 0));

			mWndContentUI->SetProperty(UIProperty::NO_BACKGROUND, "true");
			mWndContentUI->SetProperty(UIProperty::USE_NSIZEX, "true");
			mWndContentUI->SetProperty(UIProperty::USE_NSIZEY, "true");
			if (mUseScrollerV)
			{
				mPendingDelete.push_back(mScrollerV);
				mUseScrollerV = false;
				mWndContentUI->SetProperty(UIProperty::SCROLLERV, "true");
			}
			TransferChildrenTo(mWndContentUI);
			mWndContentUI->SetVisible(mVisibility.IsVisible());
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
		const auto& finalSize = GetFinalSize();
		const auto& finalPos = GetFinalPos();
		mFrames[FRAME_T]->ChangeSizeX(finalSize.x);
		mFrames[FRAME_T]->ChangePos(finalPos);

		mFrames[FRAME_L]->ChangeSizeY(finalSize.y);
		mFrames[FRAME_L]->ChangePos(finalPos);

		mFrames[FRAME_R]->ChangeSizeY(finalSize.y);
		auto wpos = finalPos;
		wpos.x += finalSize.x;
		mFrames[FRAME_R]->ChangePos(wpos);

		mFrames[FRAME_B]->ChangeSizeX(finalSize.x);
		wpos = finalPos;
		wpos.y += finalSize.y;
		mFrames[FRAME_B]->ChangePos(wpos);

		mFrames[FRAME_LT]->ChangePos(finalPos);

		wpos = finalPos;
		wpos.x += finalSize.x;
		mFrames[FRAME_RT]->ChangePos(wpos);

		wpos = finalPos;
		wpos.x += Round(finalSize.x*.5f);
		mFrames[FRAME_MT]->ChangePos(wpos);

		wpos = finalPos;
		wpos.y += finalSize.y;
		mFrames[FRAME_LB]->ChangePos(wpos);

		wpos = finalPos + finalSize;
		mFrames[FRAME_RB]->ChangePos(wpos);
	}
	else
	{
		if (!mFrames.empty())
		{
			for (auto var : mFrames)
			{
				gFBEnv->pUIManager->DeleteComponent(var);
			}
			mFrames.clear();
			gFBUIManager->DirtyRenderList(GetHwndId());
		}

		if (mWndContentUI)
			mWndContentUI->TransferChildrenTo(this);
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
		mTitlebarString = val;
								 if (!mTitlebar)
								 {
									 mTitlebar = (Button*)gFBEnv->pUIManager->CreateComponent(ComponentType::Button);
									 mTitlebar->SetHwndId(GetHwndId());
									 mTitlebar->SetRender3D(mRender3D, GetRenderTargetSize());
									 mTitlebar->SetVisible(mVisibility.IsVisible());
									 mTitlebar->RegisterEventFunc(UIEvents::EVENT_MOUSE_DRAG,
										 std::bind(&Wnd::OnTitlebarDrag, this, std::placeholders::_1));
									 mTitlebar->SetManualParent(this);
									 mTitlebar->ChangeSizeX(GetFinalSize().x);
									 mTitlebar->ChangeSizeY(44);
									 mTitlebar->ChangePos(GetFinalPos());
									 mTitlebar->SetProperty(UIProperty::TEXT_ALIGN, "center");
									 mTitlebar->SetProperty(UIProperty::TEXT_VALIGN, "middle");
									 mTitlebar->SetProperty(UIProperty::NO_BACKGROUND, "true");
									 mTitlebar->SetProperty(UIProperty::TEXT_SIZE, "24");									 
									 mTitlebar->SetProperty(UIProperty::SPECIAL_ORDER, "3");
									 mTitlebar->SetName("_@TitleBar");
								 }
								 auto text = TranslateText(val);
								 if (text.empty())
									mTitlebar->SetText(AnsiToWide(val));
								 else
									 mTitlebar->SetText(AnsiToWide(text.c_str()));
								 
								 return true;
	}
	case UIProperty::BACKGROUND_IMAGE_NOATLAS:
	{
		mStrBackground = val;
												 if (!mBackgroundImage)
												 {
													 mBackgroundImage = FB_NEW(ImageBox);
													 mBackgroundImage->SetHwndId(GetHwndId());
													 mBackgroundImage->SetRender3D(mRender3D, GetRenderTargetSize());
													 mBackgroundImage->SetManualParent(this);
													 mBackgroundImage->ChangePos(GetFinalPos());
													 mBackgroundImage->ChangeSize(GetFinalSize());
													 mBackgroundImage->SetProperty(UIProperty::NO_MOUSE_EVENT, "true");
												 }
												 gFBEnv->pUIManager->DirtyRenderList(GetHwndId());

												 mBackgroundImage->SetTexture(val);
												 return true;
	}

	case UIProperty::KEEP_IMAGE_RATIO:
	{
		mStrKeepRatio = val;
										 if (!mBackgroundImage)
										 {
											 mBackgroundImage = FB_NEW(ImageBox);
											 mBackgroundImage->SetHwndId(GetHwndId());
											 mBackgroundImage->SetRender3D(mRender3D, GetRenderTargetSize());
											 mBackgroundImage->SetManualParent(this);
											 mBackgroundImage->ChangePos(GetFinalPos());
											 mBackgroundImage->ChangeSize(GetFinalSize());
											 mBackgroundImage->SetProperty(UIProperty::NO_MOUSE_EVENT, "true");
										 }
										 gFBEnv->pUIManager->DirtyRenderList(GetHwndId());
										 mBackgroundImage->SetKeepImageRatio(StringConverter::parseBool(val, true));
										 return true;
	}

	case UIProperty::ALWAYS_ON_TOP:
	{
									  mAlwaysOnTop = StringConverter::parseBool(val);
									  if (mAlwaysOnTop)
										  gFBEnv->pUIManager->RegisterAlwaysOnTopWnd(this);
									  else
										  gFBEnv->pUIManager->UnRegisterAlwaysOnTopWnd(this);
									  return true;
	}

	case UIProperty::CLOSE_BY_ESC:
	{
									 mCloseByEsc = StringConverter::parseBool(val);
									 return true;
	}

	case UIProperty::SYNC_WINDOW_POS:
	{
		mSyncWindowPos = StringConverter::parseBool(val);
		return true;
	}

	case UIProperty::MSG_TRANSLATION:
	{
		mMsgTranslationUnit = val;
		return true;
	}

	}

	return __super::SetProperty(prop, val);
}

bool Wnd::GetProperty(UIProperty::Enum prop, char val[], bool notDefaultOnly)
{
	switch (prop)
	{
	case UIProperty::USE_WND_FRAME:
	{
		if (notDefaultOnly)
		{
			if (mUseFrame == UIProperty::GetDefaultValueBool(prop))
				return false;
		}
		strcpy(val, StringConverter::toString(mUseFrame).c_str());
		return true;
	}
	case UIProperty::TITLEBAR:
	{
		if (notDefaultOnly)
		{
			if (mTitlebarString.empty())
				return false;
		}
		strcpy(val, mTitlebarString.c_str());
		return true;
	}
	case UIProperty::BACKGROUND_IMAGE_NOATLAS:
	{
		if (notDefaultOnly)
		{
			if (mStrBackground.empty())
				return false;
		}
		strcpy(val, mStrBackground.c_str());
		return true;
	}

	case UIProperty::KEEP_IMAGE_RATIO:
	{
		if (notDefaultOnly)
		{
			if (mStrKeepRatio.empty())
				return false;
		}
		strcpy(val, mStrKeepRatio.c_str());
		return true;
	}

	case UIProperty::ALWAYS_ON_TOP:
	{
		if (notDefaultOnly)
		{
			if (mAlwaysOnTop == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		strcpy(val, StringConverter::toString(mAlwaysOnTop).c_str()); 
		return true;
	}

	case UIProperty::CLOSE_BY_ESC:
	{
		if (notDefaultOnly)
		{
			if (mCloseByEsc == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		strcpy(val, StringConverter::toString(mCloseByEsc).c_str());
		return true;
	}

	case UIProperty::SYNC_WINDOW_POS:
	{
		if (notDefaultOnly)
		{
			if (mSyncWindowPos == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		strcpy(val, StringConverter::toString(mSyncWindowPos).c_str());
		return true;
	}

	case UIProperty::MSG_TRANSLATION:
	{
		if (notDefaultOnly)
		{
			if (mMsgTranslationUnit.empty())
				return false;
		}
		strcpy(val, mMsgTranslationUnit.c_str());
		return true;
	}

	}

	return __super::GetProperty(prop, val, notDefaultOnly);
}

void Wnd::OnTitlebarDrag(void *arg)
{
	if (mSyncWindowPos)
	{
		auto mouse = gFBEnv->pEngine->GetMouse();		
		// move OS window
		long sx, sy, x, y;
		mouse->GetDragStart(sx, sy);
		mouse->GetPos(x, y);
		auto hwnd = gFBEnv->pEngine->GetWindowHandle(mHwndId);
		RECT rect;
		GetWindowRect(hwnd, &rect);
		MoveWindow(hwnd, OSWindowPos.x + x - sx, OSWindowPos.y + y - sy, rect.right - rect.left, rect.bottom - rect.top, FALSE);
	}
	else
	{
		long x, y;
		gFBEnv->pEngine->GetMouse()->GetDeltaXY(x, y);
		Move(Vec2I(x, y));
		/*auto rtSize = GetRenderTargetSize();
		Vec2 nposOffset = { x / (float)rtSize.x, y / (float)rtSize.y };
		SetNPos(GetNPos() + nposOffset);*/
	}
}

bool Wnd::SetVisible(bool show)
{
	bool changed = __super::SetVisible(show);
	if (changed)
	{
		if (!mParent && !mManualParent)
			gFBEnv->pUIManager->SetFocusUI(this);

		if (mTitlebar)
			mTitlebar->SetVisible(show);
	}
	return changed;
}

void Wnd::RefreshScissorRects()
{
	__super::RefreshScissorRects();
	if (mTitlebar)
	{
		mTitlebar->RefreshScissorRects();
	}
	if (!mFrames.empty())
	{
		for (auto var : mFrames)
		{
			var->RefreshScissorRects();
		}
	}
}

void Wnd::SetAnimScale(const Vec2& scale)
{
	if (mTitlebar)
	{
		mTitlebar->SetAnimScale(scale);
	}
	if (!mFrames.empty())
	{
		for (auto var : mFrames)
		{
			var->SetAnimScale(scale);
		}
	}

	__super::SetAnimScale(scale);
}


void Wnd::StartHighlight(float speed)
{
	for (auto var : mFrames)
	{
		var->StartHighlight(speed);
	}
	__super::StartHighlight(speed);
}
void Wnd::StopHighlight()
{
	for (auto var : mFrames)
	{
		var->StopHighlight();
	}

	__super::StopHighlight();
}

void Wnd::SetHwndId(HWND_ID hwndId)
{
	__super::SetHwndId(hwndId);
	if (mTitlebar)
	{
		mTitlebar->SetHwndId(hwndId);
	}
	for (auto win : mFrames)
	{
		if (win)
		{
			win->SetHwndId(hwndId);
		}
	}
	if (mBackgroundImage)
		mBackgroundImage->SetHwndId(hwndId);
}

const char* Wnd::GetMsgTranslationUnit() const
{
	auto root = GetRootWnd();
	if (root == this)
	{
		if (mMsgTranslationUnit.empty())
			return "msg";
		else
			return mMsgTranslationUnit.c_str();
	}
	else
	{
		return root->GetMsgTranslationUnit();
	}
}

}

