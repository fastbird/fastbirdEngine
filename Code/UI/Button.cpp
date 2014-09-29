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
	: mImage(0)
	, mImageOver(0)
	, mFrameImage(0)
	, mProgressBar(0)
	, mInProgress(false)
	, mNoBackgroundBackup(false)
	, mEnable(true)
{
	mUIObject = IUIObject::CreateUIObject(false);
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
	FB_DELETE(mImage);
	FB_DELETE(mImageOver);
	FB_DELETE(mFrameImage);
	FB_DELETE(mProgressBar);
}

void Button::OnPosChanged()
{
	__super::OnPosChanged();
	if (mImage)
		mImage->SetWNPos(mWNPos);
	if (mImageOver)
		mImageOver->SetWNPos(mWNPos);
	if (mFrameImage)
		mFrameImage->SetWNPos(mWNPos);
	if (mProgressBar)
		mProgressBar->SetWNPos(mWNPos);

}

void Button::SetNPosOffset(const Vec2& offset)
{
	__super::SetNPosOffset(offset);
	if (mImage)
		mImage->SetNPosOffset(offset);
	if (mImageOver)
		mImageOver->SetNPosOffset(offset);
	if (mFrameImage)
		mFrameImage->SetNPosOffset(offset);
	if (mProgressBar)
		mProgressBar->SetNPosOffset(offset);
}

void Button::OnSizeChanged()
{
	__super::OnSizeChanged();
	/*float imgRatio = 1.0f;
	const RECT& uiRect = mUIObject->GetRegion();
	float uiRatio = (uiRect.right - uiRect.left) /
		(float)(uiRect.bottom - uiRect.top);
	if (uiRatio == imgRatio)
	{*/
		Vec2 texcoords[4] = {
			Vec2(0.f, 1.f),
			Vec2(0.f, 0.f),
			Vec2(1.f, 1.f),
			Vec2(1.f, 0.f)
		};
		mUIObject->SetTexCoord(texcoords, 4);
	/*}
	else
	{
		float halfu = (uiRatio / imgRatio) * .5f;
		Vec2 texcoords[4] = {
			Vec2(0.5f - halfu, 1.f),
			Vec2(0.5f - halfu, 0.f),
			Vec2(0.5f + halfu, 1.f),
			Vec2(0.5f + halfu, 0.f)
		};
		mUIObject->SetTexCoord(texcoords, 4);
	}*/
	if (mImage)
		mImage->SetWNSize(mWNSize);
	if (mImageOver)
		mImageOver->SetWNSize(mWNSize);
	if (mFrameImage)
		mFrameImage->SetWNSize(mWNSize);
	if (mProgressBar)
		mProgressBar->SetWNSize(mWNSize);
}

void Button::GatherVisit(std::vector<IUIObject*>& v)
{
	assert(mUIObject);

	if (mProgressBar && mInProgress)
		mProgressBar->GatherVisit(v);

	v.push_back(mUIObject);

	if (mMouseIn && mImageOver)
		mImageOver->GatherVisit(v);
	else if (mImage)
		mImage->GatherVisit(v);	
	if (mFrameImage)
		mFrameImage->GatherVisit(v);

	__super::GatherVisit(v);
}

void Button::OnMouseDown(void* arg)
{
	if (!mEnable)
		return;
	mUIObject->GetMaterial()->SetDiffuseColor(mBackColorDown.GetVec4());
	mUIObject->GetMaterial()->SetEmissiveColor(1.0f, 1.0f, 0.2f, 1);
	mUIObject->SetTextColor(mEnable ? mTextColorDown : mTextColorDown*.5f);
}

void Button::OnMouseHover(void* arg)
{
	if (!mEnable)
		return;
	mUIObject->GetMaterial()->SetDiffuseColor(mBackColorOver.GetVec4());
	mUIObject->SetTextColor(mEnable ? mTextColorHover : mTextColorHover * .5f);
	SetCursor(WinBase::mCursorOver);

	mUIObject->GetMaterial()->SetEmissiveColor(0.8f, 0.8f, 0.1f, 1);
	//  1 is edge color
	mUIObject->GetMaterial()->SetMaterialParameters(1, mEdgeColorOver.GetVec4());
	if (mImageOver)
		IUIManager::GetUIManager().DirtyRenderList();

	// sometimes OnMouseHover function is called manually even the mouse
	// is in out. - DropDown
	mMouseIn = true;
}

