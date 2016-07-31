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
#include "ColorRampComp.h"
#include "Button.h"
#include "StaticText.h"
#include "UIObject.h"

using namespace fb;

ColorRampCompPtr ColorRampComp::Create(){
	ColorRampCompPtr p(new ColorRampComp, [](ColorRampComp* obj){ delete obj; });
	p->mSelfPtr = p;
	return p;
}

ColorRampComp::ColorRampComp()
{
	mUIObject = UIObject::Create(GetRenderTargetSize(), this);
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
}

ColorRampComp::~ColorRampComp()
{

}

void ColorRampComp::GatherVisit(std::vector<UIObject*>& v)
{
	if (!mVisibility.IsVisible())
		return;
	v.push_back(mUIObject.get());
	__super::GatherVisit(v);
}

bool ColorRampComp::SetProperty(UIProperty::Enum prop, const char* val)
{
	switch (prop)
	{
	case UIProperty::COLOR_RAMP_VALUES:
	{
		SetColorRampValues(val);
		return true;
	}
	}

	return __super::SetProperty(prop, val);
}

bool ColorRampComp::GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly)
{
	switch (prop)
	{
	case UIProperty::COLOR_RAMP_VALUES:
	{
		GetColorRampValues(val, bufsize, 6);
		return true;
	}
	}

	return __super::GetProperty(prop, val, bufsize, notDefaultOnly);
}

void ColorRampComp::SetColorRampValues(const char* values)
{
	if (!values || strlen(values) == 0)
	{
		return;
	}
	auto strs = Split(values, ", ");
	if (strs.size() < 2)
	{
		Logger::Log(FB_ERROR_LOG_ARG,
			"void ColorRampComp::SetColorRampValues(const char* values) : "
			"'values' should have at least two values.");
		assert(0);
		return;
	}
	
	std::vector<float> ratios;
	for (unsigned i = 0; i < strs.size(); i++)
	{
		ratios.push_back(StringConverter::ParseReal(strs[i]));
	}
	SetColorRampValuesFloats(ratios);
}

void ColorRampComp::GetColorRampValues(char val[], unsigned bufsize, int precision)
{
	std::vector<float> values;
	GetColorRampValuesFloats(values);

	std::string strValues;
	int i = 0;
	for (auto val : values)
	{
		strValues += StringConverter::ToString(val, precision);
		if (i != values.size() - 1)
		{
			strValues += ",";
		}
	}
	strcpy_s(val, bufsize, strValues.c_str());
}

void ColorRampComp::GetColorRampValuesFloats(std::vector<float>& values)
{
	values.clear();
	float prevXnpos = -1;
	for (unsigned i = 0; i < mBars.size(); i++)
	{
		auto bar = mBars[i].lock();
		assert(bar);
		if (bar){
			auto& npos = bar->GetNPos();
			float xnpos = npos.x;
			if (prevXnpos != -1)
				xnpos -= prevXnpos;
			values.push_back(xnpos);
			prevXnpos = npos.x;
		}
	}
	values.push_back(1.0f - prevXnpos);
}

void ColorRampComp::SetColorRampValuesFloats(const std::vector<float>& values)
{
	RemoveAllChildren();
	mBars.clear();
	mTexts.clear();

	float prevXPos = -1;
	for (unsigned i = 0; i < values.size() - 1; i++)
	{
		float ratio = values[i];
		float xPos = ratio;
		if (prevXPos != -1)
		{
			xPos += prevXPos;
		}

		auto bar = std::static_pointer_cast<Button>(AddChild(xPos, 0.5f, 0.05f, 1.0f, ComponentType::Button));
		bar->SetRuntimeChild(true);
		mBars.push_back(bar);
		bar->ModifySize(Vec2I(0, -10));
		bar->SetAlign(ALIGNH::CENTER, ALIGNV::MIDDLE);
		bar->SetProperty(UIProperty::TEXTUREATLAS, "EssentialEngineData/textures/ui.xml");
		bar->SetProperty(UIProperty::REGION, "BarComp");
		bar->SetProperty(UIProperty::HOVER_IMAGE, "BarCompHover");
		bar->SetProperty(UIProperty::DRAGABLE, "1, 0");

		float centerXPos;
		if (prevXPos == -1)
		{
			centerXPos = ratio * .5f;
		}
		else
		{
			centerXPos = prevXPos + ratio * .5f;
		}
		auto text = std::static_pointer_cast<StaticText>(AddChild(centerXPos, 0.5f, 1, 1, ComponentType::StaticText));
		text->SetRuntimeChild(true);
		text->SetSize(Vec2I(50, 20));
		text->SetProperty(UIProperty::TEXT_ALIGN, "center");
		text->SetAlign(ALIGNH::CENTER, ALIGNV::MIDDLE);
		char buf[255];
		sprintf_s(buf, "%d", (int)(ratio * 100));
		text->SetText(AnsiToWide(buf));
		mTexts.push_back(text);

		prevXPos = xPos;
	}
	float ratio = values.back();
	float centerXPos = prevXPos + ratio * .5f;
	auto text = std::static_pointer_cast<StaticText>(AddChild(centerXPos, 0.5f, 1, 1, ComponentType::StaticText));
	text->SetRuntimeChild(true);
	text->SetSize(Vec2I(50, 20));
	text->SetProperty(UIProperty::TEXT_ALIGN, "center");
	text->SetAlign(ALIGNH::CENTER, ALIGNV::MIDDLE);
	char buf[255];
	sprintf_s(buf, "%d", (int)(ratio * 100));
	text->SetText(AnsiToWide(buf));
	mTexts.push_back(text);
}

void ColorRampComp::OnChildHasDragged()
{
	if (mBars.empty())
		return;

	assert(mBars.size() == mTexts.size()-1);
	for (unsigned i = 0; i < mBars.size(); ++i)
	{
		auto bar = mBars[i].lock();
		assert(bar);
		if (bar){
			auto npos = bar->GetNPos();
			float ratio = npos.x;
			if (i != 0)
			{
				ratio -= bar->GetNPos().x;
			}
			char buf[255];
			sprintf_s(buf, "%d", (int)(ratio * 100));
			auto text = mTexts[i].lock();
			assert(text);
			if (text){
				text->SetText(AnsiToWide(buf));
				if (i != 0)
				{
					auto prevBar = mBars[i - 1].lock();
					assert(prevBar);
					text->SetNPos(Vec2(prevBar->GetNPos().x + ratio*.5f, 0.5f));
				}
				else
				{
					text->SetNPos(Vec2(ratio*.5f, 0.5f));
				}
			}
		}
	}

	float xPos = (mBars.back()).lock()->GetNPos().x;
	char buf[255];
	float ratio = 1.0f - xPos;
	sprintf_s(buf, "%d", (int)(ratio * 100));
	mTexts.back().lock()->SetText(AnsiToWide(buf));
	mTexts.back().lock()->SetNPos(Vec2(xPos + ratio*.5f, 0.5f));

	// issue the event
	OnEvent(UIEvents::EVENT_COLORRAMP_DRAGGED);
}
