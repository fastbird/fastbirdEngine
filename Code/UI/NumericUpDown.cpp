#include <UI/StdAfx.h>
#include <UI/NumericUpDown.h>
#include <UI/Button.h>
#include <UI/StaticText.h>
#include <UI/IUIManager.h>
#include <Engine/GlobalEnv.h>
#include <CommonLib/StringUtils.h>

namespace fastbird
{
	NumericUpDown::NumericUpDown()
		:mValue(0)
		, mMin(0)
		, mMax(100)
		, mUp(0)
		, mDown(0)
		, mShiftStep(5)
		, mStep(1)
	{
		mUIObject = gFBEnv->pEngine->CreateUIObject(false, GetRenderTargetSize());
		mUIObject->mOwnerUI = this;
		mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
		mUIObject->SetTextColor(mTextColor);
		mUIObject->SetNoDrawBackground(true);		
	}

	NumericUpDown::~NumericUpDown()
	{
	}

	void NumericUpDown::GatherVisit(std::vector<IUIObject*>& v)
	{
		if (!mVisibility.IsVisible())
			return;
		v.push_back(mUIObject);

		__super::GatherVisit(v);
	}

	void NumericUpDown::OnCreated()
	{
		InitializeButtons();
	}

	void NumericUpDown::InitializeButtons()
	{
		//mDown = (Button*)AddChild(0.0, 0.0, 0.33333f, 1.0f, ComponentType::Button);
		assert(!mDown);
		mDown = (Button*)AddChild(ComponentType::Button);
		mDown->SetName("down");
		mDown->SetRuntimeChild(true);


		//mUp = (Button*)AddChild(1.0, 0.0, 0.33333f, 1.0f, ComponentType::Button);
		mUp = (Button*)AddChild(ComponentType::Button);
		mUp->SetName("up");
		mUp->SetRuntimeChild(true);

		mDown->ChangeSize(Vec2I(20, 20));
		mDown->ChangeNPos(Vec2(0, 0));
		mDown->SetUseAbsPos(false);

		mDown->SetProperty(UIProperty::REGION, "DownTriangle");
		mDown->SetProperty(UIProperty::IMAGE_COLOR_OVERLAY, "1, 1, 0, 1");
		mDown->SetProperty(UIProperty::NO_BACKGROUND, "true");
		mDown->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
				std::bind(&NumericUpDown::OnDown, this, std::placeholders::_1));
		mDown->SetEnable(mValue > mMin && mEnable);

		mUp->ChangeSize(Vec2I(20, 20));
		mUp->ChangeNPos(Vec2(1, 0));
		mUp->SetUseAbsPos(false);
		mUp->SetProperty(UIProperty::REGION, "UpTriangle");
		mUp->SetProperty(UIProperty::IMAGE_COLOR_OVERLAY, "1, 1, 0, 1");
		mUp->SetProperty(UIProperty::ALIGNH, "right");
		mUp->SetProperty(UIProperty::NO_BACKGROUND, "true");
		mUp->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
				std::bind(&NumericUpDown::OnUp, this, std::placeholders::_1));
		mUp->SetEnable(mValue < mMax && mEnable);

		SetProperty(UIProperty::TEXT_ALIGN, "center");

		WCHAR buffer[100];
		swprintf_s(buffer, L"%d", mValue);
		SetText(buffer);

		if (mUp && mDown){
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
		if (mUp)
			mUp->SetEnable(mValue < mMax && mEnable);
		if (mDown)
			mDown->SetEnable(mValue > mMin && mEnable);
		OnEvent(UIEvents::EVENT_NUMERIC_SET);
	}
	
	void NumericUpDown::SetMinMax(int min, int max)
	{
		mMin = min;
		mMax = max;

		if (mUp && mDown){
			mUp->SetEnable(mValue < mMax && mEnable);
			mDown->SetEnable(mValue > mMin && mEnable);
		}
	}

	void NumericUpDown::OnDown(void* arg)
	{
		if (mValue > mMin)
		{
			if (gFBEnv->pEngine->GetKeyboard()->IsKeyDown(VK_SHIFT)){
				SetNumber(mValue - mShiftStep);
			}
			else{
				SetNumber(mValue - mStep);
			}
			
			OnEvent(UIEvents::EVENT_NUMERIC_DOWN);
		}
	}

	void NumericUpDown::OnUp(void* arg)
	{
		if (mValue < mMax)
		{
			if (gFBEnv->pEngine->GetKeyboard()->IsKeyDown(VK_SHIFT)){
				SetNumber(mValue + mShiftStep);
			}
			else{
				SetNumber(mValue + mStep);
			}
			OnEvent(UIEvents::EVENT_NUMERIC_UP);
		}
	}

	void NumericUpDown::SetEnableUp(bool enable)
	{
		if (mUp)
			mUp->SetEnable(enable&& mEnable);
	}

	void NumericUpDown::SetEnableDown(bool enable)
	{
		if (mDown)
			mDown->SetEnable(enable&& mEnable);
	}
	void NumericUpDown::SetEnable(bool enable){
		__super::SetEnable(enable);
		if (mUp)
			mUp->SetEnable(mValue < mMax && enable);
		if (mDown)
			mDown->SetEnable(mValue > mMin && enable);
	}

	bool NumericUpDown::SetProperty(UIProperty::Enum prop, const char* val)
	{
		switch (prop)
		{
		case UIProperty::NUMERIC_UPDOWN_MINMAX:
		{
												  Vec2I minmax = StringConverter::parseVec2I(val);
												  SetMinMax(minmax.x, minmax.y);
												  return true;
		}
		case UIProperty::NUMERIC_UPDOWN_NUMBER:
		{
			int num = StringConverter::parseInt(val);
			SetNumber(num);
			return true;
		}

		case UIProperty::NUMERIC_UPDOWN_SHIFT_STEP:
		{
			mShiftStep = StringConverter::parseInt(val);
			return true;
		}
		case UIProperty::NUMERIC_UPDOWN_STEP:
		{
			mStep = StringConverter::parseInt(val);
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
			auto data = StringConverter::toString(Vec2I(mMin, mMax));
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
			auto data = StringConverter::toString(mValue);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}
		case UIProperty::NUMERIC_UPDOWN_SHIFT_STEP:
		{
			if (notDefaultOnly){
				if (mShiftStep == UIProperty::GetDefaultValueInt(prop))
					return false;
			}
			strcpy_s(val, bufsize, StringConverter::toString(mShiftStep).c_str());
			return true;
		}
		case UIProperty::NUMERIC_UPDOWN_STEP:
		{
			if (notDefaultOnly){
				if (mStep == UIProperty::GetDefaultValueInt(prop))
					return false;
			}
			strcpy_s(val, bufsize, StringConverter::toString(mStep).c_str());
			return true;
		}
		}

		return __super::GetProperty(prop, val, bufsize,  notDefaultOnly);
	}
}