#include <UI/StdAfx.h>
#include <UI/NamedPortrait.h>
#include <UI/IUIManager.h>
#include <UI/ImageBox.h>
namespace fastbird
{

	NamedPortrait::NamedPortrait()
		: mImageBox(0), mStaticText(0)
	{
		mUIObject = IUIObject::CreateUIObject(false, GetRenderTargetSize());		
		mUIObject->mOwnerUI = this;
		mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
		mImageBox = AddChild(0.5f, 0.0f, 1.0f, 0.8f, ComponentType::ImageBox);
		mStaticText = AddChild(0.5f, 1.0f, 1.f, 0.2f, ComponentType::StaticText);
	}

	NamedPortrait::~NamedPortrait()
	{
	}

	void NamedPortrait::OnCreated()
	{
		assert(mImageBox);
		mImageBox->SetAlign(ALIGNH::CENTER, ALIGNV::TOP);	
		mImageBox->SetProperty(UIProperty::OFFSETY, "2");
		assert(mStaticText);
		mStaticText->SetProperty(UIProperty::TEXT_ALIGN, "center");
		mStaticText->SetProperty(UIProperty::OFFSETY, "-2");
		mStaticText->SetAlign(ALIGNH::CENTER, ALIGNV::BOTTOM);
		
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
			return mStaticText->SetProperty(UIProperty::TEXT, val);

		case UIProperty::NAMED_PORTRAIT_TEXT_COLOR:
			return mStaticText->SetProperty(UIProperty::TEXT_COLOR, val);
		}

		return __super::SetProperty(prop, val);
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
