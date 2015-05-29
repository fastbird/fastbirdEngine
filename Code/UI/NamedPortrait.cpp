#include <UI/StdAfx.h>
#include <UI/NamedPortrait.h>
#include <UI/IUIManager.h>
#include <UI/ImageBox.h>
namespace fastbird
{

	NamedPortrait::NamedPortrait()
	{
		mUIObject = gFBEnv->pEngine->CreateUIObject(false, GetRenderTargetSize());		
		mUIObject->mOwnerUI = this;
		mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
		mImageBox = AddChild(0.5f, 0.0f, 1.0f, 0.8f, ComponentType::ImageBox);
		mImageBox->SetRuntimeChild(true);
		mTextBox = AddChild(0.5f, 1.0f, 1.f, 0.2f, ComponentType::TextBox);
		mTextBox->SetRuntimeChild(true);
		mTextBox->SetProperty(UIProperty::TEXT_SIZE, "22");
	}

	NamedPortrait::~NamedPortrait()
	{
	}

	void NamedPortrait::OnCreated()
	{
		assert(mImageBox);
		mImageBox->SetAlign(ALIGNH::CENTER, ALIGNV::TOP);	
		mImageBox->SetProperty(UIProperty::OFFSETY, "2");
		assert(mTextBox);		
		mTextBox->SetPosY(mImageBox->GetSize().y+2);
		mTextBox->SetProperty(UIProperty::TEXT_GAP, "2");
		mTextBox->SetProperty(UIProperty::NSIZEY, "fill");
		mTextBox->SetProperty(UIProperty::TEXT_ALIGN, "center");
		mTextBox->SetProperty(UIProperty::TEXT_VALIGN, "top");
		mTextBox->SetAlign(ALIGNH::CENTER, ALIGNV::TOP);
		
	}

	void NamedPortrait::GatherVisit(std::vector<IUIObject*>& v)
	{
		v.push_back(mUIObject);
		__super::GatherVisit(v);
	}

	bool NamedPortrait::SetProperty(UIProperty::Enum prop, const char* val)
	{
		switch (prop)
		{
		case UIProperty::TEXTUREATLAS:
		case UIProperty::REGION:
		case UIProperty::REGIONS:
		case UIProperty::FPS:
		case UIProperty::TEXTURE_FILE:
		case UIProperty::KEEP_IMAGE_RATIO:
		case UIProperty::FRAME_IMAGE:
		case UIProperty::IMAGE_COLOR_OVERLAY:
		case UIProperty::IMAGE_FIXED_SIZE:
			if (mImageBox)
			{
				return mImageBox->SetProperty(prop, val);
			}
			break;

		case UIProperty::NAMED_PORTRAIT_IMAGE_SIZE:
			return mImageBox->SetProperty(UIProperty::SIZE, val);

		case UIProperty::NAMED_PORTRAIT_TEXT:
			return mTextBox->SetProperty(UIProperty::TEXT, val);

		case UIProperty::NAMED_PORTRAIT_TEXT_COLOR:
			return mTextBox->SetProperty(UIProperty::TEXT_COLOR, val);
		}

		return __super::SetProperty(prop, val);
	}

	bool NamedPortrait::GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly)
	{
		switch (prop)
		{
		case UIProperty::TEXTUREATLAS:
		case UIProperty::REGION:
		case UIProperty::REGIONS:
		case UIProperty::FPS:
		case UIProperty::TEXTURE_FILE:
		case UIProperty::KEEP_IMAGE_RATIO:
		case UIProperty::FRAME_IMAGE:
		case UIProperty::IMAGE_COLOR_OVERLAY:
		case UIProperty::IMAGE_FIXED_SIZE:
			if (mImageBox)
			{
				return mImageBox->GetProperty(prop, val, bufsize, notDefaultOnly);
			}
			break;

		case UIProperty::NAMED_PORTRAIT_IMAGE_SIZE:
			return mImageBox->GetProperty(UIProperty::SIZE, val, bufsize, notDefaultOnly);

		case UIProperty::NAMED_PORTRAIT_TEXT:
			return mTextBox->GetProperty(UIProperty::TEXT, val, bufsize, notDefaultOnly);

		case UIProperty::NAMED_PORTRAIT_TEXT_COLOR:
			return mTextBox->GetProperty(UIProperty::TEXT_COLOR, val, bufsize, notDefaultOnly);
		}

		return __super::GetProperty(prop, val, bufsize, notDefaultOnly);
	}


	void NamedPortrait::SetTexture(ITexture* texture)
	{
		if (mImageBox)
		{
			assert(mImageBox->GetType() == ComponentType::ImageBox);
			ImageBox* imgBox = (ImageBox*)mImageBox;
			imgBox->SetTexture(texture);
		}
	}


}
