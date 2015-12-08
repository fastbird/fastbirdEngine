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
#include "Button.h"
#include "ImageBox.h"
#include "UIManager.h"
#include "HorizontalGauge.h"
#include "UIObject.h"
#include "FBRenderer/TextureAtlas.h"
#include "FBRenderer/Texture.h"

namespace fb
{

	const float Button::LEFT_GAP = 0.001f;
	ButtonPtr Button::Create(){
		ButtonPtr p(new Button, [](Button* obj){ delete obj; });
		p->mSelfPtr = p;
		return p;
	}

	Button::Button()
		: mInProgress(false)
		, mNoBackgroundBackup(false)
		, mActivated(false)
		, mChangeImageActivation(false)
		, mIconText(false)
		, mNoButton(false)
		, mButtonIconSize(0)
		, mFps(0), mActivatedRot(false), mImageColorOverlay(1, 1, 1, 1)
		, mImageSize(0, 0)
	{
		mUIObject = UIObject::Create(GetRenderTargetSize());
		mUIObject->SetMaterial("EssentialEngineData/materials/UIButton.material");
		mUIObject->mOwnerUI = this;
		mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
		mTextColor = Color(1.0f, 0.92f, 0.0f);
		mTextColorHover = Color(1.0f, 1.0f, 0.5f);
		mTextColorDown = Color(1.0f, 0.2f, 0.2f);
		mUIObject->SetTextColor(mTextColor);

		// default colors
		mBackColor = Color(0.0f, 0.0f, 0.0f, 0.7f);
		mBackColorOver = Color(0.1f, 0.1f, 0.43f, 0.8f);
		mBackColorDown = Color(0.3f, 0.3f, 0.f, 0.5f);
		mEdgeColor = Color(1.f, 1.f, 1.f, 0.7f);
		mEdgeColorOver = Color(0.9f, 0.85f, 0.0f, 0.7f);

		mUIObject->GetMaterial()->SetDiffuseColor(mBackColor.GetVec4());
		// material param 0 : ratio, sizes
		// material param 1 : edge color
		mUIObject->GetMaterial()->SetMaterialParameter(1, mEdgeColor.GetVec4());

		RegisterEventFunc(UIEvents::EVENT_MOUSE_DOWN,
			std::bind(&Button::OnMouseDown, this, std::placeholders::_1));
		RegisterEventFunc(UIEvents::EVENT_MOUSE_HOVER,
			std::bind(&Button::OnMouseHover, this, std::placeholders::_1));
		RegisterEventFunc(UIEvents::EVENT_MOUSE_IN,
			std::bind(&Button::OnMouseIn, this, std::placeholders::_1));
		RegisterEventFunc(UIEvents::EVENT_MOUSE_OUT,
			std::bind(&Button::OnMouseOut, this, std::placeholders::_1));
	}

	Button::~Button()
	{
	}

	void Button::OnPosChanged(bool anim)
	{
		__super::OnPosChanged(anim);
		if (mButtonIconSize > 0)
			AlignIconText();
	}

	void Button::GatherVisit(std::vector<UIObject*>& v)
	{
		if (!mVisibility.IsVisible())
			return;
		assert(mUIObject);

		//	Profiler p("Button::GatherVisit");

		auto hoverBack = mImages[ButtonImages::BackImageHover].lock();
		auto back = mImages[ButtonImages::BackImage].lock();
		if (mMouseIn && hoverBack)
			hoverBack->GatherVisit(v);
		else if (back)
			back->GatherVisit(v);

		auto progressBar = mProgressBar.lock();
		if (progressBar && mInProgress)
			progressBar->GatherVisit(v);

		v.push_back(mUIObject.get());

		auto active = mImages[ButtonImages::ActiveImage].lock();
		auto hover = mImages[ButtonImages::ImageHover].lock();
		auto image = mImages[ButtonImages::Image].lock();
		auto deactive = mImages[ButtonImages::DeactiveImage].lock();
		if (active && active->GetVisible())
		{
			if (mChangeImageActivation)
			{
				active->GatherVisit(v);
			}
			else
			{
				if (mMouseIn && hover)
					hover->GatherVisit(v);
				else if (image)
					image->GatherVisit(v);
				active->GatherVisit(v);
			}
		}
		else if (deactive && deactive->GetVisible())
		{
			if (mChangeImageActivation)
			{
				deactive->GatherVisit(v);
			}
			else
			{
				if (mMouseIn && hover)
					hover->GatherVisit(v);
				else if (image)
					image->GatherVisit(v);
				deactive->GatherVisit(v);
			}
		}
		else
		{
			if (mMouseIn && hover)
				hover->GatherVisit(v);
			else if (image)
				image->GatherVisit(v);
		}

		auto frame = mImages[ButtonImages::FrameImage].lock();
		if (frame)
			frame->GatherVisit(v);

		__super::GatherVisit(v);
	}

