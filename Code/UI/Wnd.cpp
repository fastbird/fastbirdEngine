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
, mCloseBtn(0)
, mNoFocus(false)
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
	for (auto var : mFrames)
	{
		if (var)
			gFBEnv->pUIManager->DeleteComponent(var);
	}
	mFrames.clear();
}

void Wnd::GatherVisit(std::vector<IUIObject*>& v)
{
	if (!mVisibility.IsVisible())
		return;

	v.push_back(mUIObject);

	__super::GatherVisit(v);

	for (auto var : mFrames)
	{
		if (var)
			var->GatherVisit(v);
	}	
}

void Wnd::OnSizeChanged()
{
	__super::OnSizeChanged();
	RefreshFrame();
}

void Wnd::OnPosChanged(bool anim)
{
	__super::OnPosChanged(anim);
	RefreshFrame();
}

void Wnd::RefreshFrame()
{
	if (mTitlebar)
	{
		mTitlebar->ChangeSizeX(std::max(40, GetFinalSize().x - 80));
	}
	if (mUseFrame)
	{
		const char* uixmlPath = "es/textures/ui.xml";
		if (mFrames.empty())
		{			
			ImageBox* T = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
			T->SetHwndId(GetHwndId());
			T->SetRender3D(mRender3D, GetRenderTargetSize());
			T->SetManualParent(this);
			T->SetProperty(UIProperty::SPECIAL_ORDER, "1");
			T->SetProperty(UIProperty::KEEP_IMAGE_RATIO, "false");
			T->SetVisible(true);
			const auto& sizeT = T->SetTextureAtlasRegion(uixmlPath,
				gFBUIManager->GetWndBorderRegion("t"));
			T->ChangeSize(sizeT);
			mFrames.push_back(T);			

			ImageBox* L = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
			L->SetHwndId(GetHwndId());
			L->SetRender3D(mRender3D, GetRenderTargetSize());
			L->SetManualParent(this);
			L->SetProperty(UIProperty::SPECIAL_ORDER, "1");
			L->SetProperty(UIProperty::KEEP_IMAGE_RATIO, "false");
			L->SetVisible(true);
			const auto& sizeL = L->SetTextureAtlasRegion(uixmlPath,
				gFBUIManager->GetWndBorderRegion("l"));
			L->ChangeSize(sizeL);
			mFrames.push_back(L);			

			ImageBox* R = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
			R->SetHwndId(GetHwndId());
			R->SetRender3D(mRender3D, GetRenderTargetSize());
			R->SetAlign(ALIGNH::RIGHT, ALIGNV::TOP);
			R->SetManualParent(this);
			R->SetProperty(UIProperty::SPECIAL_ORDER, "1");
			R->SetProperty(UIProperty::KEEP_IMAGE_RATIO, "false");
			R->SetVisible(true);
			const auto& sizeR = R->SetTextureAtlasRegion(uixmlPath,
				gFBUIManager->GetWndBorderRegion("r"));
			R->ChangeSize(sizeR);
			mFrames.push_back(R);

			ImageBox* B = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
			B->SetHwndId(GetHwndId());
			B->SetRender3D(mRender3D, GetRenderTargetSize());
			B->SetAlign(ALIGNH::LEFT, ALIGNV::BOTTOM);
			B->SetManualParent(this);
			B->SetProperty(UIProperty::SPECIAL_ORDER, "1");
			B->SetProperty(UIProperty::KEEP_IMAGE_RATIO, "false");
			B->SetVisible(true);
			const auto& sizeB = B->SetTextureAtlasRegion(uixmlPath,
				gFBUIManager->GetWndBorderRegion("b"));
			B->ChangeSize(sizeB);
			mFrames.push_back(B);

			ImageBox* LT = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
			LT->SetHwndId(GetHwndId());
			LT->SetRender3D(mRender3D, GetRenderTargetSize());
			LT->SetManualParent(this);
			const auto& sizeLT = LT->SetTextureAtlasRegion(uixmlPath,
				gFBUIManager->GetWndBorderRegion("lt"));
			LT->ChangeSize(sizeLT);			
			LT->SetProperty(UIProperty::SPECIAL_ORDER, "2");
			LT->SetVisible(true);
			mFrames.push_back(LT);

			ImageBox* RT = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
			RT->SetHwndId(GetHwndId());
			RT->SetRender3D(mRender3D, GetRenderTargetSize());
			RT->SetAlign(ALIGNH::RIGHT, ALIGNV::TOP);
			RT->SetManualParent(this);
			const auto& sizeRT = RT->SetTextureAtlasRegion(uixmlPath,
				gFBUIManager->GetWndBorderRegion("rt"));
			RT->ChangeSize(sizeRT);
			RT->SetProperty(UIProperty::SPECIAL_ORDER, "2");
			RT->SetVisible(true);
			mFrames.push_back(RT);

			const char* mtRegion = gFBUIManager->GetWndBorderRegion("mt");
			if (mtRegion&&strlen(mtRegion)>0){
				ImageBox* MT = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
				MT->SetHwndId(GetHwndId());
				MT->SetRender3D(mRender3D, GetRenderTargetSize());
				MT->SetAlign(ALIGNH::CENTER, ALIGNV::TOP);
				MT->SetManualParent(this);
				const auto& sizeMT = MT->SetTextureAtlasRegion(uixmlPath, mtRegion);
				MT->ChangeSize(sizeMT);
				MT->SetProperty(UIProperty::SPECIAL_ORDER, "2");
				MT->SetVisible(true);
				mFrames.push_back(MT);
			}
			else{
				mFrames.push_back(0);
			}
			

			ImageBox* LB = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
			LB->SetHwndId(GetHwndId());
			LB->SetRender3D(mRender3D, GetRenderTargetSize());
			LB->SetAlign(ALIGNH::LEFT, ALIGNV::BOTTOM);
			LB->SetManualParent(this);
			const auto& sizeLB = LB->SetTextureAtlasRegion(uixmlPath,
				gFBUIManager->GetWndBorderRegion("lb"));
			LB->ChangeSize(sizeLB);
			LB->SetProperty(UIProperty::SPECIAL_ORDER, "2");
			LB->SetVisible(true);
			mFrames.push_back(LB);

			ImageBox* RB = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
			RB->SetHwndId(GetHwndId());
			RB->SetRender3D(mRender3D, GetRenderTargetSize());			
			RB->SetAlign(ALIGNH::RIGHT, ALIGNV::BOTTOM);
			RB->SetManualParent(this);
			const auto& sizeRB = RB->SetTextureAtlasRegion(uixmlPath,
				gFBUIManager->GetWndBorderRegion("rb"));
			RB->ChangeSize(sizeRB);
			RB->SetProperty(UIProperty::SPECIAL_ORDER, "2");
			RB->SetVisible(true);
			mFrames.push_back(RB);
		}

		if (!mWndContentUI && !mTitlebarString.empty()){
			mWndContentUI = (Wnd*)AddChild(0.f, 0.f, 1.0f, 1.0f, ComponentType::Window);
			DoNotTransfer(mWndContentUI);
			mWndContentUI->SetName("_@ContentWindow");
			mWndContentUI->SetGhost(true);
			mWndContentUI->SetRuntimeChild(true);
			mWndContentUI->SetRender3D(mRender3D, GetRenderTargetSize());
			auto atlas = gFBEnv->pRenderer->GetTextureAtlas(uixmlPath);
			if (atlas){
				Vec2I leftTopSize(0, 0);
				auto ltRegion = atlas->GetRegion(gFBUIManager->GetWndBorderRegion("lt"));
				if (ltRegion){
					leftTopSize = ltRegion->GetSize();
				}
				Vec2I leftBottomSize(0, 0);
				auto lbRegion = atlas->GetRegion(gFBUIManager->GetWndBorderRegion("lb"));
				if (lbRegion){
					leftBottomSize = lbRegion->GetSize();
				}
				const int titleBar = 44;
				Vec2I sizeMod = {
					mUseFrame ? -leftTopSize.x*2 : 0, // x
					mUseFrame ? -(titleBar + leftBottomSize.y) : -titleBar, // y
				};
				mWndContentUI->ModifySize(sizeMod);
				mWndContentUI->ChangePos(Vec2I(leftTopSize.x, titleBar));
			}

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
		auto ltSize = mFrames[FRAME_LT]->GetSize();
		mFrames[FRAME_T]->ChangeSizeX(finalSize.x - ltSize.x*2);
		mFrames[FRAME_T]->ChangePos(Vec2I(finalPos.x + ltSize.x, finalPos.y));

		mFrames[FRAME_L]->ChangeSizeY(finalSize.y - ltSize.y*2);
		mFrames[FRAME_L]->ChangePos(Vec2I(finalPos.x, finalPos.y + ltSize.y));

		mFrames[FRAME_R]->ChangeSizeY(finalSize.y - ltSize.y*2);
		auto wpos = finalPos;
		wpos.x += finalSize.x;
		mFrames[FRAME_R]->ChangePos(Vec2I(wpos.x, wpos.y + ltSize.y));

		mFrames[FRAME_B]->ChangeSizeX(finalSize.x - ltSize.x*2);
		wpos = finalPos;
		wpos.y += finalSize.y;
		mFrames[FRAME_B]->ChangePos(Vec2I(wpos.x + ltSize.x, wpos.y));

		mFrames[FRAME_LT]->ChangePos(finalPos);

		wpos = finalPos;
		wpos.x += finalSize.x;
		mFrames[FRAME_RT]->ChangePos(wpos);

		if (mFrames[FRAME_MT]){
			wpos = finalPos;
			wpos.x += Round(finalSize.x*.5f);
			mFrames[FRAME_MT]->ChangePos(wpos);
		}

		wpos = finalPos;
		wpos.y += finalSize.y;
		mFrames[FRAME_LB]->ChangePos(wpos);

		wpos = finalPos + finalSize;
		mFrames[FRAME_RB]->ChangePos(wpos);
	}
	else
	{
		// !mUseFrame
		if (!mFrames.empty())
		{
			for (auto var : mFrames)
			{
				if (var)
					gFBEnv->pUIManager->DeleteComponent(var);
			}
			mFrames.clear();
			gFBUIManager->DirtyRenderList(GetHwndId());
			if (mWndContentUI){
				mWndContentUI->TransferChildrenTo(this);
				RemoveChild(mWndContentUI);
			}
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
		mTitlebarString = val;
		if (!mTitlebarString.empty())
		{
			if (!mTitlebar)
			{
				BackupContentWnd backup(&mWndContentUI);
				mTitlebar = (Button*)AddChild(0.5f, 0.f, 1.f, 0.1f, ComponentType::Button);				
				DoNotTransfer(mTitlebar);
				mTitlebar->SetVisible(mVisibility.IsVisible());				
				mTitlebar->SetSizeX(std::max(40, GetFinalSize().x - 80));
				mTitlebar->SetRuntimeChild(true);
				mTitlebar->ChangeSizeY(44);
				mTitlebar->SetUseAbsPos(false);
				mTitlebar->SetProperty(UIProperty::ALIGNH, "center");
				mTitlebar->SetProperty(UIProperty::TEXT_ALIGN, "center");
				mTitlebar->SetProperty(UIProperty::TEXT_VALIGN, "middle");
				mTitlebar->SetProperty(UIProperty::NO_BACKGROUND, "true");
				mTitlebar->SetProperty(UIProperty::TEXT_SIZE, "24");
				mTitlebar->SetProperty(UIProperty::SPECIAL_ORDER, "3");
				mTitlebar->SetName("_@TitleBar");
				mTitlebar->RegisterEventFunc(UIEvents::EVENT_MOUSE_DRAG,
					std::bind(&Wnd::OnTitlebarDrag, this, std::placeholders::_1));
				RefreshFrame();

			}
			auto text = TranslateText(val);
			if (text.empty())
				mTitlebar->SetText(AnsiToWide(val));
			else
				mTitlebar->SetText(AnsiToWide(text.c_str()));
		}
		else{
			if (mTitlebar){
				BackupContentWnd backup(&mWndContentUI);
				RemoveChild(mTitlebar);
			}
		}
								 
		return true;
	}

	case UIProperty::CLOSE_BTN:
	{
		bool closeBtn = StringConverter::parseBool(val);
		if (closeBtn){
			if (!mCloseBtn){
				BackupContentWnd backup(&mWndContentUI);

				mCloseBtn = (Button*)AddChild(Vec2I(0, 0), Vec2I(26, 24), ComponentType::Button);
				DoNotTransfer(mCloseBtn);
				mCloseBtn->SetVisible(mVisibility.IsVisible());				
				mCloseBtn->SetInitialOffset(Vec2I(-10, +10));
				mCloseBtn->ChangeNPos(Vec2(1, 0));
				mCloseBtn->SetUseAbsPos(false);
				mCloseBtn->SetRuntimeChild(true);
				mCloseBtn->SetProperty(UIProperty::ALIGNH, "right");				
				mCloseBtn->SetProperty(UIProperty::REGION, "x");				
				mCloseBtn->SetProperty(UIProperty::BACK_COLOR, "0.2, 0.2, 0.6, 0.1");
				mCloseBtn->SetProperty(UIProperty::BACK_COLOR_OVER, "0.2, 0.2, 0.6, 0.4");
				mCloseBtn->SetProperty(UIProperty::SPECIAL_ORDER, "4");
				mCloseBtn->SetName("_@CloseBtn");
				mCloseBtn->SetProperty(UIProperty::USE_BORDER, "true");
				mCloseBtn->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
					std::bind(&Wnd::OnCloseBtnClicked, this, std::placeholders::_1));
			}
		}
		else{
			if (mCloseBtn){
				BackupContentWnd backup(&mWndContentUI);
				RemoveChild(mCloseBtn);
				mCloseBtn = 0;
			}
		}
		return true;
	}
	case UIProperty::BACKGROUND_IMAGE_NOATLAS:
	{
		mStrBackground = val;
		if (!mBackgroundImage)
		{
			mBackgroundImage = CreateBackgroundImage();
		}

		mBackgroundImage->SetTexture(val);
		return true;
	}

	case UIProperty::KEEP_IMAGE_RATIO:
	{
		mStrKeepRatio = val;
		if (!mBackgroundImage)
		{
			mBackgroundImage = CreateBackgroundImage();
		}
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

	case UIProperty::WND_NO_FOCUS:
	{
		mNoFocus = StringConverter::parseBool(val);
		return true;
	}

	}

	return __super::SetProperty(prop, val);
}

ImageBox* Wnd::CreateBackgroundImage(){
	if (!mBackgroundImage)
	{
		mBackgroundImage = (ImageBox*)AddChild(0.f, 0.f, 1.f, 1.f, ComponentType::ImageBox);
		mBackgroundImage->SetRuntimeChild(true);
		mBackgroundImage->SetProperty(UIProperty::NO_MOUSE_EVENT_ALONE, "true");
		mBackgroundImage->SetVisible(GetVisible());
		mBackgroundImage->SetUseAbsSize(false);
		mBackgroundImage->SetGhost(true);
	}
	return mBackgroundImage;
}

bool Wnd::GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly)
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
		strcpy_s(val, bufsize, StringConverter::toString(mUseFrame).c_str());
		return true;
	}
	case UIProperty::TITLEBAR:
	{
		if (notDefaultOnly)
		{
			if (mTitlebarString.empty())
				return false;
		}
		strcpy_s(val, bufsize, mTitlebarString.c_str());
		return true;
	}
	case UIProperty::CLOSE_BTN:
	{
		bool closeBtn = mCloseBtn != 0;
		if (notDefaultOnly){			
			if (closeBtn == UIProperty::GetDefaultValueBool(prop)){
				return false;
			}
		}

		strcpy_s(val, bufsize, StringConverter::toString(closeBtn).c_str());
		return true;
	}
	case UIProperty::BACKGROUND_IMAGE_NOATLAS:
	{
		if (notDefaultOnly)
		{
			if (mStrBackground.empty())
				return false;
		}
		strcpy_s(val, bufsize, mStrBackground.c_str());
		return true;
	}

	case UIProperty::KEEP_IMAGE_RATIO:
	{
		if (notDefaultOnly)
		{
			if (mStrKeepRatio.empty())
				return false;
		}
		strcpy_s(val, bufsize, mStrKeepRatio.c_str());
		return true;
	}

	case UIProperty::ALWAYS_ON_TOP:
	{
		if (notDefaultOnly)
		{
			if (mAlwaysOnTop == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		strcpy_s(val, bufsize, StringConverter::toString(mAlwaysOnTop).c_str());
		return true;
	}

	case UIProperty::CLOSE_BY_ESC:
	{
		if (notDefaultOnly)
		{
			if (mCloseByEsc == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		strcpy_s(val, bufsize, StringConverter::toString(mCloseByEsc).c_str());
		return true;
	}

	case UIProperty::SYNC_WINDOW_POS:
	{
		if (notDefaultOnly)
		{
			if (mSyncWindowPos == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		strcpy_s(val, bufsize, StringConverter::toString(mSyncWindowPos).c_str());
		return true;
	}

	case UIProperty::MSG_TRANSLATION:
	{
		if (notDefaultOnly)
		{
			if (mMsgTranslationUnit.empty())
				return false;
		}
		strcpy_s(val, bufsize, mMsgTranslationUnit.c_str());
		return true;
	}

	case UIProperty::WND_NO_FOCUS:
	{
		if (notDefaultOnly){
			if (mNoFocus == UIProperty::GetDefaultValueBool(prop))
				return false;
		}
		strcpy_s(val, bufsize, StringConverter::toString(mNoFocus).c_str());
		return true;
	}

	}

	return __super::GetProperty(prop, val, bufsize, notDefaultOnly);
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
		MoveWindow(hwnd, rect.left + x - sx, rect.top + y - sy, rect.right - rect.left, rect.bottom - rect.top, TRUE);
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

void Wnd::OnCloseBtnClicked(void* arg){
	SetVisible(false);
}

bool Wnd::SetVisible(bool show)
{
	bool changed = __super::SetVisible(show);
	if (changed)
	{
		if (!mParent && !mManualParent){
			if (mNoFocus){
				gFBEnv->pUIManager->MoveToTop(this);
			}
			else{
				gFBEnv->pUIManager->SetFocusUI(this);
			}

		}

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
			if (var)
				var->RefreshScissorRects();
		}
	}
}

void Wnd::OnResolutionChanged(HWND_ID hwndId){
	__super::OnResolutionChanged(hwndId);
	//if (mBackgroundImage){
	//	mBackgroundImage->OnResolutionChanged(hwndId);
	//}
	for (auto it : mFrames){
		if (it){
			it->OnResolutionChanged(hwndId);
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
			if (var)
				var->SetAnimScale(scale);
		}
	}

	__super::SetAnimScale(scale);
}


void Wnd::StartHighlight(float speed)
{
	for (auto var : mFrames)
	{
		if (var)
			var->StartHighlight(speed);
	}
	__super::StartHighlight(speed);
}
void Wnd::StopHighlight()
{
	for (auto var : mFrames)
	{
		if (var)
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

