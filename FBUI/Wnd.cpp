/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#include "StdAfx.h"
#include "Wnd.h"
#include "UIManager.h"
#include "Button.h"
#include "ImageBox.h"
#include "Scroller.h"
#include "UIObject.h"
#include "FBRenderer/TextureAtlas.h"

namespace fb
{
WndPtr Wnd::Create(){
	WndPtr p(new Wnd, [](Wnd* obj){ delete obj; });
	p->mSelfPtr = p;
	return p;
}
Wnd::Wnd()
: mUseFrame(false)
, mAlwaysOnTop(false)
, mCloseByEsc(false)
, mSyncWindowPos(false)
, mNoFocus(false)
, mMoveToBottom(false)
{
	mUIObject = UIObject::Create(GetRenderTargetSize());
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	//RegisterEventFunc(IEventHandler::EVENT_MOUSE_HOVER,
		//std::bind(&Wnd::MouseConsumer, this, std::placeholders::_1));
}

Wnd::~Wnd()
{
	if (UIManager::HasInstance()){
		if (mAlwaysOnTop)
		{
			UIManager::GetInstance().UnRegisterAlwaysOnTopWnd(mSelfPtr.lock());
		}
	}
	mFrames.clear();
}

void Wnd::GatherVisit(std::vector<UIObject*>& v)
{
	if (!mVisibility.IsVisible())
		return;

	v.push_back(mUIObject.get());

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
	auto titlebar = mTitlebar.lock();
	if (titlebar)
	{
		titlebar->ChangeSizeX(std::max(40, GetFinalSize().x - 80));
	}

	if (mUseFrame)
	{
		const char* uixmlPath = "EssentialEngineData/textures/ui.xml";
		if (mFrames.empty())
		{			
			auto T = std::static_pointer_cast<ImageBox>(UIManager::GetInstance().CreateComponent(ComponentType::ImageBox));
			T->SetHwndId(GetHwndId());
			T->SetRender3D(mRender3D, GetRenderTargetSize());
			T->SetManualParent(mSelfPtr.lock());
			T->SetProperty(UIProperty::SPECIAL_ORDER, "1");			
			T->SetVisible(true);
			const auto& sizeT = T->SetTextureAtlasRegion(uixmlPath, UIManager::GetInstance().GetWndBorderRegion("t"));
			T->ChangeSize(sizeT);
			mFrames.push_back(T);			

			auto L = std::static_pointer_cast<ImageBox>(UIManager::GetInstance().CreateComponent(ComponentType::ImageBox));
			L->SetHwndId(GetHwndId());
			L->SetRender3D(mRender3D, GetRenderTargetSize());
			L->SetManualParent(mSelfPtr.lock());
			L->SetProperty(UIProperty::SPECIAL_ORDER, "1");			
			L->SetVisible(true);
			const auto& sizeL = L->SetTextureAtlasRegion(uixmlPath,
				UIManager::GetInstance().GetWndBorderRegion("l"));
			L->ChangeSize(sizeL);
			mFrames.push_back(L);			

			auto R = std::static_pointer_cast<ImageBox>(UIManager::GetInstance().CreateComponent(ComponentType::ImageBox));
			R->SetHwndId(GetHwndId());
			R->SetRender3D(mRender3D, GetRenderTargetSize());
			R->SetAlign(ALIGNH::RIGHT, ALIGNV::TOP);
			R->SetManualParent(mSelfPtr.lock());
			R->SetProperty(UIProperty::SPECIAL_ORDER, "1");			
			R->SetVisible(true);
			const auto& sizeR = R->SetTextureAtlasRegion(uixmlPath,
				UIManager::GetInstance().GetWndBorderRegion("r"));
			R->ChangeSize(sizeR);
			mFrames.push_back(R);

			auto B = std::static_pointer_cast<ImageBox>(UIManager::GetInstance().CreateComponent(ComponentType::ImageBox));
			B->SetHwndId(GetHwndId());
			B->SetRender3D(mRender3D, GetRenderTargetSize());
			B->SetAlign(ALIGNH::LEFT, ALIGNV::BOTTOM);
			B->SetManualParent(mSelfPtr.lock());
			B->SetProperty(UIProperty::SPECIAL_ORDER, "1");			
			B->SetVisible(true);
			const auto& sizeB = B->SetTextureAtlasRegion(uixmlPath,
				UIManager::GetInstance().GetWndBorderRegion("b"));
			B->ChangeSize(sizeB);
			mFrames.push_back(B);

			auto LT = std::static_pointer_cast<ImageBox>(UIManager::GetInstance().CreateComponent(ComponentType::ImageBox));
			LT->SetHwndId(GetHwndId());
			LT->SetRender3D(mRender3D, GetRenderTargetSize());
			LT->SetManualParent(mSelfPtr.lock());
			const auto& sizeLT = LT->SetTextureAtlasRegion(uixmlPath,
				UIManager::GetInstance().GetWndBorderRegion("lt"));
			LT->ChangeSize(sizeLT);			
			LT->SetProperty(UIProperty::SPECIAL_ORDER, "2");
			LT->SetVisible(true);
			mFrames.push_back(LT);

			auto RT = std::static_pointer_cast<ImageBox>(UIManager::GetInstance().CreateComponent(ComponentType::ImageBox));
			RT->SetHwndId(GetHwndId());
			RT->SetRender3D(mRender3D, GetRenderTargetSize());
			RT->SetAlign(ALIGNH::RIGHT, ALIGNV::TOP);
			RT->SetManualParent(mSelfPtr.lock());
			const auto& sizeRT = RT->SetTextureAtlasRegion(uixmlPath,
				UIManager::GetInstance().GetWndBorderRegion("rt"));
			RT->ChangeSize(sizeRT);
			RT->SetProperty(UIProperty::SPECIAL_ORDER, "2");
			RT->SetVisible(true);
			mFrames.push_back(RT);

			const char* mtRegion = UIManager::GetInstance().GetWndBorderRegion("mt");
			if (mtRegion&&strlen(mtRegion)>0){
				auto MT = std::static_pointer_cast<ImageBox>(UIManager::GetInstance().CreateComponent(ComponentType::ImageBox));
				MT->SetHwndId(GetHwndId());
				MT->SetRender3D(mRender3D, GetRenderTargetSize());
				MT->SetAlign(ALIGNH::CENTER, ALIGNV::TOP);
				MT->SetManualParent(mSelfPtr.lock());
				const auto& sizeMT = MT->SetTextureAtlasRegion(uixmlPath, mtRegion);
				MT->ChangeSize(sizeMT);
				MT->SetProperty(UIProperty::SPECIAL_ORDER, "2");
				MT->SetVisible(true);
				mFrames.push_back(MT);
			}
			else{
				mFrames.push_back(0);
			}
			

			auto LB = std::static_pointer_cast<ImageBox>(UIManager::GetInstance().CreateComponent(ComponentType::ImageBox));
			LB->SetHwndId(GetHwndId());
			LB->SetRender3D(mRender3D, GetRenderTargetSize());
			LB->SetAlign(ALIGNH::LEFT, ALIGNV::BOTTOM);
			LB->SetManualParent(mSelfPtr.lock());
			const auto& sizeLB = LB->SetTextureAtlasRegion(uixmlPath,
				UIManager::GetInstance().GetWndBorderRegion("lb"));
			LB->ChangeSize(sizeLB);
			LB->SetProperty(UIProperty::SPECIAL_ORDER, "2");
			LB->SetVisible(true);
			mFrames.push_back(LB);

			auto RB = std::static_pointer_cast<ImageBox>(UIManager::GetInstance().CreateComponent(ComponentType::ImageBox));
			RB->SetHwndId(GetHwndId());
			RB->SetRender3D(mRender3D, GetRenderTargetSize());			
			RB->SetAlign(ALIGNH::RIGHT, ALIGNV::BOTTOM);
			RB->SetManualParent(mSelfPtr.lock());
			const auto& sizeRB = RB->SetTextureAtlasRegion(uixmlPath,
				UIManager::GetInstance().GetWndBorderRegion("rb"));
			RB->ChangeSize(sizeRB);
			RB->SetProperty(UIProperty::SPECIAL_ORDER, "2");
			RB->SetVisible(true);
			mFrames.push_back(RB);
		}

		auto contentUI = mWndContentUI.lock();
		if (!contentUI && !mTitlebarString.empty()){
			auto wndContentUI = std::static_pointer_cast<Wnd>(AddChild(0.f, 0.f, 1.0f, 1.0f, ComponentType::Window));
			mWndContentUI = wndContentUI;
			DoNotTransfer(wndContentUI);
			wndContentUI->SetName("_@ContentWindow");
			wndContentUI->SetGhost(true);
			wndContentUI->SetRuntimeChild(true);
			wndContentUI->SetRender3D(mRender3D, GetRenderTargetSize());
			auto atlas = Renderer::GetInstance().GetTextureAtlas(uixmlPath);
			if (atlas){
				Vec2I leftTopSize(0, 0);
				auto ltRegion = atlas->GetRegion(UIManager::GetInstance().GetWndBorderRegion("lt"));
				if (ltRegion){
					leftTopSize = ltRegion->GetSize();
				}
				Vec2I leftBottomSize(0, 0);
				auto lbRegion = atlas->GetRegion(UIManager::GetInstance().GetWndBorderRegion("lb"));
				if (lbRegion){
					leftBottomSize = lbRegion->GetSize();
				}
				const int titleBar = 44;
				Vec2I sizeMod = {
					mUseFrame ? -leftTopSize.x*2 : 0, // x
					mUseFrame ? -(titleBar + leftBottomSize.y) : -titleBar, // y
				};
				wndContentUI->ModifySize(sizeMod);
				wndContentUI->ChangePos(Vec2I(leftTopSize.x, titleBar));
			}

			wndContentUI->SetProperty(UIProperty::NO_BACKGROUND, "true");
			wndContentUI->SetProperty(UIProperty::USE_NSIZEX, "true");
			wndContentUI->SetProperty(UIProperty::USE_NSIZEY, "true");
			if (mUseScrollerV)
			{
				if (!mScrollerV.expired())
					mPendingDelete.push_back(mScrollerV.lock());
				mUseScrollerV = false;
				wndContentUI->SetProperty(UIProperty::SCROLLERV, "true");
			}			
			TransferChildrenTo(wndContentUI);
			wndContentUI->SetVisible(mVisibility.IsVisible());
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
			mFrames.clear();
			UIManager::GetInstance().DirtyRenderList(GetHwndId());
			auto contentUI = mWndContentUI.lock();
			if (contentUI){
				contentUI->TransferChildrenTo(std::static_pointer_cast<Container>(mSelfPtr.lock()));
				RemoveChild(contentUI);
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
									  mUseFrame = StringConverter::ParseBool(val);
									  RefreshFrame();
									  return true;
	}
	case UIProperty::TITLEBAR:
	{
		mTitlebarString = val;
		if (!mTitlebarString.empty())
		{
			if (mTitlebar.expired())
			{
				// backup and remove content UI
				auto contentUI = mWndContentUI.lock();
				mWndContentUI.reset();			

				auto titlebar = std::static_pointer_cast<Button>(AddChild(0.5f, 0.f, 1.f, 0.1f, ComponentType::Button));
				mTitlebar = titlebar;
				DoNotTransfer(titlebar);
				titlebar->SetVisible(mVisibility.IsVisible());				
				titlebar->SetSizeX(std::max(40, GetFinalSize().x - 80));
				titlebar->SetRuntimeChild(true);
				titlebar->ChangeSizeY(44);
				titlebar->SetUseAbsPos(false);
				titlebar->SetProperty(UIProperty::ALIGNH, "center");
				titlebar->SetProperty(UIProperty::TEXT_ALIGN, "center");
				titlebar->SetProperty(UIProperty::TEXT_VALIGN, "middle");
				titlebar->SetProperty(UIProperty::NO_BACKGROUND, "true");
				titlebar->SetProperty(UIProperty::TEXT_SIZE, "20");
				titlebar->SetProperty(UIProperty::SPECIAL_ORDER, "3");
				titlebar->SetName("_@TitleBar");
				titlebar->RegisterEventFunc(UIEvents::EVENT_MOUSE_DRAG,
					std::bind(&Wnd::OnTitlebarDrag, this, std::placeholders::_1));
				// recover content ui
				mWndContentUI = contentUI;
				RefreshFrame();
			}
			auto titilebar = mTitlebar.lock();
			auto text = TranslateText(val);
			if (text.empty())
				titilebar->SetText(AnsiToWide(val));
			else
				titilebar->SetText(AnsiToWide(text.c_str()));
		}
		else{
			auto titilebar = mTitlebar.lock();
			if (titilebar){
				auto contentUI = mWndContentUI.lock();
				mWndContentUI.reset();				
				RemoveChild(titilebar);
				mWndContentUI = contentUI;
			}
		}
								 
		return true;
	}

	case UIProperty::CLOSE_BTN:
	{
		bool closeBtn = StringConverter::ParseBool(val);
		if (closeBtn){
			auto cbtn = mCloseBtn.lock();
			if (!cbtn){
				auto contentUI = mWndContentUI.lock();
				mWndContentUI.reset();

				cbtn = std::static_pointer_cast<Button>(AddChild(Vec2I(0, 0), Vec2I(26, 24), ComponentType::Button));
				mCloseBtn = cbtn;
				DoNotTransfer(cbtn);
				cbtn->SetVisible(mVisibility.IsVisible());				
				cbtn->SetInitialOffset(Vec2I(-10, +10));
				cbtn->ChangeNPos(Vec2(1, 0));
				cbtn->SetUseAbsPos(false);
				cbtn->SetRuntimeChild(true);
				cbtn->SetProperty(UIProperty::ALIGNH, "right");				
				cbtn->SetProperty(UIProperty::REGION, "x");				
				cbtn->SetProperty(UIProperty::BACK_COLOR, "0.2, 0.2, 0.6, 0.1");
				cbtn->SetProperty(UIProperty::BACK_COLOR_OVER, "0.2, 0.2, 0.6, 0.4");
				cbtn->SetProperty(UIProperty::SPECIAL_ORDER, "4");
				cbtn->SetName("_@CloseBtn");
				cbtn->SetProperty(UIProperty::USE_BORDER, "true");
				cbtn->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
					std::bind(&Wnd::OnCloseBtnClicked, this, std::placeholders::_1));

				mWndContentUI = contentUI;
			}
		}
		else{
			auto cbtn = mCloseBtn.lock();
			if (cbtn){
				auto contentUI = mWndContentUI.lock();				
				RemoveChild(cbtn);
				mCloseBtn.reset();
				mWndContentUI = contentUI;
			}
		}
		return true;
	}
	case UIProperty::BACKGROUND_IMAGE_NOATLAS:
	{
		mStrBackground = val;
		if (mBackgroundImage.expired())
		{
			mBackgroundImage = CreateBackgroundImage();
		}

		mBackgroundImage.lock()->SetTexture(val);
		return true;
	}

	case UIProperty::IMAGE_DISPLAY:
	{
		mStrImageDisplay = val;
		if (mBackgroundImage.expired())
		{
			mBackgroundImage = CreateBackgroundImage();
		}
		mBackgroundImage.lock()->SetProperty(prop, val);
		return true;
	}

	case UIProperty::ALWAYS_ON_TOP:
	{
									  mAlwaysOnTop = StringConverter::ParseBool(val);
									  if (mAlwaysOnTop)
										  UIManager::GetInstance().RegisterAlwaysOnTopWnd(mSelfPtr.lock());
									  else
										  UIManager::GetInstance().UnRegisterAlwaysOnTopWnd(mSelfPtr.lock());
									  return true;
	}

	case UIProperty::CLOSE_BY_ESC:
	{
									 mCloseByEsc = StringConverter::ParseBool(val);
									 return true;
	}

	case UIProperty::SYNC_WINDOW_POS:
	{
		mSyncWindowPos = StringConverter::ParseBool(val);
		return true;
	}

	case UIProperty::MSG_TRANSLATION:
	{
		mMsgTranslationUnit = val;
		return true;
	}

	case UIProperty::WND_NO_FOCUS:
	{
		mNoFocus = StringConverter::ParseBool(val);
		return true;
	}

	case UIProperty::WND_MOVE_TO_BOTTOM:
	{
		mMoveToBottom = StringConverter::ParseBool(val);
		return true;
	}

	case UIProperty::OPEN_SOUND:
	{
		mOpenSound = val;
		return true;
	}

	case UIProperty::CLOSE_SOUND:
	{
		mCloseSound = val;
		return true;
	}

	}

	return __super::SetProperty(prop, val);
}

ImageBoxPtr Wnd::CreateBackgroundImage(){
	if (mBackgroundImage.expired())
	{
		auto image = std::static_pointer_cast<ImageBox>(AddChild(0.f, 0.f, 1.f, 1.f, ComponentType::ImageBox));
		mBackgroundImage = image;
		image->SetRuntimeChild(true);
		image->SetProperty(UIProperty::NO_MOUSE_EVENT_ALONE, "true");
		image->SetVisible(GetVisible());
		image->SetUseAbsSize(false);
		image->SetGhost(true);
	}
	return mBackgroundImage.lock();
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
		strcpy_s(val, bufsize, StringConverter::ToString(mUseFrame).c_str());
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
		bool closeBtn = !mCloseBtn.expired();
		if (notDefaultOnly){			
			if (closeBtn == UIProperty::GetDefaultValueBool(prop)){
				return false;
			}
		}

		strcpy_s(val, bufsize, StringConverter::ToString(closeBtn).c_str());
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

	case UIProperty::IMAGE_DISPLAY:
	{
		if (notDefaultOnly)
		{
			if (mStrImageDisplay.empty() || ImageDisplay::ConvertToEnum(mStrImageDisplay.c_str()) == UIProperty::GetDefaultValueInt(prop))
				return false;
		}
		strcpy_s(val, bufsize, mStrImageDisplay.c_str());
		return true;
	}

	case UIProperty::ALWAYS_ON_TOP:
	{
		if (notDefaultOnly)
		{
			if (mAlwaysOnTop == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		strcpy_s(val, bufsize, StringConverter::ToString(mAlwaysOnTop).c_str());
		return true;
	}

	case UIProperty::CLOSE_BY_ESC:
	{
		if (notDefaultOnly)
		{
			if (mCloseByEsc == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		strcpy_s(val, bufsize, StringConverter::ToString(mCloseByEsc).c_str());
		return true;
	}

	case UIProperty::SYNC_WINDOW_POS:
	{
		if (notDefaultOnly)
		{
			if (mSyncWindowPos == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		strcpy_s(val, bufsize, StringConverter::ToString(mSyncWindowPos).c_str());
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
		strcpy_s(val, bufsize, StringConverter::ToString(mNoFocus).c_str());
		return true;
	}

	case UIProperty::WND_MOVE_TO_BOTTOM:
	{
		if (notDefaultOnly){
			if (mMoveToBottom == UIProperty::GetDefaultValueBool(prop))
				return false;
		}
		strcpy_s(val, bufsize, StringConverter::ToString(mMoveToBottom).c_str());
		return true;
	}

	case UIProperty::OPEN_SOUND:
	{
		if (notDefaultOnly){
			if (mOpenSound == UIProperty::GetDefaultValueString(prop))
				return false;
		}
		strcpy_s(val, bufsize, mOpenSound.c_str());
		return true;
	}

	case UIProperty::CLOSE_SOUND:
	{
		if (notDefaultOnly){
			if (mCloseSound == UIProperty::GetDefaultValueString(prop))
				return false;
		}
		strcpy_s(val, bufsize, mCloseSound.c_str());
		return true;
	}

	}

	return __super::GetProperty(prop, val, bufsize, notDefaultOnly);
}

void Wnd::OnTitlebarDrag(void *arg)
{
	auto injector = InputManager::GetInstance().GetInputInjector();
	if (mSyncWindowPos)
	{
		// move OS window
		long sx, sy, x, y;
		injector->GetDragStart(sx, sy);
		injector->GetMousePos(x, y);
		auto hwnd = Renderer::GetInstance().GetWindowHandle(mHwndId);
		RECT rect;
#if defined(_PLATFORM_WINDOWS_)
		GetWindowRect((HWND)hwnd, &rect);
		MoveWindow((HWND)hwnd, rect.left + x - sx, rect.top + y - sy, rect.right - rect.left, rect.bottom - rect.top, TRUE);
#else
		assert(0 && "Not Implemented");
#endif
	}
	else
	{
		long x, y;		
		injector->GetDeltaXY(x, y);
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
	if (show){
		auto parent = GetParent();
		auto manualParent = GetManualParent();
		if (!parent && !manualParent){
			if (mNoFocus && !mMoveToBottom){
				UIManager::GetInstance().MoveToTop(mSelfPtr.lock());
			}
			else if (!mNoFocus){
				UIManager::GetInstance().SetFocusUI(mSelfPtr.lock());
			}
			if (mMoveToBottom){
				UIManager::GetInstance().MoveToBottom(mSelfPtr.lock());
			}

		}
	}
	bool changed = __super::SetVisible(show);
	if (changed)
	{
		auto titlebar = mTitlebar.lock();
		if (titlebar)
			titlebar->SetVisible(show);
	}
	return changed;
}

void Wnd::RefreshScissorRects()
{
	__super::RefreshScissorRects();
	auto titleBar = mTitlebar.lock();
	if (titleBar)
	{
		titleBar->RefreshScissorRects();
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

void Wnd::OnResolutionChanged(HWindowId hwndId){
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
	auto titlebar = mTitlebar.lock();
	if (titlebar)
	{
		titlebar->SetAnimScale(scale);
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

void Wnd::SetHwndId(HWindowId hwndId)
{
	__super::SetHwndId(hwndId);
	auto titlebar = mTitlebar.lock();
	if (titlebar)
	{
		titlebar->SetHwndId(hwndId);
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
	auto root = GetRootWnd().get();
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

