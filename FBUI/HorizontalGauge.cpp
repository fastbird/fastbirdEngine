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
#include "HorizontalGauge.h"
#include "UIObject.h"

namespace fb
{

	HorizontalGaugePtr HorizontalGauge::Create(){
		HorizontalGaugePtr p(new HorizontalGauge, [](HorizontalGauge* obj){ delete obj; });
		p->mSelfPtr = p;
		return p;
	}

	HorizontalGauge::HorizontalGauge()
		:mPercentage(0), mMaximum(1.f)
		, mBlink(false)
		, mBlinkSpeed(3.f)
		, mGaugeColorEmptySet(false)
		, mBlinkTime(0)
	{
		mUIObject = UIObject::Create(GetRenderTargetSize());
		mUIObject->SetMaterial("EssentialEngineData/materials/UIHorizontalGauge.material");
		mUIObject->mOwnerUI = this;
		mUIObject->mTypeString = ComponentType::ConvertToString(GetType());

		auto mat = mUIObject->GetMaterial();
		mGaugeColor = Color(1, 1, 1, 1);
		mBlinkColor = Color(1, 1, 0, 1);
		mGaugeBorderColor = Color(0.5f, 0.5f, 0.5f, 0.5f);
		mat->SetMaterialParameter(1, mGaugeColor.GetVec4());
		mat->SetMaterialParameter(2, mBlinkColor.GetVec4());
		mat->SetAmbientColor(mGaugeBorderColor.GetVec4());

		// x is lerp.
		mat->SetMaterialParameter(3, Vec4(0, 0, 0, 0));
		Vec2 texcoords[4] = {
			Vec2(0.f, 1.f),
			Vec2(0.f, 0.f),
			Vec2(1.f, 1.f),
			Vec2(1.f, 0.f)
		};
		mUIObject->SetTexCoord(texcoords, 4);
		mat->SetMaterialParameter(4, Vec4(mPercentage, mMaximum, 0, 0));
	}

	HorizontalGauge::~HorizontalGauge()
	{
	}

	void HorizontalGauge::GatherVisit(std::vector<UIObject*>& v)
	{
		v.push_back(mUIObject.get());
		__super::GatherVisit(v);
	}
	void HorizontalGauge::OnStartUpdate(float elapsedTime)
	{
		__super::OnStartUpdate(elapsedTime);
		if (mBlink && mBlinkTime > 0)
		{
			auto mat = mUIObject->GetMaterial();
			mat->SetMaterialParameter(3, Vec4(sin(gpTimer->GetTime()*mBlinkSpeed)*.5f + .5f, 0, 0, 0));
			mBlinkTime -= elapsedTime;
			if (mBlinkTime <= 0){
				Blink(false);
				
			}
		}
	}

	void HorizontalGauge::SetPercentage(float p)
	{
		mPercentage = p;
		auto mat = mUIObject->GetMaterial();
		// x : ratio
		// y : percent
		// z : maximum
		Vec4 val = mat->GetMaterialParameter(4);
		val.x = p;
		val.y = mMaximum;
		mat->SetMaterialParameter(4, val);

		if (mGaugeColorEmptySet)
		{
			auto mat = mUIObject->GetMaterial();
			Color c= Lerp(mGaugeColorEmpty, mGaugeColor, p);
			mat->SetMaterialParameter(1, c.GetVec4());
		}
	}

	void HorizontalGauge::SetMaximum(float m)
	{
		mMaximum = m;
		auto mat = mUIObject->GetMaterial();
		
		Vec4 val = mat->GetMaterialParameter(4);
		val.y = mMaximum;
		mat->SetMaterialParameter(4, val);
	}

	void HorizontalGauge::Blink(bool blink)
	{
		if (mBlink == blink)
			return;

		mBlink = blink;
		if (!blink){
			auto mat = mUIObject->GetMaterial();
			mat->SetMaterialParameter(3, Vec4(0, 0, 0, 0));
			mBlinkTime = 0;
		}
		else{
			mBlinkTime = FLT_MAX;
		}
	}

	void HorizontalGauge::Blink(bool blink, float time){

		if (mBlink == blink)
			return;

		mBlink = blink;
		mBlinkTime = time;

		if (!blink)
		{
			auto mat = mUIObject->GetMaterial();
			mat->SetMaterialParameter(3, Vec4(0, 0, 0, 0));
			mBlinkTime = 0;
		}

	}

	void HorizontalGauge::SetGaugeColor(const Color& color)
	{
		mGaugeColor = color;
		auto mat = mUIObject->GetMaterial();
		mat->SetMaterialParameter(1, color.GetVec4());
	}

