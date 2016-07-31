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
#include "NumericUpDown.h"
#include "Button.h"
#include "StaticText.h"
#include "UIManager.h"
#include "UIObject.h"

namespace fb
{

	NumericUpDownPtr NumericUpDown::Create(){
		NumericUpDownPtr p(new NumericUpDown, [](NumericUpDown* obj){ delete obj; });
		p->mSelfPtr = p;
		return p;
	}

	NumericUpDown::NumericUpDown()
		:mValue(0)
		, mMin(0)
		, mMax(100)		
		, mShiftStep(5)
		, mStep(1)
	{
		mUIObject = UIObject::Create(GetRenderTargetSize(), this);
		mUIObject->mOwnerUI = this;
		mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
		mUIObject->SetTextColor(mTextColor);
		mUIObject->SetNoDrawBackground(true);		
	}

	NumericUpDown::~NumericUpDown()
	{
	}

	void NumericUpDown::GatherVisit(std::vector<UIObject*>& v)
	{
		if (!mVisibility.IsVisible())
			return;
		v.push_back(mUIObject.get());

		__super::GatherVisit(v);
	}

	void NumericUpDown::OnCreated()
	{
		InitializeButtons();
	}

	void NumericUpDown::InitializeButtons()
	{
		//mDown = (Button*)AddChild(0.0, 0.0, 0.33333f, 1.0f, ComponentType::Button);
		assert(mDown.expired());
		auto down = std::static_pointer_cast<Button>(AddChild(ComponentType::Button));
		mDown = down;
		down->SetName("down");
		down->SetRuntimeChild(true);


		//mUp = (Button*)AddChild(1.0, 0.0, 0.33333f, 1.0f, ComponentType::Button);
		auto up = std::static_pointer_cast<Button>(AddChild(ComponentType::Button));
		mUp = up;
		up->SetName("up");
		up->SetRuntimeChild(true);

		down->ChangeSize(Vec2I(20, 20));
		down->ChangeNPos(Vec2(0, 0));
		down->SetUseAbsPos(false);

		down->SetProperty(UIProperty::REGION, "DownTriangle");
		down->SetProperty(UIProperty::IMAGE_COLOR_OVERLAY, "1, 1, 0, 1");
		down->SetProperty(UIProperty::NO_BACKGROUND, "true");
		down->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
				std::bind(&NumericUpDown::OnDown, this, std::placeholders::_1));
		down->SetEnable(mValue > mMin && mEnable);