	void Button::OnMouseDown(void* arg)
	{
		if (mNoButton)
			return;
		mUIObject->GetMaterial()->SetDiffuseColor(mBackColorDown.GetVec4());
		mUIObject->GetMaterial()->SetEmissiveColor(1.0f, 1.0f, 0.2f, 1);
		mUIObject->SetTextColor(mEnable ? mTextColorDown : mTextColorDown*.5f);
		auto backImageHover = mImages[ButtonImages::BackImageHover].lock();
		auto backImage = mImages[ButtonImages::BackImage].lock();
		auto imageHover = mImages[ButtonImages::ImageHover].lock();
		auto image = mImages[ButtonImages::Image].lock();
		if (backImageHover)
			backImageHover->SetSpecularColor(Vec4(0.5f, 0.1f, 0.1f, 1.0f));
		else if (backImage)
			backImage->SetSpecularColor(Vec4(0.5f, 0.1f, 0.1f, 1.0f));
		else if (imageHover)
			imageHover->SetSpecularColor(Vec4(0.5f, 0.1f, 0.1f, 1.0f));
		else if (image)
			image->SetSpecularColor(Vec4(0.5f, 0.1f, 0.1f, 1.0f));
		TriggerRedraw();
	}

	void Button::OnMouseClicked(IInputInjectorPtr injector){
		__super::OnMouseClicked(injector);
		if (mNoButton)
			return;
		UIManager::GetInstance().PlaySound(UISounds::ButtonClick);
	}

	void Button::OnMouseHover(void* arg)
	{
		if (mNoButton)
			return;
		mUIObject->GetMaterial()->SetDiffuseColor(mBackColorOver.GetVec4());
		mUIObject->SetTextColor(mEnable ? mTextColorHover : mTextColorHover * .5f);
		SetCursor(WinBase::sCursorOver);

		mUIObject->GetMaterial()->SetEmissiveColor(0.8f, 0.8f, 0.1f, 1);
		//  1 is edge color
		mUIObject->GetMaterial()->SetMaterialParameter(1, mEdgeColorOver.GetVec4());
		//if (mImages[ButtonImages::ImageHover] || mImages[ButtonImages::BackImageHover])
		//UIManager::GetInstance().DirtyRenderList(GetHwndId());

		auto backImageHover = mImages[ButtonImages::BackImageHover].lock();
		auto backImage = mImages[ButtonImages::BackImage].lock();
		auto imageHover = mImages[ButtonImages::ImageHover].lock();
		auto image = mImages[ButtonImages::Image].lock();
		if (backImageHover)
			backImageHover->SetSpecularColor(Vec4(1.0f, 1.0f, 1.0f, 1.0f));
		else if (backImage)
			backImage->SetSpecularColor(Vec4(1.0f, 1.0f, 1.0f, 1.0f));
		else if (imageHover)
			imageHover->SetSpecularColor(Vec4(1.0f, 1.0f, 1.0f, 1.0f));
		else if (image)
			image->SetSpecularColor(Vec4(1.0f, 1.0f, 1.0f, 1.0f));


		// sometimes OnMouseHover function is called manually even the mouse
		// is in out. - DropDown
		mMouseIn = true;

		TriggerRedraw();

	}

	void Button::OnMouseIn(void* arg){
		if (!mImages[ButtonImages::ImageHover].expired() || !mImages[ButtonImages::BackImageHover].expired())
			UIManager::GetInstance().DirtyRenderList(GetHwndId());
		if (mNoButton)
			return;
		UIManager::GetInstance().PlaySound(UISounds::ButtonIn);
	}

	void Button::OnMouseOut(void* arg)
	{
		if (mNoButton)
			return;
		mUIObject->GetMaterial()->SetDiffuseColor(mBackColor.GetVec4());
		mUIObject->SetTextColor(mEnable ? mTextColor : mTextColor*.5f);
		mUIObject->GetMaterial()->SetEmissiveColor(0, 0, 0, 0);
		//  1 is edge color
		mUIObject->GetMaterial()->SetMaterialParameter(1, mEdgeColor.GetVec4());
		if (!mImages[ButtonImages::ImageHover].expired() || !mImages[ButtonImages::BackImageHover].expired())
			UIManager::GetInstance().DirtyRenderList(GetHwndId());

		auto backImageHover = mImages[ButtonImages::BackImageHover].lock();
		auto backImage = mImages[ButtonImages::BackImage].lock();
		auto imageHover = mImages[ButtonImages::ImageHover].lock();
		auto image = mImages[ButtonImages::Image].lock();
		if (backImageHover)
			backImageHover->SetSpecularColor(Vec4(1.0f, 1.0f, 1.0f, 1.0f));
		else if (backImage)
			backImage->SetSpecularColor(Vec4(1.0f, 1.0f, 1.0f, 1.0f));
		else if (imageHover)
			imageHover->SetSpecularColor(Vec4(1.0f, 1.0f, 1.0f, 1.0f));
		else if (image)
			image->SetSpecularColor(Vec4(1.0f, 1.0f, 1.0f, 1.0f));

		// sometimes OnMouseHover function is called manually even the mouse
		// is in out. - DropDown

		mMouseIn = false;		
		TriggerRedraw();
	}