	const Color& HorizontalGauge::GetGaugeColor() const
	{
		return mGaugeColor;
	}

	void HorizontalGauge::SetGaugeColorEmpty(const Color& color)
	{
		mGaugeColorEmpty = color;
		mGaugeColorEmptySet = true;
	}

	void HorizontalGauge::SetBlinkColor(const Color& color)
	{
		mBlinkColor = color;
		auto mat = mUIObject->GetMaterial();
		mat->SetMaterialParameter(2, color.GetVec4());
	}

	const Color& HorizontalGauge::GetBlinkColor() const
	{
		return mBlinkColor;
	}

	bool HorizontalGauge::SetProperty(UIProperty::Enum prop, const char* val)
	{

		switch (prop)
		{
		case UIProperty::GAUGE_MAX:
		{
									  SetMaximum(StringConverter::ParseReal(val));
									  return true;
		}
		case UIProperty::GAUGE_CUR:
		{
									  SetPercentage(StringConverter::ParseReal(val));
									  return true;
		}
		case UIProperty::GAUGE_COLOR:
		{
										SetGaugeColor(StringMathConverter::ParseColor(val));
										return true;
		}
		case UIProperty::GAUGE_COLOR_EMPTY:
		{
											  SetGaugeColorEmpty(StringMathConverter::ParseColor(val));
											  return true;
		}

		case UIProperty::GAUGE_BLINK_COLOR:
		{
											  SetBlinkColor(StringMathConverter::ParseColor(val));
											  return true;
		}

		case UIProperty::GAUGE_BLINK_SPEED:
		{
											  mBlinkSpeed = StringConverter::ParseReal(val);
											  return true;
		}

		case UIProperty::GAUGE_BORDER_COLOR:
		{
											   mGaugeBorderColor = StringMathConverter::ParseColor(val);
											   if (mUIObject)
											   {
												   auto mat = mUIObject->GetMaterial();
												   if (mat)
												   {
													   mat->SetAmbientColor(mGaugeBorderColor.GetVec4());
												   }
											   }
											   return true;
		}
		}

		return __super::SetProperty(prop, val);
	}

	bool HorizontalGauge::GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly)
	{
		switch (prop)
		{
		case UIProperty::GAUGE_MAX:
		{
			if (notDefaultOnly)
			{
				if (mMaximum == UIProperty::GetDefaultValueFloat(prop))
					return false;
			}
			auto data = StringConverter::ToString(mMaximum);
			strcpy_s(val, bufsize, data.c_str());			
			return true;
		}
		case UIProperty::GAUGE_CUR:
		{
			if (notDefaultOnly)
			{
				if (mPercentage == UIProperty::GetDefaultValueFloat(prop))
					return false;
			}
			auto data = StringConverter::ToString(mPercentage);
			strcpy_s(val, bufsize, data.c_str());
			return true;
			
		}
		case UIProperty::GAUGE_COLOR:
		{
			if (notDefaultOnly)
			{
				if (mGaugeColor == UIProperty::GetDefaultValueVec4(prop))
					return false;
			}
			auto data = StringConverter::ToString(mGaugeColor);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}
		case UIProperty::GAUGE_COLOR_EMPTY:
		{
			if (mGaugeColorEmptySet)
			{
				auto data = StringConverter::ToString(mGaugeColorEmpty);
				strcpy_s(val, bufsize, data.c_str());
				return true;
			}
			return false;
		}

		case UIProperty::GAUGE_BLINK_COLOR:
		{
			if (notDefaultOnly)
			{
				if (mBlinkColor == UIProperty::GetDefaultValueVec4(prop))
					return false;
			}
			auto data = StringConverter::ToString(mBlinkColor);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}

		case UIProperty::GAUGE_BLINK_SPEED:
		{
			if (notDefaultOnly)
			{
				if (mBlinkSpeed == UIProperty::GetDefaultValueFloat(prop))
					return false;
			}
			auto data = StringConverter::ToString(mBlinkSpeed);
			strcpy_s(val, bufsize, data.c_str());
			return true;			
		}

		case UIProperty::GAUGE_BORDER_COLOR:
		{
			if (notDefaultOnly)
			{
				if (mGaugeBorderColor == UIProperty::GetDefaultValueVec4(prop))
					return false;
			}
			auto data = StringConverter::ToString(mGaugeBorderColor);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}
		}

		return __super::GetProperty(prop, val, bufsize, notDefaultOnly);
	}


	
}