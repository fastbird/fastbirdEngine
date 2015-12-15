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
#include "TextBox.h"
#include "KeyboardCursor.h"
#include "UIManager.h"
#include "ImageBox.h"
#include "UIObject.h"


namespace fb
{

	TextBoxPtr TextBox::Create(){
		TextBoxPtr p(new TextBox, [](TextBox* obj){ delete obj; });
		p->mSelfPtr = p;
		return p;
	}

	const float TextBox::LEFT_GAP = 0.001f;

	TextBox::TextBox()
		: mCursorPos(0)
		, mPasswd(false)		
		, mMatchHeight(false)
	{
		mUIObject = UIObject::Create(GetRenderTargetSize());
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
	}

	void TextBox::GatherVisit(std::vector<UIObject*>& v)
	{
		if (!mVisibility.IsVisible())
			return;

		if (mImage)
			mImage->GatherVisit(v);

		v.push_back(mUIObject.get());

		__super::GatherVisit(v);
	}

	void TextBox::SetWNScollingOffset(const Vec2& offset)
	{
		__super::SetWNScollingOffset(offset);
		if (mImage)
			mImage->SetWNScollingOffset(offset);
	}

	void TextBox::OnPosChanged(bool anim)
	{
		__super::OnPosChanged(anim);
		if (mImage)
			mImage->SetPos(GetFinalPos());
	}

	void TextBox::OnSizeChanged()
	{
		__super::OnSizeChanged();
		if (mImage)
			mImage->SetSize(GetFinalSize());

	}

	void TextBox::SetText(const wchar_t* szText)
	{
		__super::SetText(szText);
		if (mMatchHeight)
		{
			unsigned height = GetTextBoxHeight();
			ChangeSizeY(height);
		}

		TriggerRedraw();
	}

	void TextBox::CalcTextWidth()
	{
		// analyze the text length
		FontPtr pFont = Renderer::GetInstance().GetFontWithHeight(mTextSize);
		unsigned width = mSize.x;		
		float textWidth;
		mMultiLineText = pFont->InsertLineFeed((const char*)mTextw.c_str(), mTextw.size() * 2, width, &textWidth, &mNumTextLines);
		mTextWidth = (unsigned)Round(textWidth);		
		mUIObject->SetText(mMultiLineText.c_str());
	}

	bool TextBox::SetProperty(UIProperty::Enum prop, const char* val)
	{		
		switch (prop)
		{
		case UIProperty::BACKGROUND_IMAGE_NOATLAS:
		{
			mStrBackImage = val;
			if (!mImage)
			{
				mImage = ImageBox::Create();
				mImage->SetHwndId(GetHwndId());
				mImage->SetRender3D(mRender3D, GetRenderTargetSize());
				mImage->SetManualParent(mSelfPtr.lock());
				mImage->ChangePos(GetFinalPos());
				mImage->ChangeSize(GetFinalSize());
			}
			UIManager::GetInstance().DirtyRenderList(GetHwndId());

			mImage->SetTexture(val);
			return true;
		}

		case UIProperty::IMAGE_DISPLAY:
		{
			mStrImageDisplay = val;
			if (!mImage)
			{
				mImage = ImageBox::Create();
				mImage->SetHwndId(GetHwndId());
				mImage->SetRender3D(mRender3D, GetRenderTargetSize());
				mImage->SetManualParent(mSelfPtr.lock());
				mImage->ChangePos(GetFinalPos());
				mImage->ChangeSize(GetFinalSize());
			}
			UIManager::GetInstance().DirtyRenderList(GetHwndId());
			mImage->SetProperty(UIProperty::IMAGE_DISPLAY, val);
			return true;
		}

		case UIProperty::TEXTBOX_MATCH_HEIGHT:
		{
												 mMatchHeight = StringConverter::ParseBool(val);
												 if (mMatchHeight)
												 {
													 unsigned height = GetTextBoxHeight();
													 ChangeSizeY(height);
												 }
												 return true;
		}

		}
		
		
		return __super::SetProperty(prop, val);
	}

	bool TextBox::GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly)
	{
		switch (prop)
		{
		case UIProperty::BACKGROUND_IMAGE_NOATLAS:
		{
			if (notDefaultOnly)
			{
				if (mStrBackImage.empty())
					return false;
			}

			strcpy_s(val, bufsize, mStrBackImage.c_str());
			return true;
		}

		case UIProperty::IMAGE_DISPLAY:
		{
			if (notDefaultOnly)
			{
				if (mStrImageDisplay.empty() || 
					ImageDisplay::ConvertToEnum(mStrImageDisplay.c_str()) == UIProperty::GetDefaultValueInt(prop))
					return false;
			}

			strcpy_s(val, bufsize, mStrImageDisplay.c_str());
		}

		case UIProperty::TEXTBOX_MATCH_HEIGHT:
		{
			if (notDefaultOnly)
			{
				if (mMatchHeight == UIProperty::GetDefaultValueBool(prop))
					return false;
			}

			auto data = StringConverter::ToString(mMatchHeight);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}

		}


		return __super::GetProperty(prop, val, bufsize, notDefaultOnly);
	}


	unsigned TextBox::GetTextBoxHeight() const
	{
		/*FontPtr pFont = Renderer::GetInstance().GetFontWithHeight(mTextSize);
		float height = pFont->GetHeight();*/

		return Round(mTextSize * mNumTextLines);
	}

	void TextBox::SetHwndId(HWindowId hwndId)
	{
		__super::SetHwndId(hwndId);
		if (mImage)
		{
			mImage->SetHwndId(hwndId);
		}
	}

}