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
#include "RadioBox.h"
#include "ImageBox.h"
#include "StaticText.h"
#include "UIObject.h"
namespace fb
{

RadioBoxPtr RadioBox::Create(){
	RadioBoxPtr p(new RadioBox, [](RadioBox* obj){ delete obj; });
	p->mSelfPtr = p;
	return p;
}

RadioBox::RadioBox()
	: mChecked(false)
	, mGroupID(-1)	
{	
}

RadioBox::~RadioBox()
{
}

void RadioBox::OnCreated(){
	auto imageBox = std::static_pointer_cast<ImageBox>(
		AddChild(Vec2I(0, 0), Vec2I(mSize.y, mSize.y), ComponentType::ImageBox));
	mRadioImageBox = imageBox;
	imageBox->SetRuntimeChild(true);	
	imageBox->SetTextureAtlasRegion("EssentialEngineData/textures/ui.xml", "radiobox_unchecked");
	imageBox->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&RadioBox::OnChildrenClicked, this, std::placeholders::_1));
	imageBox->RegisterEventFunc(UIEvents::EVENT_MOUSE_HOVER,
		std::bind(&RadioBox::OnMouseHover, this, std::placeholders::_1));
	imageBox->SetUseAbsPos(false);
	imageBox->SetUseAbsSize(false);
	imageBox->SetProperty(UIProperty::KEEP_UI_RATIO, "false");
	imageBox->SetProperty(UIProperty::IMAGE_LINEAR_SAMPLER, "true");
	UpdateImage();

	auto staticText = std::static_pointer_cast<StaticText>(
		AddChild(0.06f, 0.f, 0.79f, 1.0f, ComponentType::StaticText));
	mStaticText = staticText;
	staticText->ChangePosX(24);
	staticText->SetRuntimeChild(true);
	staticText->SetProperty(UIProperty::BACK_COLOR, "0.1, 0.1, 0.1, 0.5");
	staticText->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&RadioBox::OnChildrenClicked, this, std::placeholders::_1));
	staticText->RegisterEventFunc(UIEvents::EVENT_MOUSE_HOVER,
		std::bind(&RadioBox::OnMouseHover, this, std::placeholders::_1));
	staticText->SetUseAbsPos(false);
	staticText->SetUseAbsSize(false);
	if (!mTempString.empty()){
		staticText->SetText(mTempString.c_str());
		ClearWithSwap(mTempString);		
	}

	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());

	RegisterEventFunc(UIEvents::EVENT_MOUSE_HOVER,
		std::bind(&RadioBox::OnMouseHover, this, std::placeholders::_1));
	RegisterEventFunc(UIEvents::EVENT_MOUSE_HOVER,
		std::bind(&RadioBox::OnMouseHover, this, std::placeholders::_1));
}

void RadioBox::GatherVisit(std::vector<UIObject*>& v)
{
	__super::GatherVisit(v);
}

bool RadioBox::SetProperty(UIProperty::Enum prop, const char* val)
{
	switch (prop)
	{
	case UIProperty::RADIO_GROUP:
	{
		SetGroupID(StringConverter::ParseUnsignedInt(val));
		return true;
	}
	case UIProperty::RADIO_CHECK:
	{
		SetCheck(StringConverter::ParseBool(val));
		return true;
	}
	}
	return __super::SetProperty(prop, val);
}

bool RadioBox::GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly)
{
	switch (prop)
	{
	case UIProperty::RADIO_GROUP:
	{
		if (notDefaultOnly)
		{
			if (mGroupID == UIProperty::GetDefaultValueInt(prop))
				return false;
		}
		auto data = StringConverter::ToString(mGroupID);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}
	case UIProperty::RADIO_CHECK:
	{
		if (notDefaultOnly)
		{
			if (mChecked == UIProperty::GetDefaultValueBool(prop))
				return false;
		}
		auto data = StringConverter::ToString(mChecked);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}
	}
	return __super::GetProperty(prop, val, bufsize, notDefaultOnly);
}

void RadioBox::OnSizeChanged(){
	__super::OnSizeChanged();
	auto image = mRadioImageBox.lock();
	if (image)
		image->ChangeSize(Vec2I(mSize.y, mSize.y));
}

void RadioBox::SetText(const wchar_t* szText)
{
	auto staticText = mStaticText.lock();
	if (staticText)
		staticText->SetText(szText);
	else
		mTempString = szText;
}

void RadioBox::SetCheck(bool check)
{
	if (mChecked != check)
	{
		mChecked = check;
		UpdateImage();
	}
}

bool RadioBox::GetCheck() const
{
	return mChecked;
}

void RadioBox::OnChildrenClicked(void* arg)
{
	SetCheck(!mChecked);
	auto parent = GetParent();
	if (parent)
		parent->OnClickRadio(this);

	OnEvent(UIEvents::EVENT_MOUSE_LEFT_CLICK);
}

void RadioBox::OnClicked(void* arg)
{
	SetCheck(!mChecked);
	auto parent = GetParent();
	if (parent)
		parent->OnClickRadio(this);
}

void RadioBox::UpdateImage()
{
	auto image = mRadioImageBox.lock();
	if (!image)
		return;
	if (mChecked)
		image->SetTextureAtlasRegion("EssentialEngineData/textures/ui.xml", "radiobox_checked");
	else
		image->SetTextureAtlasRegion("EssentialEngineData/textures/ui.xml", "radiobox_unchecked");
}

void RadioBox::OnMouseHover(void* arg)
{
	if (!mEnable)
		return;
	SetCursor(WinBase::sCursorOver);
}

}