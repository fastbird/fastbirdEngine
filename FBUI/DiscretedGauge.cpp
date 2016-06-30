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
#include "DiscretedGauge.h"
#include "UIObject.h"

namespace fb
{

	DiscretedGaugePtr DiscretedGauge::Create() {
		DiscretedGaugePtr p(new DiscretedGauge, [](DiscretedGauge* obj) { delete obj; });
		p->mSelfPtr = p;
		return p;
	}

	DiscretedGauge::DiscretedGauge()
	{
		mSteps = UIProperty::GetDefaultValueInt(UIProperty::DISCRETED_GAUGE_STEPS);
		mStrRegion = UIProperty::GetDefaultValueString(UIProperty::DISCRETED_GAUGE_REGION);
		mMarkSize = UIProperty::GetDefaultValueFloat(UIProperty::DISCRETED_GAUGE_MARK_SIZE);
	}

	DiscretedGauge::~DiscretedGauge()
	{
	}

	void DiscretedGauge::GatherVisit(std::vector<UIObject*>& v)
	{
		// skip horizontalGauge
		Container::GatherVisit(v);
	}

	void DiscretedGauge::OnParentVisibleChanged(bool visible) {
		if (visible)
			RefreshStpes();
	}

	void DiscretedGauge::RefreshStpes() {
		if (mSteps == 0)
			return;
		auto p = mPercentage / mMaximum;
		float fgap = mMaximum / mSteps;
		int count = Round(p / fgap);
		if (mChildren.size() != mSteps) {
			CreateMarks();
		}
		EnableMarkUntil(count);	
	}

	void DiscretedGauge::SetSteps(unsigned steps) {
		mSteps = steps;
		RefreshStpes();
	}

	void DiscretedGauge::SetPercentage(float p)
	{
		__super::SetPercentage(p);
		RefreshStpes();
	}

	void DiscretedGauge::SetMaximum(float m)
	{
		__super::SetMaximum(m);
		RefreshStpes();
	}

	void DiscretedGauge::SetGaugeColor(const Color& color)
	{
		__super::SetGaugeColor(color);
		RefreshStpes();
	}

	void DiscretedGauge::SetGaugeColorEmpty(const Color& color)
	{
		__super::SetGaugeColorEmpty(color);		
		RefreshStpes();
	}

	bool DiscretedGauge::SetProperty(UIProperty::Enum prop, const char* val)
	{

		switch (prop)
		{
		case UIProperty::DISCRETED_GAUGE_STEPS:
		{
			SetSteps(StringConverter::ParseUnsignedInt(val));
			return true;
		}
		case UIProperty::DISCRETED_GAUGE_REGION:
		{
			mStrRegion = val;
			return true;
		}
		case UIProperty::DISCRETED_GAUGE_MARK_SIZE:
		{
			mMarkSize = StringConverter::ParseReal(val);
			CreateMarks();
			return true;
		}
		}

		return __super::SetProperty(prop, val);
	}

	bool DiscretedGauge::GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly)
	{
		switch (prop)
		{
		case UIProperty::DISCRETED_GAUGE_STEPS:
		{
			if (notDefaultOnly)
			{
				if (mSteps == UIProperty::GetDefaultValueInt(prop))
					return false;
			}
			auto data = StringConverter::ToString(mSteps);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}
		case UIProperty::DISCRETED_GAUGE_REGION:
		{
			if (notDefaultOnly)
			{
				if (mStrRegion == UIProperty::GetDefaultValueString(prop))
					return false;
			}

			strcpy_s(val, bufsize, mStrRegion.c_str());
			return true;
		}
		case UIProperty::DISCRETED_GAUGE_MARK_SIZE:
		{
			if (notDefaultOnly)
			{
				if (mMarkSize == UIProperty::GetDefaultValueFloat(prop))
					return false;
			}

			strcpy_s(val, bufsize, StringConverter::ToString(mMarkSize).c_str());			
			return true;
		}
		}

		return __super::GetProperty(prop, val, bufsize, notDefaultOnly);
	}

	void DiscretedGauge::CreateMarks() {
		RemoveAllChildren(true);
		float x = 0;
		float y = 0.5f;
		float stepGap = 1.0f / mSteps;
		for (unsigned i = 0; i < mSteps; ++i) {
			auto image = AddChild(x, y, stepGap * mMarkSize, mMarkSize, ComponentType::ImageBox);
			image->SetProperty(UIProperty::ALIGNV, "MIDDLE");
			image->SetProperty(UIProperty::INHERIT_VISIBLE_TRUE, "false");
			image->SetProperty(UIProperty::IMAGE_LINEAR_SAMPLER, "true");
			image->SetProperty(UIProperty::REGION, mStrRegion.c_str());
			image->SetProperty(UIProperty::USE_NPOSX, "true");
			image->SetProperty(UIProperty::USE_NPOSY, "true");
			image->SetProperty(UIProperty::USE_NSIZEX, "true");
			image->SetProperty(UIProperty::USE_NSIZEY, "true");
			
			image->SetRuntimeChild(true);
			x += stepGap;
		}
		RefreshStpes();
	}
	void DiscretedGauge::EnableMarkUntil(unsigned num) {

		unsigned curIndex = 0;
		for (; curIndex < num; ++curIndex) {
			if (curIndex >= mChildren.size())
				break;
			auto child = mChildren.begin();
			std::advance(child, curIndex);
			(*child)->SetProperty(UIProperty::VISIBLE, "true");
			(*child)->SetProperty(UIProperty::IMAGE_COLOR_OVERLAY, mGaugeColor.ToString().c_str());
		}
		for (; curIndex < mChildren.size(); ++curIndex) {
			auto child = mChildren.begin();
			std::advance(child, curIndex);
			if (UsingEmptyColor()) {
				(*child)->SetProperty(UIProperty::VISIBLE, "true");
				(*child)->SetProperty(UIProperty::IMAGE_COLOR_OVERLAY, mGaugeColorEmpty.ToString().c_str());
			}
			else {
				(*child)->SetProperty(UIProperty::VISIBLE, "false");
			}
		}
	}
}