void Button::OnMouseOut(void* arg)
{
	mUIObject->GetMaterial()->SetDiffuseColor(mBackColor.GetVec4());
	mUIObject->SetTextColor(mEnable ? mTextColor : mTextColor*.5f);
	mUIObject->GetMaterial()->SetEmissiveColor(0, 0, 0, 0);
	//  1 is edge color
	mUIObject->GetMaterial()->SetMaterialParameters(1, mEdgeColor.GetVec4());
	if (mImageOver)
		IUIManager::GetUIManager().DirtyRenderList();

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
								   mBackColor = StringConverter::parseColor(val);
								   mUIObject->GetMaterial()->SetDiffuseColor(mBackColor.GetVec4());
								   return true;
	}
	case UIProperty::BACK_COLOR_OVER:
	{
										mBackColorOver = StringConverter::parseColor(val);
										return true;
	}
	case UIProperty::BACK_COLOR_DOWN:
	{
										mBackColorDown = StringConverter::parseColor(val);
										return true;
	}

	case UIProperty::TEXTUREATLAS:
	{
									 mImageAtlas = val;
									 return true;
	}
	case UIProperty::BACKGROUND_IMAGE:
	{
										 if (!mImage)
										 {
											 mImage = FB_NEW(ImageBox);
											 mImage->SetParent(this);
											 mImage->SetWNPos(mWNPos);
											 mImage->SetWNSize(mWNSize);
											 IUIManager::GetUIManager().DirtyRenderList();
										 }

										 assert(!mImageAtlas.empty());
										 mImage->SetTextureAtlasRegion(mImageAtlas.c_str(), val);
										 return true;
	}

	case UIProperty::BACKGROUND_IMAGE_HOVER:
	{
											   if (!mImageOver)
											   {
												   mImageOver = FB_NEW(ImageBox);
												   mImageOver->SetParent(this);
												   mImageOver->SetWNPos(mWNPos);
												   mImageOver->SetWNSize(mWNSize);
												   IUIManager::GetUIManager().DirtyRenderList();
											   }

											   assert(!mImageAtlas.empty());
											   mImageOver->SetTextureAtlasRegion(mImageAtlas.c_str(), val);
											  
											   return true;
	}

	case UIProperty::BACKGROUND_IMAGE_NOATLAS:
	{
										 if (!mImage)
										 {
											 mImage = FB_NEW(ImageBox);
											 mImage->SetParent(this);
											 mImage->SetWNPos(mWNPos);
											 mImage->SetWNSize(mWNSize);
											 IUIManager::GetUIManager().DirtyRenderList();
										 }

										 mImage->SetTexture( val);
										 return true;
	}

	case UIProperty::BACKGROUND_IMAGE_HOVER_NOATLAS:
	{
											   if (!mImageOver)
											   {
												   mImageOver = FB_NEW(ImageBox);
												   mImageOver->SetParent(this);
												   mImageOver->SetWNPos(mWNPos);
												   mImageOver->SetWNSize(mWNSize);
												   IUIManager::GetUIManager().DirtyRenderList();
											   }

											   
											   mImageOver->SetTexture(val);
											   return true;
	}

	case UIProperty::FRAME_IMAGE:
	{
										 if (!mFrameImage)
										 {
											 mFrameImage = FB_NEW(ImageBox);
											 mFrameImage->SetParent(this);
											 mFrameImage->SetWNPos(mWNPos);
											 mFrameImage->SetWNSize(mWNSize);
										 }

										 assert(!mImageAtlas.empty());
										 mFrameImage->SetTextureAtlasRegion(mImageAtlas.c_str(), val);
										 IUIManager::GetUIManager().DirtyRenderList();
										 return true;
	}

	case UIProperty::PROGRESSBAR:
	{
									FB_DELETE(mProgressBar);
									mProgressBar = FB_NEW(HorizontalGauge);
									mProgressBar->SetParent(this);
									mProgressBar->SetWNSize(mWNSize);
									mProgressBar->SetWNPos(mWNPos);
									mProgressBar->SetMaximum(StringConverter::parseReal(val));
									mProgressBar->SetProperty(UIProperty::ALPHA, "true");
									SetProperty(UIProperty::ALPHA, "true");
									return true;
	}

	case UIProperty::GAUGE_COLOR:
	{
									
									if (mProgressBar)
										mProgressBar->SetGaugeColor(StringConverter::parseColor(val));

									return true;									
	}

	case UIProperty::GAUGE_BLINK_COLOR:
	{
										  if (mProgressBar)
											  mProgressBar->SetBlinkColor(StringConverter::parseColor(val));

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
								   mEdgeColor = StringConverter::parseColor(val);
								   assert(mUIObject);
								   mUIObject->GetMaterial()->SetMaterialParameters(1, mEdgeColor.GetVec4());
								   return true;
	}

	case UIProperty::EDGE_COLOR_OVER:
	{
										mEdgeColorOver = StringConverter::parseColor(val);
										return true;
	}
	

	}

	return __super::SetProperty(prop, val);
}

void Button::OnStartUpdate(float elapsedTime)
{
	if (mProgressBar)
		mProgressBar->OnStartUpdate(elapsedTime);
}

void Button::StartProgress()
{
	mInProgress = true;
	mNoBackgroundBackup = mUIObject->GetNoDrawBackground();
	if (mProgressBar)
		mProgressBar->SetPercentage(0);
	mProgressBar->SetVisible(true);

}

void Button::SetPercentage(float p) // progress bar
{
	if (mProgressBar)
		mProgressBar->SetPercentage(p);
}

void Button::Blink(bool blink)
{
	if (mProgressBar)
		mProgressBar->Blink(blink);
}

void Button::EndProgress()
{
	mInProgress = false;
	mProgressBar->SetVisible(false);
}

void Button::SetEnable(bool enable)
{
	__super::SetEnable(enable);
	mEnable = enable;
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

void Button::SetBackgroundTexture(ITexture* pTexture)
{
	if (!mImage)
	{
		mImage = FB_NEW(ImageBox);
		mImage->SetParent(this);
		mImage->SetWNPos(mWNPos);
		mImage->SetWNSize(mWNSize);
		IUIManager::GetUIManager().DirtyRenderList();
	}

	mImage->SetTexture(pTexture);
}
}