		up->ChangeSize(Vec2I(20, 20));
		up->ChangeNPos(Vec2(1, 0));
		up->SetUseAbsPos(false);
		up->SetProperty(UIProperty::REGION, "UpTriangle");
		up->SetProperty(UIProperty::IMAGE_COLOR_OVERLAY, "1, 1, 0, 1");
		up->SetProperty(UIProperty::ALIGNH, "right");
		up->SetProperty(UIProperty::NO_BACKGROUND, "true");
		up->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
				std::bind(&NumericUpDown::OnUp, this, std::placeholders::_1));
		up->SetEnable(mValue < mMax && mEnable);

		SetProperty(UIProperty::TEXT_ALIGN, "center");

		WCHAR buffer[100];
		swprintf_s(buffer, L"%d", mValue);
		SetText(buffer);

		if (up && down){
			mValue = std::min(mValue, mMax);
			mValue = std::max(mValue, mMin);
			SetNumber(mValue);			
		}
	}

	void NumericUpDown::SetNumber(int number)
	{
		mValue = number;
		mValue = std::min(mMax, mValue);
		mValue = std::max(mMin, mValue);
		WCHAR buffer[100];
		swprintf_s(buffer, L"%d", mValue);
		SetText(buffer);
		auto up = mUp.lock();
		if (up)
			up->SetEnable(mValue < mMax && mEnable);
		auto down = mDown.lock();
		if (down)
			down->SetEnable(mValue > mMin && mEnable);
		OnEvent(UIEvents::EVENT_NUMERIC_SET);
	}
	
	void NumericUpDown::SetMinMax(int min, int max)
	{
		mMin = min;
		mMax = max;
		auto up = mUp.lock();
		auto down = mDown.lock();
		if (up && down){
			up->SetEnable(mValue < mMax && mEnable);
			down->SetEnable(mValue > mMin && mEnable);
		}
	}

	void NumericUpDown::OnDown(void* arg)
	{
		if (mValue > mMin)
		{
			auto injector = InputManager::GetInstance().GetInputInjector();
			if (injector->IsKeyDown(VK_SHIFT)){
				SetNumber(mValue - mShiftStep);
			}
			else if (injector->IsKeyDown(VK_CONTROL)) {
				SetNumber(mMin);
			}
			else{
				SetNumber(mValue - mStep);
			}
			InputManager::GetInstance().GetInputInjector()->InvalidateClickTime();
			OnEvent(UIEvents::EVENT_NUMERIC_DOWN);
		}
	}

	void NumericUpDown::OnUp(void* arg)
	{
		if (mValue < mMax)
		{
			auto injector = InputManager::GetInstance().GetInputInjector();
			if (injector->IsKeyDown(VK_SHIFT)){
				SetNumber(mValue + mShiftStep);
			}
			else if (injector->IsKeyDown(VK_CONTROL)) {
				SetNumber(mMax);
			}
			else{
				SetNumber(mValue + mStep);
			}
			InputManager::GetInstance().GetInputInjector()->InvalidateClickTime();
			OnEvent(UIEvents::EVENT_NUMERIC_UP);
		}
	}

	void NumericUpDown::SetEnableUp(bool enable)
	{
		auto up = mUp.lock();
		if (up)
			up->SetEnable(enable&& mEnable);
	}

	void NumericUpDown::SetEnableDown(bool enable)
	{
		auto down = mDown.lock();
		if (down)
			down->SetEnable(enable&& mEnable);
	}
	void NumericUpDown::SetEnable(bool enable){
		__super::SetEnable(enable);
		auto up = mUp.lock();
		if (up)
			up->SetEnable(mValue < mMax && enable);

		auto down = mDown.lock();
		if (down)
			down->SetEnable(mValue > mMin && enable);
	}

	bool NumericUpDown::SetProperty(UIProperty::Enum prop, const char* val)
	{
		switch (prop)
		{
		case UIProperty::NUMERIC_UPDOWN_MINMAX:
		{
												  Vec2I minmax = StringMathConverter::ParseVec2I(val);
												  SetMinMax(minmax.x, minmax.y);
												  return true;
		}
		case UIProperty::NUMERIC_UPDOWN_NUMBER:
		{
			int num = StringConverter::ParseInt(val);
			SetNumber(num);
			return true;
		}

		case UIProperty::NUMERIC_UPDOWN_SHIFT_STEP:
		{
			mShiftStep = StringConverter::ParseInt(val);
			return true;
		}
		case UIProperty::NUMERIC_UPDOWN_STEP:
		{
			mStep = StringConverter::ParseInt(val);
			return true;
		}
		}

		return __super::SetProperty(prop, val);
	}

	bool NumericUpDown::GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly)
	{
		switch (prop)
		{
		case UIProperty::NUMERIC_UPDOWN_MINMAX:
		{
			if (notDefaultOnly)
			{
				if (Vec2I(mMin, mMax)== UIProperty::GetDefaultValueVec2I(prop))
					return false;
			}
			auto data = StringMathConverter::ToString(Vec2I(mMin, mMax));
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}
		case UIProperty::NUMERIC_UPDOWN_NUMBER:
		{
			if (notDefaultOnly)
			{
				if (mValue == UIProperty::GetDefaultValueInt(prop))
					return false;
			}
			auto data = StringConverter::ToString(mValue);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}
		case UIProperty::NUMERIC_UPDOWN_SHIFT_STEP:
		{
			if (notDefaultOnly){
				if (mShiftStep == UIProperty::GetDefaultValueInt(prop))
					return false;
			}
			strcpy_s(val, bufsize, StringConverter::ToString(mShiftStep).c_str());
			return true;
		}
		case UIProperty::NUMERIC_UPDOWN_STEP:
		{
			if (notDefaultOnly){
				if (mStep == UIProperty::GetDefaultValueInt(prop))
					return false;
			}
			strcpy_s(val, bufsize, StringConverter::ToString(mStep).c_str());
			return true;
		}
		}

		return __super::GetProperty(prop, val, bufsize,  notDefaultOnly);
	}
}