	bool Button::SetProperty(UIProperty::Enum prop, const char* val)
	{
		switch (prop)
		{
		case UIProperty::BACK_COLOR:
		{
			mBackColor = Color(val);
			mUIObject->GetMaterial()->SetDiffuseColor(mBackColor.GetVec4());
			return true;
		}
		case UIProperty::BACK_COLOR_OVER:
		{

			mBackColorOver = Color(val);
			return true;
		}
		case UIProperty::BACK_COLOR_DOWN:
		{
			mBackColorDown = Color(val);
			return true;
		}

		case UIProperty::TEXTUREATLAS:
		{
			mImageAtlas = val;
			return true;
		}
		case UIProperty::REGION:
		{
			mRegionName = val;
			if (mImages[ButtonImages::Image].expired())
			{
				mImages[ButtonImages::Image] = CreateImageBox();
				UpdateImageSize();
			}

			auto image = mImages[ButtonImages::Image].lock();
			assert(image);
			SetDefaultImageAtlasPathIfNotSet();
			image->SetTextureAtlasRegion(mImageAtlas.c_str(), val);
			image->DrawAsFixedSizeAtCenter();
			//mImages[ButtonImages::Image]->DrawAsFixedSizeCenteredAt(GetFinalPos() + Round(GetFinalSize() * .5f));
			if (mIconText)
			{
				OnSizeChanged();
				AlignIconText();
			}
			return true;
		}
		case UIProperty::REGIONS:
		{
			mRegionNames = val;
			if (mImages[ButtonImages::Image].expired())
			{
				mImages[ButtonImages::Image] = CreateImageBox();
			}
			else
			{
				RemoveChild(mImages[ButtonImages::Image].lock());
				mImages[ButtonImages::Image] = CreateImageBox();
			}

			SetDefaultImageAtlasPathIfNotSet();
			auto image = mImages[ButtonImages::Image].lock();
			assert(image);
			image->SetProperty(UIProperty::TEXTUREATLAS, mImageAtlas.c_str());
			image->SetProperty(prop, val);
			image->DrawAsFixedSizeCenteredAt(GetFinalPos() + Round(GetFinalSize() * .5f));
			if (mIconText)
			{
				OnSizeChanged();
				AlignIconText();
			}
			return true;
		}
		case UIProperty::TEXTURE_FILE:
		{
			mTextureFile = val;
			if (mImages[ButtonImages::Image].expired())
			{
				mImages[ButtonImages::Image] = CreateImageBox();
				UpdateImageSize();
			}
			auto image = mImages[ButtonImages::Image].lock();
			image->SetTexture(val);
			//mImages[ButtonImages::Image]->DrawAsFixedSizeAtCenter();
			if (mIconText)
			{
				OnSizeChanged();
				AlignIconText();
			}
			return true;
		}
		case UIProperty::FPS:
		{
			mFps = StringConverter::ParseReal(val);
			auto image = mImages[ButtonImages::Image].lock();
			if_assert_pass(image)
			{
				image->SetProperty(prop, val);
			}
			return true;
		}
		case UIProperty::HOVER_IMAGE:
		{
			mHorverImage = val;
			if (mImages[ButtonImages::ImageHover].expired())
			{
				mImages[ButtonImages::ImageHover] = CreateImageBox();
				UpdateImageSize();
			}

			SetDefaultImageAtlasPathIfNotSet();
			if (strlen(val) == 0)
			{
				RemoveChild(mImages[ButtonImages::ImageHover].lock());				
			}
			else
			{
				auto imageHover = mImages[ButtonImages::ImageHover].lock();
				imageHover->SetTextureAtlasRegion(mImageAtlas.c_str(), val);
				imageHover->DrawAsFixedSizeAtCenter();
			}

			return true;
		}
		case UIProperty::BACKGROUND_IMAGE:
		{
			mBackgroundImage = val;
			if (mImages[ButtonImages::BackImage].expired())
			{
				mImages[ButtonImages::BackImage] = CreateImageBox();
			}
			SetDefaultImageAtlasPathIfNotSet();
			auto backImage = mImages[ButtonImages::BackImage].lock();
			backImage->SetTextureAtlasRegion(mImageAtlas.c_str(), val);
			//mImages[ButtonImages::BackImage]->DrawAsFixedSizeCenteredAt(GetFinalPos() + Round(GetFinalSize() * .5f));
			return true;
		}
		case UIProperty::BACKGROUND_IMAGE_DISABLED:
		{
			mBackgroundImageDisabled = val;
			if (mImages[ButtonImages::BackImageDisabled].expired())
			{
				mImages[ButtonImages::BackImageDisabled] = CreateImageBox();
			}
			SetDefaultImageAtlasPathIfNotSet();
			auto backImageDisabled = mImages[ButtonImages::BackImageDisabled].lock();
			backImageDisabled->SetTextureAtlasRegion(mImageAtlas.c_str(), val);
			backImageDisabled->DrawAsFixedSizeCenteredAt(GetFinalPos() + Round(GetFinalSize() * .5f));
			return true;
		}

		case UIProperty::BACKGROUND_IMAGE_HOVER:
		{
			mBackgoundImageHover = val;
			if (mImages[ButtonImages::BackImageHover].expired())
			{
				mImages[ButtonImages::BackImageHover] = CreateImageBox();
			}

			SetDefaultImageAtlasPathIfNotSet();
			auto backImageHover = mImages[ButtonImages::BackImageHover].lock();
			backImageHover->SetTextureAtlasRegion(mImageAtlas.c_str(), val);
			backImageHover->DrawAsFixedSizeCenteredAt(GetFinalPos() + Round(GetFinalSize() * .5f));
			return true;
		}

		case UIProperty::BACKGROUND_IMAGE_NOATLAS:
		{
			mBackgroundImageNoAtlas = val;
			if (mImages[ButtonImages::BackImage].expired())
			{
				mImages[ButtonImages::BackImage] = CreateImageBox();
			}
			auto backImage = mImages[ButtonImages::BackImage].lock();
			backImage->SetTexture(val);
			backImage->DrawAsFixedSizeCenteredAt(GetFinalPos() + Round(GetFinalSize() * .5f));
			return true;
		}

		case UIProperty::BACKGROUND_IMAGE_HOVER_NOATLAS:
		{
			mBackgroundImageHoverNoAtlas = val;
			if (mImages[ButtonImages::BackImageHover].expired())
			{
				mImages[ButtonImages::BackImageHover] = CreateImageBox();
			}

			auto backImageHover = mImages[ButtonImages::BackImageHover].lock();
			backImageHover->SetTexture(val);
			backImageHover->DrawAsFixedSizeCenteredAt(GetFinalPos() + Round(GetFinalSize() * .5f));
			return true;
		}

		case UIProperty::FRAME_IMAGE:
		{
			mFrameImage = val;
			if (mImages[ButtonImages::FrameImage].expired())
			{
				mImages[ButtonImages::FrameImage] = CreateImageBox();
			}
			auto frameImage = mImages[ButtonImages::FrameImage].lock();
			SetDefaultImageAtlasPathIfNotSet();
			frameImage->SetTextureAtlasRegion(mImageAtlas.c_str(), val);
			if (strlen(val) == 0)
			{
				frameImage->SetVisible(false);
			}
			else
			{
				frameImage->SetVisible(true);
			}

			UIManager::GetInstance().DirtyRenderList(GetHwndId());
			return true;
		}

		case UIProperty::FRAME_IMAGE_DISABLED:
		{
			mFrameImageDisabled = val;
			if (mImages[ButtonImages::FrameImageDisabled].expired())
			{
				mImages[ButtonImages::FrameImageDisabled] = CreateImageBox();
			}

			SetDefaultImageAtlasPathIfNotSet();
			auto frameImageDisabled = mImages[ButtonImages::FrameImageDisabled].lock();
			frameImageDisabled->SetTextureAtlasRegion(mImageAtlas.c_str(), val);
			if (strlen(val) == 0)
			{
				frameImageDisabled->SetVisible(false);
			}
			else
			{
				frameImageDisabled->SetVisible(true);
			}

			UIManager::GetInstance().DirtyRenderList(GetHwndId());
			return true;
		}

		case UIProperty::ACTIVATED_IMAGE:
		{
			mActivatedImage = val;
			if (mImages[ButtonImages::ActiveImage].expired())
			{
				auto imageBox = CreateImageBox();
				mImages[ButtonImages::ActiveImage] = imageBox;
				imageBox->SetProperty(UIProperty::INHERIT_VISIBLE_TRUE, "false");
				UpdateImageSize();
			}

			auto activeImage = mImages[ButtonImages::ActiveImage].lock();
			if (strlen(val) == 0)
			{
				activeImage->SetVisible(false);
			}
			else
			{
				SetDefaultImageAtlasPathIfNotSet();
				activeImage->SetTextureAtlasRegion(mImageAtlas.c_str(), val);
				activeImage->SetVisible(false);
				activeImage->SetCenterUVMatParam();
			}
			activeImage->DrawAsFixedSizeCenteredAt(GetFinalPos() + Round(GetFinalSize() * .5f));

			return true;
		}

		case UIProperty::DEACTIVATED_IMAGE:
		{
			mDeactivatedImage = val;
			if (mImages[ButtonImages::DeactiveImage].expired())
			{
				auto p = CreateImageBox();
				mImages[ButtonImages::DeactiveImage] = p;
				p->SetProperty(UIProperty::INHERIT_VISIBLE_TRUE, "false");
				UpdateImageSize();
			}

			auto deactiveImage = mImages[ButtonImages::DeactiveImage].lock();
			if (strlen(val) == 0)
			{
				deactiveImage->SetVisible(false);
			}
			else
			{
				SetDefaultImageAtlasPathIfNotSet();
				deactiveImage->SetTextureAtlasRegion(mImageAtlas.c_str(), val);
				deactiveImage->SetVisible(false);
				deactiveImage->SetCenterUVMatParam();
			}
			deactiveImage->DrawAsFixedSizeCenteredAt(GetFinalPos() + Round(GetFinalSize() * .5f));
			return true;
		}

		case UIProperty::ACTIVATED_IMAGE_ROT:
		{
			mActivatedRot = StringConverter::ParseBool(val);
			auto activeImage = mImages[ButtonImages::ActiveImage].lock();
			assert(activeImage);
			activeImage->SetUVRot(StringConverter::ParseBool(val));
			return true;
		}

		case UIProperty::BUTTON_ACTIVATED:
		{
			mActivated = StringConverter::ParseBool(val);
			auto activeImage = mImages[ButtonImages::ActiveImage].lock();
			auto deactiveImage = mImages[ButtonImages::DeactiveImage].lock();
			if (activeImage)
			{
				if (mActivated)
				{
					activeImage->SetVisible(true);
				}
				else
				{
					activeImage->SetVisible(false);
				}
			}

			if (deactiveImage)
			{
				if (mActivated)
				{
					deactiveImage->SetVisible(false);
				}
				else
				{
					deactiveImage->SetVisible(true);
				}
			}
			UIManager::GetInstance().DirtyRenderList(GetHwndId());
			return true;
		}

		case UIProperty::BUTTON_ICON_TEXT:
		{
			mButtonIconSize = StringConverter::ParseInt(val);
			if (mButtonIconSize == 0)
			{
				mIconText = false;
			}
			else
			{
				mIconText = true;
				auto image = mImages[ButtonImages::Image].lock();
				if (image)
				{
					image->ChangeSize(Vec2I(mButtonIconSize, mButtonIconSize));
				}
				AlignIconText();
			}

			return true;
		}

		case UIProperty::CHANGE_IMAGE_ACTIVATE:
		{
			mChangeImageActivation = StringConverter::ParseBool(val);
			return true;
		}

		case UIProperty::PROGRESSBAR:
		{
			if (strcmp(val, "0") != 0){
				RemoveChild(mProgressBar.lock());
				auto progressBar = std::dynamic_pointer_cast<HorizontalGauge>(AddChild(Vec2I(0, 0), GetFinalSize(), ComponentType::HorizontalGauge));
				mProgressBar = progressBar;
				progressBar->SetUseAbsSize(false);
				progressBar->SetRuntimeChild(true);
				progressBar->SetGhost(true);
				progressBar->SetGatheringException();
				progressBar->SetVisible(GetVisible());
				progressBar->SetMaximum(StringConverter::ParseReal(val));
			}
			return true;
		}

		case UIProperty::GAUGE_CUR:
		{
			if (!mProgressBar.expired())
				SetPercentage(StringConverter::ParseReal(val));
			return true;
		}

		case UIProperty::GAUGE_COLOR:
		{
			auto progressBar = mProgressBar.lock();
			if (progressBar)
				progressBar->SetGaugeColor(Color(val));

			return true;
		}

		case UIProperty::GAUGE_BLINK_COLOR:
		{
			auto progressBar = mProgressBar.lock();
			if (progressBar)
				progressBar->SetBlinkColor(Color(val));

			return true;
		}

		case UIProperty::GAUGE_BLINK_SPEED:
		{
			auto progressBar = mProgressBar.lock();
			if (progressBar)
				progressBar->SetProperty(prop, val);
			return true;
		}

		case UIProperty::EDGE_COLOR:
		{
			mEdgeColor = Color(val);
			assert(mUIObject);
			mUIObject->GetMaterial()->SetMaterialParameter(1, mEdgeColor.GetVec4());
			return true;
		}

		case UIProperty::EDGE_COLOR_OVER:
		{
			mEdgeColorOver = Color(val);
			return true;
		}

		case UIProperty::IMAGE_COLOR_OVERLAY:
		{
			mImageColorOverlay = StringMathConverter::ParseColor(val);
			auto image = mImages[ButtonImages::Image].lock();
			if (image)
			{
				return image->SetProperty(UIProperty::IMAGE_COLOR_OVERLAY, val);
			}
			return true;
		}
		case UIProperty::NO_BUTTON:
		{
			mNoButton = StringConverter::ParseBool(val);
			return true;
		}

		case UIProperty::ALPHA_REGION:
		{
			mAlphaRegion = val;
			auto mat = mUIObject->GetMaterial();
			assert(mat);
			if (!mAlphaRegion.empty()){
				mat->AddShaderDefine("_ALPHA_TEXTURE", "1");
			}
			else{
				mat->RemoveShaderDefine("_ALPHA_TEXTURE");
			}
			//mat->ApplyShaderDefines();
			SetAlphaRegionTexture();
			return true;
		}

		case UIProperty::BUTTON_IMAGE_SIZE:
		{
			mImageSize = StringMathConverter::ParseVec2I(val);
			UpdateImageSize();
			return true;
		}

		}

		return __super::SetProperty(prop, val);
	}

