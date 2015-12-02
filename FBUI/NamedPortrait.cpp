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
#include "NamedPortrait.h"
#include "UIManager.h"
#include "ImageBox.h"
#include "UIObject.h"
#include "TextBox.h"

namespace fb
{

	NamedPortraitPtr NamedPortrait::Create(){
		NamedPortraitPtr p(new NamedPortrait, [](NamedPortrait* obj){ delete obj; });
		p->mSelfPtr = p;
		return p;
	}

	NamedPortrait::NamedPortrait()
	{
		mUIObject = UIObject::Create(GetRenderTargetSize());		
		mUIObject->mOwnerUI = this;
		mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
		auto imageBox = std::static_pointer_cast<ImageBox>(AddChild(0.5f, 0.0f, 1.0f, 0.8f, ComponentType::ImageBox));
		mImageBox = imageBox;		
		imageBox->SetRuntimeChild(true);
		auto textBox = std::static_pointer_cast<TextBox>(AddChild(0.5f, 1.0f, 1.f, 0.2f, ComponentType::TextBox));
		mTextBox = textBox;		
		textBox->SetRuntimeChild(true);
		textBox->SetProperty(UIProperty::TEXT_SIZE, "22");
	}

	NamedPortrait::~NamedPortrait()
	{
	}

	void NamedPortrait::OnCreated()
	{
		auto imageBox = mImageBox.lock();
		assert(imageBox);
		imageBox->SetAlign(ALIGNH::CENTER, ALIGNV::TOP);
		imageBox->SetProperty(UIProperty::OFFSETY, "2");
		auto textBox = mTextBox.lock();
		assert(textBox);
		textBox->SetPosY(imageBox->GetSize().y + 2);
		textBox->SetProperty(UIProperty::TEXT_GAP, "2");
		textBox->SetProperty(UIProperty::NSIZEY, "fill");
		textBox->SetProperty(UIProperty::TEXT_ALIGN, "center");
		textBox->SetProperty(UIProperty::TEXT_VALIGN, "top");
		textBox->SetAlign(ALIGNH::CENTER, ALIGNV::TOP);
	}

	void NamedPortrait::GatherVisit(std::vector<UIObject*>& v)
	{
		v.push_back(mUIObject.get());
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
			return mImageBox.lock()->SetProperty(prop, val);	
		case UIProperty::NAMED_PORTRAIT_IMAGE_SIZE:
			return mImageBox.lock()->SetProperty(UIProperty::SIZE, val);
		case UIProperty::NAMED_PORTRAIT_TEXT:
			return mTextBox.lock()->SetProperty(UIProperty::TEXT, val);
		case UIProperty::NAMED_PORTRAIT_TEXT_COLOR:
			return mTextBox.lock()->SetProperty(UIProperty::TEXT_COLOR, val);
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
			return mImageBox.lock()->GetProperty(prop, val, bufsize, notDefaultOnly);
		case UIProperty::NAMED_PORTRAIT_IMAGE_SIZE:
			return mImageBox.lock()->GetProperty(UIProperty::SIZE, val, bufsize, notDefaultOnly);
		case UIProperty::NAMED_PORTRAIT_TEXT:
			return mTextBox.lock()->GetProperty(UIProperty::TEXT, val, bufsize, notDefaultOnly);
		case UIProperty::NAMED_PORTRAIT_TEXT_COLOR:
			return mTextBox.lock()->GetProperty(UIProperty::TEXT_COLOR, val, bufsize, notDefaultOnly);
		}

		return __super::GetProperty(prop, val, bufsize, notDefaultOnly);
	}


	void NamedPortrait::SetTexture(TexturePtr texture)
	{
		mImageBox.lock()->SetTexture(texture);		
	}
}
