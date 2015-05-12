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
		, mMatchHeight(false)
	{
		mUIObject = gFBEnv->pEngine->CreateUIObject(false, GetRenderTargetSize());
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
		if (!mVisibility.IsVisible())
			return;

		if (mImage)
			mImage->GatherVisit(v);

		v.push_back(mUIObject);

		__super::GatherVisit(v);
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
		if (mMatchHeight)
		{
			unsigned height = GetTextBoxHeight();
			SetSizeY(height);
		}
	}

	void TextBox::CalcTextWidth()
	{
		// analyze the text length
		IFont* pFont = gFBEnv->pRenderer->GetFont();
		unsigned width = mSize.x;
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
		case UIProperty::BACKGROUND_IMAGE_NOATLAS:
		{
			if (!mImage)
			{
				mImage = FB_NEW(ImageBox);
				mImage->SetHwndId(GetHwndId());
				mImage->SetRender3D(mRender3D, GetRenderTargetSize());
				mImage->SetParent(this);
				mImage->SetWNPos(mWNPos);
				mImage->SetWNSize(mWNSize);
			}
			gFBEnv->pUIManager->DirtyRenderList(GetHwndId());

			mImage->SetTexture(val);
			return true;
		}

		case UIProperty::KEEP_IMAGE_RATIO:
		{
											 if (!mImage)
											 {
												 mImage = FB_NEW(ImageBox);
												 mImage->SetHwndId(GetHwndId());
												 mImage->SetRender3D(mRender3D, GetRenderTargetSize());
												 mImage->SetParent(this);
												 mImage->SetWNPos(mWNPos);
												 mImage->SetWNSize(mWNSize);
											 }
											 gFBEnv->pUIManager->DirtyRenderList(GetHwndId());
											 mImage->SetKeepImageRatio(StringConverter::parseBool(val, true));
											 
											 return true;
		}

		case UIProperty::TEXTBOX_MATCH_HEIGHT:
		{
												 mMatchHeight = StringConverter::parseBool(val);
												 if (mMatchHeight)
												 {
													 unsigned height = GetTextBoxHeight();
													 SetSizeY(height);
												 }
												 return true;
		}

		}
		
		
		return __super::SetProperty(prop, val);
	}


	unsigned TextBox::GetTextBoxHeight() const
	{
		IFont* pFont = gFBEnv->pRenderer->GetFont();
		pFont->SetHeight(mTextSize);
		float height = pFont->GetBaseHeight();
		pFont->SetBackToOrigHeight();
		return (unsigned)(height * mNumTextLines) + 16;
	}

	void TextBox::SetHwndId(HWND_ID hwndId)
	{
		__super::SetHwndId(hwndId);
		if (mImage)
		{
			mImage->SetHwndId(hwndId);
		}
	}

}