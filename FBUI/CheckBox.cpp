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
#include "CheckBox.h"
#include "ImageBox.h"
#include "StaticText.h"
#include "UIObject.h"
namespace fb
{
CheckBoxPtr CheckBox::Create(){
	CheckBoxPtr p(new CheckBox, [](CheckBox* obj){ delete obj; });
	p->mSelfPtr = p;
	return p;
}

CheckBox::CheckBox()
	: mChecked(false)	
{
	mUIObject = UIObject::Create(GetRenderTargetSize(), this);
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	mTextGap.x = 28;
}

CheckBox::~CheckBox()
{
}

void CheckBox::OnCreated()
{
	auto imageBox = std::static_pointer_cast<ImageBox>(AddChild(ComponentType::ImageBox));
	mCheckImageBox = imageBox;
	imageBox->SetRuntimeChild(true);
	imageBox->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&CheckBox::OnClickedChildren, this, std::placeholders::_1));
	imageBox->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_DOUBLE_CLICK,
		std::bind(&CheckBox::OnClickedChildren, this, std::placeholders::_1));
	imageBox->RegisterEventFunc(UIEvents::EVENT_MOUSE_HOVER,
		std::bind(&CheckBox::OnMouseHover, this, std::placeholders::_1));
	imageBox->SetAlign(ALIGNH::LEFT, ALIGNV::MIDDLE);
	imageBox->ChangeSize(Vec2I(24, 24));
	imageBox->ChangeNPos(Vec2(0.0, 0.5f));
	imageBox->SetProperty(UIProperty::IMAGE_LINEAR_SAMPLER, "true");

	RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&CheckBox::OnClicked, this, std::placeholders::_1));
	RegisterEventFunc(UIEvents::EVENT_MOUSE_HOVER,
		std::bind(&CheckBox::OnMouseHover, this, std::placeholders::_1));
	

	if (mChecked)
		imageBox->SetTextureAtlasRegion("EssentialEngineData/textures/ui.xml", "checkbox_checked");
	else
		imageBox->SetTextureAtlasRegion("EssentialEngineData/textures/ui.xml", "checkbox_unchecked");
}

void CheckBox::GatherVisit(std::vector<UIObject*>& v)
{
	v.push_back(mUIObject.get());
	__super::GatherVisit(v);
}

void CheckBox::SetCheck(bool check)
{
	if (mChecked != check)
	{
		mChecked = check;
		UpdateImage();
		
	}
}

bool CheckBox::GetCheck() const
{
	return mChecked;
}

void CheckBox::OnClicked(void* arg)
{
	mChecked = !mChecked;
	UpdateImage();
	InputManager::GetInstance().GetInputInjector()->Invalidate(InputDevice::Mouse);
}

void CheckBox::OnClickedChildren(void* arg)
{
	OnEvent(UIEvents::EVENT_MOUSE_LEFT_CLICK);
}

void CheckBox::UpdateImage()
{
	auto imageBox = mCheckImageBox.lock();
	if (imageBox)
	{
		if (mChecked)
			imageBox->SetTextureAtlasRegion("EssentialEngineData/textures/ui.xml", "checkbox_checked");
		else
			imageBox->SetTextureAtlasRegion("EssentialEngineData/textures/ui.xml", "checkbox_unchecked");
	}
}

void CheckBox::OnMouseHover(void* arg)
{
	SetCursor(WinBase::sCursorOver);
}


bool CheckBox::SetProperty(UIProperty::Enum prop, const char* val)
{
	switch (prop)
	{
	case UIProperty::CHECKBOX_CHECKED:
	{
										 bool checked = StringConverter::ParseBool(val);
										 SetCheck(checked);
										 return true;
	}
	}

	return __super::SetProperty(prop, val);
}

bool CheckBox::GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly)
{
	switch (prop)
	{
	case UIProperty::CHECKBOX_CHECKED:
	{
		if (notDefaultOnly)
		{
			if (mChecked == GetDefaultValueBool(prop))
				return false;
		}
		auto data = StringConverter::ToString(mChecked);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}

	}
	return __super::GetProperty(prop, val, bufsize, notDefaultOnly);
}

}