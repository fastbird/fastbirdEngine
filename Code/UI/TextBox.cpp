#include <UI/StdAfx.h>
#include <UI/TextBox.h>
#include <UI/KeyboardCursor.h>
#include <UI/IUIManager.h>
#include <UI/ImageBox.h>
#include <Engine/GlobalEnv.h>

namespace fastbird
{

	const float TextBox::LEFT_GAP = 0.001f;

	TextBox::TextBox()
		: mCursorPos(0)
		, mPasswd(false)
		, mImage(0)
	{
		mUIObject = IUIObject::CreateUIObject(false, GetRenderTargetSize());
		mUIObject->mOwnerUI = this;
		mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
		mUIObject->SetTextColor(mTextColor);
		mUIObject->SetNoDrawBackground(true);
		mUIObject->SetMultiline(true);
		SetProperty(UIProperty::FIXED_TEXT_SIZE, "true");
		SetProperty(UIProperty::MATCH_SIZE, "false");
	}

	TextBox::~TextBox()
	{
		FB_DELETE(mImage);
	}

	void TextBox::GatherVisit(std::vector<IUIObject*>& v)
	{
		if (!mVisible)
			return;

		if (mImage)
			mImage->GatherVisit(v);

		v.push_back(mUIObject);
	}

	void TextBox::SetNPosOffset(const Vec2& offset)
	{
		__super::SetNPosOffset(offset);
		if (mImage)
			mImage->SetNPosOffset(offset);
	}

	void TextBox::OnPosChanged()
	{
		__super::OnPosChanged();
		if (mImage)
			mImage->SetWNPos(mWNPos);
	}

	void TextBox::OnSizeChanged()
	{
		__super::OnSizeChanged();
		if (mImage)
			mImage->SetWNSize(mWNSize);

	}

	void TextBox::SetText(const wchar_t* szText)
	{
		__super::SetText(szText);

	}

	void TextBox::CalcTextWidth()
	{
		// analyze the text length
		IFont* pFont = gEnv->pRenderer->GetFont();
		const auto& region = GetRegion();
		unsigned width = region.right - region.left;

		pFont->SetHeight(mTextSize);
		float textWidth;
		mTextw = pFont->InsertLineFeed((const char*)mTextw.c_str(), mTextw.size() * 2, width, &textWidth, &mNumTextLines);
		mTextWidth = (unsigned)Round(textWidth);
		pFont->SetBackToOrigHeight();
		mUIObject->SetText(mTextw.c_str());
	}

	bool TextBox::SetProperty(UIProperty::Enum prop, const char* val)
	{		
		switch (prop)
		{
		case UIProperty::TEXT_COLOR:
				{
					mUIObject->SetTextColor(mTextColor);
					return true;
				}
		case UIProperty::BACKGROUND_IMAGE_NOATLAS:
		{
			if (!mImage)
			{
				mImage = FB_NEW(ImageBox);
				mImage->SetRender3D(mRender3D, GetRenderTargetSize());
				mImage->SetParent(this);
				mImage->SetWNPos(mWNPos);
				mImage->SetWNSize(mWNSize);
			}
			IUIManager::GetUIManager().DirtyRenderList();

			mImage->SetTexture(val);
			return true;
		}

		case UIProperty::KEEP_IMAGE_RATIO:
		{
											 if (!mImage)
											 {
												 mImage = FB_NEW(ImageBox);
												 mImage->SetRender3D(mRender3D, GetRenderTargetSize());
												 mImage->SetParent(this);
												 mImage->SetWNPos(mWNPos);
												 mImage->SetWNSize(mWNSize);
											 }
											 IUIManager::GetUIManager().DirtyRenderList();
											 mImage->SetKeepImageRatio(StringConverter::parseBool(val, true));
											 
											 return true;
		}

		}
		
		
		return __super::SetProperty(prop, val);
	}
}