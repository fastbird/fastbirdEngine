#include <UI/StdAfx.h>
#include <UI/Button.h>
#include <UI/ImageBox.h>
#include <UI/IUIManager.h>
#include <UI/HorizontalGauge.h>
#include <Engine/GlobalEnv.h>
#include <CommonLib/StringUtils.h>

namespace fastbird
{

const float Button::LEFT_GAP = 0.001f;

Button::Button()
	: mProgressBar(0)
	, mInProgress(false)
	, mNoBackgroundBackup(false)
	, mActivated(false)
	, mChangeImageActivation(false)
	, mIconText(false)
	, mNoButton(false)
	, mButtonIconSize(0)
	, mFps(0), mActivatedRot(false), mImageColorOverlay(1, 1, 1, 1)
{
	for (int i = 0; i < ButtonImages::Num; i++)
	{
		mImages[i] = 0;
	}

	mUIObject = gFBEnv->pEngine->CreateUIObject(false, GetRenderTargetSize());
	mUIObject->SetMaterial("es/Materials/UIButton.material");
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	mTextColor = Color(1.0f, 0.92f, 0.0f);
	mTextColorHover = Color(1.0f, 1.0f, 0.5f);
	mTextColorDown = Color(1.0f, 0.2f, 0.2f);
	mUIObject->SetTextColor(mTextColor);

	// default colors
	mBackColor = Color(0.0f, 0.0f, 0.0f, 0.7f);
	mBackColorOver = Color(0.09f, 0.02f, 0.03f, 0.8f);
	mBackColorDown = Color(0.3f, 0.3f, 0.f, 0.5f);
	mEdgeColor = Color(1.f, 1.f, 1.f, 0.7f);
	mEdgeColorOver = Color(0.9f, 0.85f, 0.0f, 0.7f);

	mUIObject->GetMaterial()->SetDiffuseColor(mBackColor.GetVec4());
	// material param 0 : ratio, sizes
	// material param 1 : edge color
	mUIObject->GetMaterial()->SetMaterialParameters(1, mEdgeColor.GetVec4());
	
	RegisterEventFunc(IEventHandler::EVENT_MOUSE_DOWN, 
		std::bind(&Button::OnMouseDown, this, std::placeholders::_1));
	RegisterEventFunc(IEventHandler::EVENT_MOUSE_HOVER, 
		std::bind(&Button::OnMouseHover, this, std::placeholders::_1));
	RegisterEventFunc(IEventHandler::EVENT_MOUSE_OUT, 
		std::bind(&Button::OnMouseOut, this, std::placeholders::_1));
}

Button::~Button()
{
	for (int i = 0; i < ButtonImages::Num; ++i)
	{
		auto p = mImages[i];
		FB_DELETE(p);
		mImages[i] = 0;
	}
}

void Button::OnPosChanged(bool anim)
{
	__super::OnPosChanged(anim);
	if (mButtonIconSize>0)
		AlignIconText();
	else
	{
		for (int i = 0; i < ButtonImages::Num; ++i)
		{
			if (mImages[i])
			{
				mImages[i]->DrawAsFixedSizeCenteredAt(GetFinalPos() + Round(GetFinalSize() * .5f));
			}
		}
	}		
}

void Button::OnSizeChanged()
{
	__super::OnSizeChanged();
	Vec2 texcoords[4] = {
		Vec2(0.f, 1.f),
		Vec2(0.f, 0.f),
		Vec2(1.f, 1.f),
		Vec2(1.f, 0.f)
	};
	mUIObject->SetTexCoord(texcoords, 4);

	for (int i = 0; i < ButtonImages::Num; ++i)
	{
		if (mImages[i])
			mImages[i]->DrawAsFixedSizeCenteredAt(
				GetFinalPos() + Round(GetFinalSize() * .5f));
	}
}

void Button::GatherVisit(std::vector<IUIObject*>& v)
{
	if (!mVisibility.IsVisible())
		return;	
	assert(mUIObject);

	if (mProgressBar && mInProgress)
		mProgressBar->GatherVisit(v);
	
	if (mMouseIn && mImages[ButtonImages::BackImageHover])
		mImages[ButtonImages::BackImageHover]->GatherVisit(v);
	else if (mImages[ButtonImages::BackImage])
		mImages[ButtonImages::BackImage]->GatherVisit(v);

	v.push_back(mUIObject);

	if (mImages[ButtonImages::ActiveImage] && mImages[ButtonImages::ActiveImage]->GetVisible())
	{
		if (mChangeImageActivation)
		{
			mImages[ButtonImages::ActiveImage]->GatherVisit(v);
		}
		else
		{
			if (mMouseIn && mImages[ButtonImages::ImageHover])
				mImages[ButtonImages::ImageHover]->GatherVisit(v);
			else if(mImages[ButtonImages::Image])
				mImages[ButtonImages::Image]->GatherVisit(v);
			mImages[ButtonImages::ActiveImage]->GatherVisit(v);
		}
	}
	else if (mImages[ButtonImages::DeactiveImage] && mImages[ButtonImages::DeactiveImage]->GetVisible())
	{
		if (mChangeImageActivation)
		{
			mImages[ButtonImages::DeactiveImage]->GatherVisit(v);
		}
		else
		{
			if (mMouseIn && mImages[ButtonImages::ImageHover])
				mImages[ButtonImages::ImageHover]->GatherVisit(v);
			else if (mImages[ButtonImages::Image])
				mImages[ButtonImages::Image]->GatherVisit(v);
			mImages[ButtonImages::DeactiveImage]->GatherVisit(v);
		}
	}
	else
	{
		if (mMouseIn && mImages[ButtonImages::ImageHover])
			mImages[ButtonImages::ImageHover]->GatherVisit(v);
		else if (mImages[ButtonImages::Image])
			mImages[ButtonImages::Image]->GatherVisit(v);
	}		

	if (mImages[ButtonImages::FrameImage])
		mImages[ButtonImages::FrameImage]->GatherVisit(v);

	__super::GatherVisit(v);
}

void Button::OnMouseDown(void* arg)
{
	if (!mEnable || mNoButton)
		return;
	mUIObject->GetMaterial()->SetDiffuseColor(mBackColorDown.GetVec4());
	mUIObject->GetMaterial()->SetEmissiveColor(1.0f, 1.0f, 0.2f, 1);
	mUIObject->SetTextColor(mEnable ? mTextColorDown : mTextColorDown*.5f);

	if (mImages[ButtonImages::BackImageHover])
		mImages[ButtonImages::BackImageHover]->SetSpecularColor(Vec4(0.5f, 0.1f, 0.1f, 1.0f));
	else if (mImages[ButtonImages::BackImage])
		mImages[ButtonImages::BackImage]->SetSpecularColor(Vec4(0.5f, 0.1f, 0.1f, 1.0f));
	else if (mImages[ButtonImages::ImageHover])
		mImages[ButtonImages::ImageHover]->SetSpecularColor(Vec4(0.5f, 0.1f, 0.1f, 1.0f));
	else if (mImages[ButtonImages::Image])
		mImages[ButtonImages::Image]->SetSpecularColor(Vec4(0.5f, 0.1f, 0.1f, 1.0f));
}

void Button::OnMouseHover(void* arg)
{
	if (!mEnable || mNoButton)
		return;
	mUIObject->GetMaterial()->SetDiffuseColor(mBackColorOver.GetVec4());
	mUIObject->SetTextColor(mEnable ? mTextColorHover : mTextColorHover * .5f);
	SetCursor(WinBase::sCursorOver);

	mUIObject->GetMaterial()->SetEmissiveColor(0.8f, 0.8f, 0.1f, 1);
	//  1 is edge color
	mUIObject->GetMaterial()->SetMaterialParameters(1, mEdgeColorOver.GetVec4());
	if (mImages[ButtonImages::ImageHover] || mImages[ButtonImages::BackImageHover])
		gFBEnv->pUIManager->DirtyRenderList(GetHwndId());

	if (mImages[ButtonImages::BackImageHover])
		mImages[ButtonImages::BackImageHover]->SetSpecularColor(Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	else if (mImages[ButtonImages::BackImage])
		mImages[ButtonImages::BackImage]->SetSpecularColor(Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	else if (mImages[ButtonImages::ImageHover])
		mImages[ButtonImages::ImageHover]->SetSpecularColor(Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	else if (mImages[ButtonImages::Image])
		mImages[ButtonImages::Image]->SetSpecularColor(Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	

	// sometimes OnMouseHover function is called manually even the mouse
	// is in out. - DropDown
	mMouseIn = true;

}

void Button::OnMouseOut(void* arg)
{
	if (!mEnable || mNoButton)
		return;
	mUIObject->GetMaterial()->SetDiffuseColor(mBackColor.GetVec4());
	mUIObject->SetTextColor(mEnable ? mTextColor : mTextColor*.5f);
	mUIObject->GetMaterial()->SetEmissiveColor(0, 0, 0, 0);
	//  1 is edge color
	mUIObject->GetMaterial()->SetMaterialParameters(1, mEdgeColor.GetVec4());
	if (mImages[ButtonImages::ImageHover] || mImages[ButtonImages::BackImageHover])
		gFBEnv->pUIManager->DirtyRenderList(GetHwndId());

	

	if (mImages[ButtonImages::BackImageHover])
		mImages[ButtonImages::BackImageHover]->SetSpecularColor(Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	else if (mImages[ButtonImages::BackImage])
		mImages[ButtonImages::BackImage]->SetSpecularColor(Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	else if (mImages[ButtonImages::ImageHover])
		mImages[ButtonImages::ImageHover]->SetSpecularColor(Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	else if (mImages[ButtonImages::Image])
		mImages[ButtonImages::Image]->SetSpecularColor(Vec4(1.0f, 1.0f, 1.0f, 1.0f));

	// sometimes OnMouseHover function is called manually even the mouse
	// is in out. - DropDown

	mMouseIn = false;
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
							   if (!mImages[ButtonImages::Image])
							   {
								   mImages[ButtonImages::Image]= CreateImageBox();
							   }
							   else
							   {
								   FB_DELETE(mImages[ButtonImages::Image]);
								   mImages[ButtonImages::Image] = CreateImageBox();
							   }
							   assert(!mImageAtlas.empty());
							   mImages[ButtonImages::Image]->SetTextureAtlasRegion(mImageAtlas.c_str(), val);
							   mImages[ButtonImages::Image]->DrawAsFixedSizeAtCenter();
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
							   if (!mImages[ButtonImages::Image])
							   {
								   mImages[ButtonImages::Image] = CreateImageBox();
							   }
							   else
							   {
								   FB_DELETE(mImages[ButtonImages::Image]);
								   mImages[ButtonImages::Image] = CreateImageBox();
							   }
							   assert(!mImageAtlas.empty());
							   mImages[ButtonImages::Image]->SetProperty(UIProperty::TEXTUREATLAS, mImageAtlas.c_str());
							   mImages[ButtonImages::Image]->SetProperty(prop, val);
							   mImages[ButtonImages::Image]->DrawAsFixedSizeCenteredAt(GetFinalPos() + Round(GetFinalSize() * .5f));
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
									 if (!mImages[ButtonImages::Image])
									 {
										 mImages[ButtonImages::Image] = CreateImageBox();
									 }
									 else
									 {
										 FB_DELETE(mImages[ButtonImages::Image]);
										 mImages[ButtonImages::Image] = CreateImageBox();
									 }
									 
									 mImages[ButtonImages::Image]->SetTexture(val);
									 mImages[ButtonImages::Image]->DrawAsFixedSizeAtCenter();
									 if (mIconText)
									 {
										 OnSizeChanged();
										 AlignIconText();
									 }
									return true;
	}
	case UIProperty::FPS:
	{
		mFps = StringConverter::parseReal(val);
							if_assert_pass(mImages[ButtonImages::Image])
							{
								mImages[ButtonImages::Image]->SetProperty(prop, val);
							}
							return true;
	}
	case UIProperty::HOVER_IMAGE:
	{
		mHorverImage = val;
									if (!mImages[ButtonImages::ImageHover])
									{
										mImages[ButtonImages::ImageHover] = CreateImageBox();
									}
									assert(!mImageAtlas.empty());
									if (strlen(val) == 0)
									{
										FB_DELETE(mImages[ButtonImages::ImageHover]);
										mImages[ButtonImages::ImageHover] = 0;
									}
									else
									{
										mImages[ButtonImages::ImageHover]->SetTextureAtlasRegion(mImageAtlas.c_str(), val);
										mImages[ButtonImages::ImageHover]->DrawAsFixedSizeCenteredAt(GetFinalPos() + Round(GetFinalSize() * .5f));
									}
									
									return true;
	}
	case UIProperty::BACKGROUND_IMAGE:
	{
		mBackgroundImage = val;
										 if (!mImages[ButtonImages::BackImage])
										 {
											 mImages[ButtonImages::BackImage] = CreateImageBox();
										 }
										 assert(!mImageAtlas.empty());
										 mImages[ButtonImages::BackImage]->SetTextureAtlasRegion(mImageAtlas.c_str(), val);
										 mImages[ButtonImages::BackImage]->DrawAsFixedSizeCenteredAt(GetFinalPos() + Round(GetFinalSize() * .5f));
										 return true;
	}
	case UIProperty::BACKGROUND_IMAGE_DISABLED:
	{
		mBackgroundImageDisabled = val;
										 if (!mImages[ButtonImages::BackImageDisabled])
										 {
											 mImages[ButtonImages::BackImageDisabled] = CreateImageBox();
										 }
										 assert(!mImageAtlas.empty());
										 mImages[ButtonImages::BackImageDisabled]->SetTextureAtlasRegion(mImageAtlas.c_str(), val);
										 mImages[ButtonImages::BackImageDisabled]->DrawAsFixedSizeCenteredAt(GetFinalPos() + Round(GetFinalSize() * .5f));
										 return true;
	}

	case UIProperty::BACKGROUND_IMAGE_HOVER:
	{
		mBackgoundImageHover = val;
											   if (!mImages[ButtonImages::BackImageHover])
											   {
												   mImages[ButtonImages::BackImageHover] = CreateImageBox();
											   }

											   assert(!mImageAtlas.empty());
											   mImages[ButtonImages::BackImageHover]->SetTextureAtlasRegion(mImageAtlas.c_str(), val);
											   mImages[ButtonImages::BackImageHover]->DrawAsFixedSizeCenteredAt(GetFinalPos() + Round(GetFinalSize() * .5f));
											   return true;
	}

	case UIProperty::BACKGROUND_IMAGE_NOATLAS:
	{
		mBackgroundImageNoAtlas = val;
										if (!mImages[ButtonImages::BackImage])
										 {
											mImages[ButtonImages::BackImage] = CreateImageBox();
										 }

										mImages[ButtonImages::BackImage]->SetTexture(val);
										mImages[ButtonImages::BackImage]->DrawAsFixedSizeCenteredAt(GetFinalPos() + Round(GetFinalSize() * .5f));
										 return true;
	}

	case UIProperty::BACKGROUND_IMAGE_HOVER_NOATLAS:
	{
		mBackgroundImageHoverNoAtlas = val;
												if (!mImages[ButtonImages::BackImageHover])
											   {
													mImages[ButtonImages::BackImageHover] = CreateImageBox();
											   }

											   
												mImages[ButtonImages::BackImageHover]->SetTexture(val);
												mImages[ButtonImages::BackImageHover]->DrawAsFixedSizeCenteredAt(GetFinalPos() + Round(GetFinalSize() * .5f));
											   return true;
	}

	case UIProperty::FRAME_IMAGE:
	{
		mFrameImage = val;
										if (!mImages[ButtonImages::FrameImage])
										 {
											mImages[ButtonImages::FrameImage] = CreateImageBox();
										 }

										 assert(!mImageAtlas.empty());
										 mImages[ButtonImages::FrameImage]->SetTextureAtlasRegion(mImageAtlas.c_str(), val);
										 if (strlen(val) == 0)
										 {
											 mImages[ButtonImages::FrameImage]->SetVisible(false);
										 }
										 else
										 {
											 mImages[ButtonImages::FrameImage]->SetVisible(true);
										 }
										 
										 gFBEnv->pUIManager->DirtyRenderList(GetHwndId());
										 return true;
	}

	case UIProperty::FRAME_IMAGE_DISABLED:
	{
		mFrameImageDisabled = val;
										if (!mImages[ButtonImages::FrameImageDisabled])
										{
											mImages[ButtonImages::FrameImageDisabled] = CreateImageBox();
										}

										assert(!mImageAtlas.empty());
										mImages[ButtonImages::FrameImageDisabled]->SetTextureAtlasRegion(mImageAtlas.c_str(), val);
										if (strlen(val) == 0)
										{
											mImages[ButtonImages::FrameImageDisabled]->SetVisible(false);
										}
										else
										{
											mImages[ButtonImages::FrameImageDisabled]->SetVisible(true);
										}

										gFBEnv->pUIManager->DirtyRenderList(GetHwndId());
										return true;
	}

	case UIProperty::ACTIVATED_IMAGE:
	{
		mActivatedImage = val;
										if (!mImages[ButtonImages::ActiveImage])
										{
											mImages[ButtonImages::ActiveImage] = CreateImageBox();
										}

										if (strlen(val) == 0)
										{
											mImages[ButtonImages::ActiveImage]->SetVisible(false);
										}
										else
										{
											assert(!mImageAtlas.empty());
											mImages[ButtonImages::ActiveImage]->SetTextureAtlasRegion(mImageAtlas.c_str(), val);
											mImages[ButtonImages::ActiveImage]->SetVisible(false);
											mImages[ButtonImages::ActiveImage]->SetCenterUVMatParam();
										}
										mImages[ButtonImages::ActiveImage]->DrawAsFixedSizeCenteredAt(GetFinalPos() + Round(GetFinalSize() * .5f));

										return true;
	}

	case UIProperty::DEACTIVATED_IMAGE:
	{
		mDeactivatedImage = val;
										  if (!mImages[ButtonImages::DeactiveImage])
										  {
											  mImages[ButtonImages::DeactiveImage] = CreateImageBox();
										  }

										  if (strlen(val) == 0)
										  {
											  mImages[ButtonImages::DeactiveImage]->SetVisible(false);
										  }
										  else
										  {
											  assert(!mImageAtlas.empty());
											  mImages[ButtonImages::DeactiveImage]->SetTextureAtlasRegion(mImageAtlas.c_str(), val);
											  mImages[ButtonImages::DeactiveImage]->SetVisible(false);
											  mImages[ButtonImages::DeactiveImage]->SetCenterUVMatParam();
										  }
										  mImages[ButtonImages::DeactiveImage]->DrawAsFixedSizeCenteredAt(GetFinalPos() + Round(GetFinalSize() * .5f));
										  return true;
	}

	case UIProperty::ACTIVATED_IMAGE_ROT:
	{
		mActivatedRot = StringConverter::parseBool(val);
											assert(mImages[ButtonImages::ActiveImage]);
											mImages[ButtonImages::ActiveImage]->SetUVRot(StringConverter::parseBool(val));
											return true;
	}

	case UIProperty::BUTTON_ACTIVATED:
	{
										 mActivated = StringConverter::parseBool(val);
										 if (mImages[ButtonImages::ActiveImage])
										 {
											 if (mActivated)
											 {
												 mImages[ButtonImages::ActiveImage]->SetVisible(true);
											 }
											 else
											 {
												 mImages[ButtonImages::ActiveImage]->SetVisible(false);
											 }
											 gFBEnv->pUIManager->DirtyRenderList(GetHwndId());
										 }

										 if (mImages[ButtonImages::DeactiveImage])
										 {
											 if (mActivated)
											 {
												 mImages[ButtonImages::DeactiveImage]->SetVisible(false);
											 }
											 else
											 {
												 mImages[ButtonImages::DeactiveImage]->SetVisible(true);
											 }
										 }
										 return true;
	}

	case UIProperty::BUTTON_ICON_TEXT:
	{
										 mButtonIconSize = StringConverter::parseInt(val);
										 if (mButtonIconSize == 0)
										 {
											 mIconText = false;
										 }
										 else
										 {
											 mIconText = true;
											 if (mImages[ButtonImages::Image])
											 {
												 mImages[ButtonImages::Image]->SetSize(Vec2I(mButtonIconSize, mButtonIconSize));
											 }
											 AlignIconText();
										 }

										 return true;
	}

	case UIProperty::CHANGE_IMAGE_ACTIVATE:
	{
											  mChangeImageActivation = StringConverter::parseBool(val);
											  return true;
	}

	case UIProperty::PROGRESSBAR:
	{
									FB_DELETE(mProgressBar);
									mProgressBar = (HorizontalGauge*)gFBEnv->pUIManager->CreateComponent(ComponentType::HorizontalGauge);
									mProgressBar->SetHwndId(GetHwndId());
									mProgressBar->SetRender3D(mRender3D, GetRenderTargetSize());
									mProgressBar->SetParent(this);
									mProgressBar->SetSize(GetFinalSize());
									mProgressBar->SetPos(GetFinalPos());
									mProgressBar->SetMaximum(StringConverter::parseReal(val));									
									return true;
	}

	case UIProperty::GAUGE_COLOR:
	{
									
									if (mProgressBar)
										mProgressBar->SetGaugeColor(Color(val));

									return true;									
	}

	case UIProperty::GAUGE_BLINK_COLOR:
	{
										  if (mProgressBar)
											  mProgressBar->SetBlinkColor(Color(val));

										  return true;
	}

	case UIProperty::GAUGE_BLINK_SPEED:
	{
										  if (mProgressBar)
											  mProgressBar->SetProperty(prop, val);

										  return true;
	}

	case UIProperty::EDGE_COLOR:
	{
									mEdgeColor = Color(val);
								   assert(mUIObject);
								   mUIObject->GetMaterial()->SetMaterialParameters(1, mEdgeColor.GetVec4());
								   return true;
	}

	case UIProperty::EDGE_COLOR_OVER:
	{
									mEdgeColorOver = Color(val);
									return true;
	}

	case UIProperty::IMAGE_COLOR_OVERLAY:
	{
		mImageColorOverlay = StringConverter::parseColor(val);
											if (mImages[ButtonImages::Image])
											{
												return mImages[ButtonImages::Image]->SetProperty(UIProperty::IMAGE_COLOR_OVERLAY, val);
											}
											return true;
	}
	case UIProperty::NO_BUTTON:
	{
								  mNoButton = StringConverter::parseBool(val);
								  return true;
	}

	}

	return __super::SetProperty(prop, val);
}

bool Button::GetProperty(UIProperty::Enum prop, char val[], bool notDefaultOnly)
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
		auto data = StringConverter::toString(mBackColorOver);
		strcpy(val, data.c_str());		
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
		auto data = StringConverter::toString(mBackColorDown);
		strcpy(val, data.c_str());
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
		
		strcpy(val, mImageAtlas.c_str());
		return true;		
	}
	case UIProperty::REGION:
	{
		if (notDefaultOnly)
		{
			if (mRegionName.empty())
				return false;
		}

		strcpy(val, mRegionName.c_str());
		return true;
	}
	case UIProperty::REGIONS:
	{
		if (notDefaultOnly)
		{
			if (mRegionNames.empty())
				return false;
		}

		strcpy(val, mRegionNames.c_str());
		return true;
	}
	case UIProperty::TEXTURE_FILE:
	{
		if (notDefaultOnly)
		{
			if (mTextureFile.empty())
				return false;
		}

		strcpy(val, mTextureFile.c_str());
		return true;
	}
	case UIProperty::FPS:
	{
		if (notDefaultOnly)
		{
			if (mFps == UIProperty::GetDefaultValueFloat(prop))
				return false;
		}
		auto data = StringConverter::toString(mFps);
		strcpy(val, data.c_str());
		return true;
	}
	case UIProperty::HOVER_IMAGE:
	{
		if (notDefaultOnly)
		{
			if (mHorverImage.empty())
				return false;
		}

		strcpy(val, mHorverImage.c_str());
		return true;		
	}
	case UIProperty::BACKGROUND_IMAGE:
	{
		if (notDefaultOnly)
		{
			if (mBackgroundImage.empty())
				return false;
		}

		strcpy(val, mBackgroundImage.c_str());
		return true;
	}
	case UIProperty::BACKGROUND_IMAGE_DISABLED:
	{
		if (notDefaultOnly)
		{
			if (mBackgroundImageDisabled.empty())
				return false;
		}

		strcpy(val, mBackgroundImageDisabled.c_str());
		return true;		
	}

	case UIProperty::BACKGROUND_IMAGE_HOVER:
	{
		if (notDefaultOnly)
		{
			if (mBackgoundImageHover.empty())
				return false;
		}

		strcpy(val, mBackgoundImageHover.c_str());
		return true;
	}

	case UIProperty::BACKGROUND_IMAGE_NOATLAS:
	{
		if (notDefaultOnly)
		{
			if (mBackgroundImageNoAtlas.empty())
				return false;
		}

		strcpy(val, mBackgroundImageNoAtlas.c_str());
		return true;
	}

	case UIProperty::BACKGROUND_IMAGE_HOVER_NOATLAS:
	{
		if (notDefaultOnly)
		{
			if (mBackgroundImageHoverNoAtlas.empty())
				return false;
		}

		strcpy(val, mBackgroundImageHoverNoAtlas.c_str());
		return true;
	}

	case UIProperty::FRAME_IMAGE:
	{
		if (notDefaultOnly)
		{
			if (mFrameImage.empty())
				return false;
		}

		strcpy(val, mFrameImage.c_str());
		return true;
		
	}

	case UIProperty::FRAME_IMAGE_DISABLED:
	{
		if (notDefaultOnly)
		{
			if (mFrameImageDisabled.empty())
				return false;
		}

		strcpy(val, mFrameImageDisabled.c_str());
		return true;
	}

	case UIProperty::ACTIVATED_IMAGE:
	{
		if (notDefaultOnly)
		{
			if (mActivatedImage.empty())
				return false;
		}

		strcpy(val, mActivatedImage.c_str());
		return true;		
	}

	case UIProperty::DEACTIVATED_IMAGE:
	{
		if (notDefaultOnly)
		{
			if (mDeactivatedImage.empty())
				return false;
		}

		strcpy(val, mDeactivatedImage.c_str());
		return true;
	}

	case UIProperty::ACTIVATED_IMAGE_ROT:
	{
		if (notDefaultOnly)
		{
			if (mActivatedRot == UIProperty::GetDefaultValueBool(prop))
				return false;
		}
		auto data = StringConverter::toString(mActivatedRot);
		strcpy(val, data.c_str());
		return true;
	}

	case UIProperty::BUTTON_ACTIVATED:
	{
		if (notDefaultOnly)
		{
			if (mActivated == UIProperty::GetDefaultValueBool(prop))
				return false;
		}
		auto data = StringConverter::toString(mActivated);
		strcpy(val, data.c_str());
		return true;
	}

	case UIProperty::BUTTON_ICON_TEXT:
	{
		if (notDefaultOnly)
		{
			if (mButtonIconSize == UIProperty::GetDefaultValueInt(prop))
				return false;
		}
		auto data = StringConverter::toString(mButtonIconSize);
		strcpy(val, data.c_str());
		return true;
	}

	case UIProperty::CHANGE_IMAGE_ACTIVATE:
	{
		if (notDefaultOnly)
		{
			if (mChangeImageActivation == UIProperty::GetDefaultValueBool(prop))
				return false;
		}
		auto data = StringConverter::toString(mChangeImageActivation);
		strcpy(val, data.c_str());
		return true;
	}

	case UIProperty::PROGRESSBAR:
	{
		if (!mProgressBar)
			return false;

		if (notDefaultOnly)
		{
			if (!mProgressBar)
				return false;
		}
		auto data = StringConverter::toString(mProgressBar->GetMaximum());
		strcpy(val, data.c_str());
		return true;
	}

	case UIProperty::GAUGE_COLOR:
	case UIProperty::GAUGE_BLINK_COLOR:
	case UIProperty::GAUGE_BLINK_SPEED:
	{
		if (!mProgressBar)
			return false;

		return mProgressBar->GetProperty(prop, val, notDefaultOnly);
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
		auto data = StringConverter::toString(mEdgeColor);
		strcpy(val, data.c_str());
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
		auto data = StringConverter::toString(mEdgeColorOver);
		strcpy(val, data.c_str());
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
		auto data = StringConverter::toString(mImageColorOverlay);
		strcpy(val, data.c_str());
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
		auto data = StringConverter::toString(mNoButton);
		strcpy(val, data.c_str());
		return true;
	}

	}

	return __super::GetProperty(prop, val, notDefaultOnly);
}


ImageBox* Button::CreateImageBox()
{
	auto image = FB_NEW(ImageBox);
	image->SetHwndId(GetHwndId());
	image->SetRender3D(mRender3D, GetRenderTargetSize());
	image->SetVisible(true);
	image->SetParent(this);
	image->SetPos(GetFinalPos());
	image->SetSize(GetFinalSize());
	gFBEnv->pUIManager->DirtyRenderList(GetHwndId());
	return image;
}

void Button::OnStartUpdate(float elapsedTime)
{
	__super::OnStartUpdate(elapsedTime);
	if (mProgressBar)
		mProgressBar->OnStartUpdate(elapsedTime);
	for (int i = 0; i < ButtonImages::Num; ++i)
	{
		if (mImages[i])
			mImages[i]->OnStartUpdate(elapsedTime);
	}
}

void Button::StartProgress()
{
	if (mInProgress)
		return;
	mInProgress = true;
	mNoBackgroundBackup = mUIObject->GetNoDrawBackground();
	if (mProgressBar)
	{
		mProgressBar->SetPercentage(0);
		mProgressBar->SetVisible(true);
	}

	gFBEnv->pUIManager->DirtyRenderList(GetHwndId());
}

void Button::SetPercentage(float p) // progress bar
{
	if (mProgressBar)
	{
		if (!mInProgress)
		{
			mInProgress = true;
			mProgressBar->SetVisible(true);
		}
			

		mProgressBar->SetPercentage(p);
	}
}

void Button::Blink(bool blink)
{
	if (mProgressBar)
		mProgressBar->Blink(blink);
}

void Button::EndProgress()
{
	if (!mInProgress)
		return;
	mInProgress = false;
	if (mProgressBar)
		mProgressBar->SetVisible(false);
	gFBEnv->pUIManager->DirtyRenderList(GetHwndId());
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

void Button::SetTexture(ButtonImages::Enum type, ITexture* pTexture)
{
	if (!mImages[type])
	{
		mImages[type] = CreateImageBox();
	}
	mImages[type]->SetTexture(pTexture);
}

void Button::OnEnableChanged()
{
	if (mEnable)
	{
		for (int i = 0; i < ButtonImages::Num; ++i)
		{
			if (mImages[i])
				mImages[i]->SetDesaturate(false);
		}
	}
	else
	{
		for (int i = 0; i < ButtonImages::Num; ++i)
		{
			if (mImages[i])
				mImages[i]->SetDesaturate(true);
		}
	}
}

void Button::AlignIconText()
{
	const int IconTextGap = 2;
	if (mImages[ButtonImages::Image])
	{
		mImages[ButtonImages::Image]->SetAlign(ALIGNH::LEFT, ALIGNV::MIDDLE);
		int iconSize = mImages[ButtonImages::Image]->GetFinalSize().x;
		int textSize = mTextWidth;
		int buttonSize = GetFinalSize().x;
		int buttonCenter = Round(buttonSize*.5f);

		int contentSize = iconSize + textSize + IconTextGap;

		int sizeGap = buttonSize - contentSize;
		int iconStart = Round(sizeGap*.5f);
		int textStart = iconStart + iconSize + IconTextGap;
		int textCenter = textStart + Round(textSize * .5f);

		mImages[ButtonImages::Image]->SetNPos(Vec2(0.f, 0.5f));
		mImages[ButtonImages::Image]->SetInitialOffset(Vec2I(((int)iconStart), 0));
		mImages[ButtonImages::Image]->UpdateWorldPos();
		mTextGap.x = textCenter - buttonCenter;
		AlignText();		
	}	
}

void Button::SetHwndId(HWND_ID hwndId)
{
	__super::SetHwndId(hwndId);
	for (int i = 0; i < ButtonImages::Num; ++i)
	{
		if (mImages[i])
		{
			mImages[i]->SetHwndId(hwndId);
		}
	}

	if (mProgressBar)
		mProgressBar->SetHwndId(hwndId);


}
}