	bool Button::GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly)
	{
		switch (prop)
		{
			/*case UIProperty::BACK_COLOR: -- Will be handled in the WinBase
			{

			}*/
		case UIProperty::BACK_COLOR_OVER:
		{
			if (notDefaultOnly){
				if (mBackColorOver == UIProperty::GetDefaultValueVec4(prop))
				{
					return false;
				}
			}
			auto data = StringConverter::ToString(mBackColorOver);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}
		case UIProperty::BACK_COLOR_DOWN:
		{
			if (notDefaultOnly){
				if (mBackColorDown == UIProperty::GetDefaultValueVec4(prop))
				{
					return false;
				}
			}
			auto data = StringConverter::ToString(mBackColorDown);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}

		case UIProperty::TEXTUREATLAS:
		{
			if (notDefaultOnly){
				if (mImageAtlas.empty())
				{
					return false;
				}
			}

			strcpy_s(val, bufsize, mImageAtlas.c_str());
			return true;
		}
		case UIProperty::REGION:
		{
			if (notDefaultOnly)
			{
				if (mRegionName.empty())
					return false;
			}

			strcpy_s(val, bufsize, mRegionName.c_str());
			return true;
		}
		case UIProperty::REGIONS:
		{
			if (notDefaultOnly)
			{
				if (mRegionNames.empty())
					return false;
			}

			strcpy_s(val, bufsize, mRegionNames.c_str());
			return true;
		}
		case UIProperty::TEXTURE_FILE:
		{
			if (notDefaultOnly)
			{
				if (mTextureFile.empty())
					return false;
			}

			strcpy_s(val, bufsize, mTextureFile.c_str());
			return true;
		}
		case UIProperty::FPS:
		{
			if (notDefaultOnly)
			{
				if (mFps == UIProperty::GetDefaultValueFloat(prop))
					return false;
			}
			auto data = StringConverter::ToString(mFps);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}
		case UIProperty::HOVER_IMAGE:
		{
			if (notDefaultOnly)
			{
				if (mHorverImage.empty())
					return false;
			}

			strcpy_s(val, bufsize, mHorverImage.c_str());
			return true;
		}
		case UIProperty::BACKGROUND_IMAGE:
		{
			if (notDefaultOnly)
			{
				if (mBackgroundImage.empty())
					return false;
			}

			strcpy_s(val, bufsize, mBackgroundImage.c_str());
			return true;
		}
		case UIProperty::BACKGROUND_IMAGE_DISABLED:
		{
			if (notDefaultOnly)
			{
				if (mBackgroundImageDisabled.empty())
					return false;
			}

			strcpy_s(val, bufsize, mBackgroundImageDisabled.c_str());
			return true;
		}

		case UIProperty::BACKGROUND_IMAGE_HOVER:
		{
			if (notDefaultOnly)
			{
				if (mBackgoundImageHover.empty())
					return false;
			}

			strcpy_s(val, bufsize, mBackgoundImageHover.c_str());
			return true;
		}

		case UIProperty::BACKGROUND_IMAGE_NOATLAS:
		{
			if (notDefaultOnly)
			{
				if (mBackgroundImageNoAtlas.empty())
					return false;
			}

			strcpy_s(val, bufsize, mBackgroundImageNoAtlas.c_str());
			return true;
		}

		case UIProperty::BACKGROUND_IMAGE_HOVER_NOATLAS:
		{
			if (notDefaultOnly)
			{
				if (mBackgroundImageHoverNoAtlas.empty())
					return false;
			}

			strcpy_s(val, bufsize, mBackgroundImageHoverNoAtlas.c_str());
			return true;
		}

		case UIProperty::FRAME_IMAGE:
		{
			if (notDefaultOnly)
			{
				if (mFrameImage.empty())
					return false;
			}

			strcpy_s(val, bufsize, mFrameImage.c_str());
			return true;

		}

		case UIProperty::FRAME_IMAGE_DISABLED:
		{
			if (notDefaultOnly)
			{
				if (mFrameImageDisabled.empty())
					return false;
			}

			strcpy_s(val, bufsize, mFrameImageDisabled.c_str());
			return true;
		}

		case UIProperty::ACTIVATED_IMAGE:
		{
			if (notDefaultOnly)
			{
				if (mActivatedImage.empty())
					return false;
			}

			strcpy_s(val, bufsize, mActivatedImage.c_str());
			return true;
		}

		case UIProperty::DEACTIVATED_IMAGE:
		{
			if (notDefaultOnly)
			{
				if (mDeactivatedImage.empty())
					return false;
			}

			strcpy_s(val, bufsize, mDeactivatedImage.c_str());
			return true;
		}

		case UIProperty::ACTIVATED_IMAGE_ROT:
		{
			if (notDefaultOnly)
			{
				if (mActivatedRot == UIProperty::GetDefaultValueBool(prop))
					return false;
			}
			auto data = StringConverter::ToString(mActivatedRot);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}

		case UIProperty::BUTTON_ACTIVATED:
		{
			if (notDefaultOnly)
			{
				if (mActivated == UIProperty::GetDefaultValueBool(prop))
					return false;
			}
			auto data = StringConverter::ToString(mActivated);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}

		case UIProperty::BUTTON_ICON_TEXT:
		{
			if (notDefaultOnly)
			{
				if (mButtonIconSize == UIProperty::GetDefaultValueInt(prop))
					return false;
			}
			auto data = StringConverter::ToString(mButtonIconSize);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}

		case UIProperty::CHANGE_IMAGE_ACTIVATE:
		{
			if (notDefaultOnly)
			{
				if (mChangeImageActivation == UIProperty::GetDefaultValueBool(prop))
					return false;
			}
			auto data = StringConverter::ToString(mChangeImageActivation);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}

		case UIProperty::PROGRESSBAR:
		{
			if (notDefaultOnly)
			{
				if (mProgressBar.expired())
					return false;
			}

			if (mProgressBar.expired())
			{
				strcpy_s(val, bufsize, "0");
				return true;
			}
			auto progressBar = mProgressBar.lock();
			auto data = StringConverter::ToString(progressBar->GetMaximum());
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}

		case UIProperty::GAUGE_CUR:
		{
			if (notDefaultOnly)
			{
				if (mProgressBar.expired())
					return false;
			}

			if (mProgressBar.expired()){
				strcpy_s(val, bufsize, "0");
				return true;
			}

			auto progressBar = mProgressBar.lock();
			auto data = StringConverter::ToString(progressBar->GetPercentage());
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}

		case UIProperty::GAUGE_COLOR:
		case UIProperty::GAUGE_BLINK_COLOR:
		case UIProperty::GAUGE_BLINK_SPEED:
		{
			if (mProgressBar.expired())
				return false;

			return mProgressBar.lock()->GetProperty(prop, val, bufsize, notDefaultOnly);
		}

		case UIProperty::EDGE_COLOR:
		{
			if (notDefaultOnly)
			{
				if (mEdgeColor == UIProperty::GetDefaultValueVec4(prop))
				{
					return false;
				}
			}
			auto data = StringConverter::ToString(mEdgeColor);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}

		case UIProperty::EDGE_COLOR_OVER:
		{
			if (notDefaultOnly)
			{
				if (mEdgeColorOver == UIProperty::GetDefaultValueVec4(prop))
				{
					return false;
				}
			}
			auto data = StringConverter::ToString(mEdgeColorOver);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}

		case UIProperty::IMAGE_COLOR_OVERLAY:
		{
			if (notDefaultOnly)
			{
				if (mImageColorOverlay == UIProperty::GetDefaultValueVec4(prop))
				{
					return false;
				}
			}
			auto data = StringConverter::ToString(mImageColorOverlay);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}
		case UIProperty::NO_BUTTON:
		{
			if (notDefaultOnly)
			{
				if (mNoButton == UIProperty::GetDefaultValueBool(prop))
				{
					return false;
				}
			}
			auto data = StringConverter::ToString(mNoButton);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}

		case UIProperty::ALPHA_REGION:
		{
			if (notDefaultOnly){
				if (mAlphaRegion.empty())
					return false;
			}
			strcpy_s(val, bufsize, mAlphaRegion.c_str());
			return true;
		}

		case UIProperty::BUTTON_IMAGE_SIZE:
		{
			if (notDefaultOnly){
				if (mImageSize == UIProperty::GetDefaultValueVec2I(prop))
					return false;
			}
			strcpy_s(val, bufsize, StringMathConverter::ToString(mImageSize).c_str());
			return true;
		}

		}

		return __super::GetProperty(prop, val, bufsize, notDefaultOnly);
	}

	bool Button::SetVisible(bool visible){
		bool changed = __super::SetVisible(visible);
		for (int i = 0; i < ButtonImages::ActiveImage; ++i){
			auto image = mImages[i].lock();
			if (image)
				image->SetVisible(visible);
		}
		return changed;
	}

	void Button::SetVisibleInternal(bool visible){
		__super::SetVisibleInternal(visible);
		for (int i = 0; i < ButtonImages::ActiveImage; ++i){
			auto image = mImages[i].lock();
			if (image){
				image->SetVisible(visible);
			}
		}
	}


	ImageBoxPtr Button::CreateImageBox()
	{
		auto image = std::static_pointer_cast<ImageBox>(AddChild(Vec2I(0, 0), GetFinalSize(), ComponentType::ImageBox));
		image->SetSpecialOrder(GetSpecialOrder());
		image->SetUseAbsSize(false);
		image->SetRuntimeChild(true);
		image->SetRender3D(mRender3D, GetRenderTargetSize());
		image->SetVisible(true);
		image->SetProperty(UIProperty::NO_MOUSE_EVENT, "true");
		UIManager::GetInstance().DirtyRenderList(GetHwndId());
		image->SetGatheringException();
		return image;
	}

	void Button::StartProgress()
	{
		if (mInProgress)
			return;
		mInProgress = true;
		mNoBackgroundBackup = mUIObject->GetNoDrawBackground();
		auto progressBar = mProgressBar.lock();
		if (progressBar)
		{
			progressBar->SetPercentage(0);
			progressBar->SetVisible(true);
		}

		UIManager::GetInstance().DirtyRenderList(GetHwndId());
	}

	void Button::SetPercentage(float p) // progress bar
	{
		auto progressBar = mProgressBar.lock();
		if (progressBar)
		{
			if (!mInProgress)
			{
				mInProgress = true;
				progressBar->SetVisible(true);
			}


			progressBar->SetPercentage(p);
		}
	}

	void Button::Blink(bool blink)
	{
		auto progressBar = mProgressBar.lock();
		if (progressBar)
			progressBar->Blink(blink);
	}

	void Button::Blink(bool blink, float time) // progress bar
	{
		auto progressBar = mProgressBar.lock();
		if (progressBar)
			progressBar->Blink(blink, time);
	}

	void Button::EndProgress()
	{
		if (!mInProgress)
			return;
		mInProgress = false;
		auto progressBar = mProgressBar.lock();
		if (progressBar)
			progressBar->SetVisible(false);
		
		UIManager::GetInstance().DirtyRenderList(GetHwndId());
	}

	void Button::Highlight(bool highlight)
	{
		if (highlight && mEnable)
		{
			mUIObject->GetMaterial()->SetAmbientColor(0.09f, 0.02f, 0.03f, 1.0f);
		}
		else
		{
			mUIObject->GetMaterial()->SetAmbientColor(0, 0, 0, 1.0f);
		}
	}

	void Button::SetTexture(ButtonImages::Enum type, TexturePtr pTexture, bool drawFixedSize)
	{
		if (mImages[type].expired())
		{
			mImages[type] = CreateImageBox();
		}
		auto image = mImages[type].lock();
		image->SetTexture(pTexture);
		if (drawFixedSize){
			image->DrawAsFixedSizeAtCenter();
		}
	}

	void Button::OnEnableChanged()
	{
		if (mEnable)
		{
			for (int i = 0; i < ButtonImages::Num; ++i)
			{
				auto image = mImages[i].lock();
				if (image)
					image->SetDesaturate(false);
			}
		}
		else
		{
			for (int i = 0; i < ButtonImages::Num; ++i)
			{
				auto image = mImages[i].lock();
				if (image)
					image->SetDesaturate(true);
			}
		}
	}

	void Button::AlignIconText()
	{
		const int IconTextGap = 2;
		auto image = mImages[ButtonImages::Image].lock();
		if (image)
		{
			image->SetAlign(ALIGNH::LEFT, ALIGNV::MIDDLE);
			int iconSize = image->GetFinalSize().x;
			int textSize = mTextWidth;
			int buttonSize = GetFinalSize().x;
			int buttonCenter = Round(buttonSize*.5f);

			int contentSize = iconSize + textSize + IconTextGap;

			int sizeGap = buttonSize - contentSize;
			int iconStart = Round(sizeGap*.5f);
			int textStart = iconStart + iconSize + IconTextGap;
			int textCenter = textStart + Round(textSize * .5f);

			image->ChangeNPos(Vec2(0.f, 0.5f));
			image->SetInitialOffset(Vec2I(((int)iconStart), 0));
			image->UpdateWorldPos();
			mTextGap.x = textCenter - buttonCenter;
			AlignText();
		}
	}


	void Button::SetAlphaRegionTexture(){
		SetDefaultImageAtlasPathIfNotSet();
		auto mat = mUIObject->GetMaterial();
		assert(mat);
		if (mAlphaRegion.empty()){
			mat->SetTexture(0, BINDING_SHADER_PS, 1);
		}
		else{
			auto atlas = Renderer::GetInstance().GetTextureAtlas(mImageAtlas.c_str());
			assert(atlas);
			auto region = atlas->GetRegion(mAlphaRegion.c_str());
			assert(region);
			auto& startUv = region->GetStartUV();
			auto endUv = startUv + region->GetUVSize();
			Vec2 texcoords[4] = {
				Vec2(startUv.x, endUv.y),
				Vec2(startUv.x, startUv.y),
				Vec2(endUv.x, endUv.y),
				Vec2(endUv.x, startUv.y)
			};
			mUIObject->SetTexCoord(texcoords, 4);

			mat->SetTexture(atlas->GetTexture(), BINDING_SHADER_PS, 1);
		}
	}

	void Button::SetDefaultImageAtlasPathIfNotSet(){
		if (mImageAtlas.empty()){
			mImageAtlas = "data/textures/gameui.xml";
		}
	}


	void Button::UpdateImageSize(){
		Vec2I finalSize;
		if (mImageSize != Vec2I(0, 0)){
			finalSize = mImageSize;
		}
		else{
			finalSize = GetFinalSize();
		}

		auto image = mImages[ButtonImages::Image].lock();
		if (image){
			image->ChangeSize(finalSize);
		}
		auto imageHover = mImages[ButtonImages::ImageHover].lock();
		if (imageHover){
			imageHover->ChangeSize(finalSize);
		}
		auto activeImage = mImages[ButtonImages::ActiveImage].lock();
		if (activeImage){
			activeImage->ChangeSize(finalSize);
		}
		/*if (mImages[ButtonImages::DeactiveImage]){
			mImages[ButtonImages::DeactiveImage]->ChangeSize(finalSize);
			}*/
	}
	void Button::SetEnable(bool enable){
		if (!enable){
			this->OnMouseOut(0);
		}
		__super::SetEnable(enable);
	}

	void Button::SetUseBorder(bool use){
		__super::SetUseBorder(use);
		for (unsigned i = 0; i < ButtonImages::Num; ++i){
			auto image = mImages[i].lock();
			if (image){
				image->SetBorderAlpha(use);
			}
		}
	}
}
