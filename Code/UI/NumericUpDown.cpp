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
		mDown = (Button*)AddChild(ComponentType::Button);


		//mUp = (Button*)AddChild(1.0, 0.0, 0.33333f, 1.0f, ComponentType::Button);
		mUp = (Button*)AddChild(ComponentType::Button);

		mDown->SetNSizeY(1.0);
		mDown->SetSizeX(12);
		mDown->SetNPos(Vec2(0, 0));		
		mDown->SetText(L"-");		
		mDown->SetProperty(UIProperty::NO_BACKGROUND, "true");
		mDown->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK,
				std::bind(&NumericUpDown::OnDown, this, std::placeholders::_1));
		mDown->SetEnable(mValue >= mMin);

		mUp->SetNSizeY(1.0f);
		mUp->SetSizeX(12);
		mUp->SetNPos(Vec2(1, 0));
		mUp->SetProperty(UIProperty::ALIGNH, "right");
		mUp->SetProperty(UIProperty::NO_BACKGROUND, "true");
		mUp->SetText(L"+");		
		mUp->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK,
				std::bind(&NumericUpDown::OnUp, this, std::placeholders::_1));
		mUp->SetEnable(mValue <= mMax);
		SetProperty(UIProperty::TEXT_ALIGN, "center");

		WCHAR buffer[100];
		swprintf_s(buffer, L"%d", mValue);
		SetText(buffer);
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
			mUp->SetEnable(mValue <= mMax);
		if (mDown)
			mDown->SetEnable(mValue >= mMin);
		OnEvent(EVENT_NUMERIC_SET);
	}
	
	void NumericUpDown::SetMinMax(int min, int max)
	{
		mMin = min;
		mMax = max;

		mUp->SetEnable(mValue < mMax);
		mDown->SetEnable(mValue > mMin);
	}

	void NumericUpDown::OnDown(void* arg)
	{
		if (mValue > mMin)
		{
			SetNumber(mValue - 1);
			OnEvent(EVENT_NUMERIC_DOWN);
		}
	}

	void NumericUpDown::OnUp(void* arg)
	{
		if (mValue < mMax)
		{
			SetNumber(mValue + 1);
			OnEvent(EVENT_NUMERIC_UP);
		}
	}

	void NumericUpDown::SetEnableUp(bool enable)
	{
		mUp->SetEnable(enable);
	}

	void NumericUpDown::SetEnableDown(bool enable)
	{
		mDown->SetEnable(enable);
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
		}

		return __super::SetProperty(prop, val);
	}

	bool NumericUpDown::GetProperty(UIProperty::Enum prop, char val[])
	{
		switch (prop)
		{
		case UIProperty::NUMERIC_UPDOWN_NUMBER:
		{
			sprintf_s(val, 256, "%d", mValue);
			return true;
		}
		}

		return __super::GetProperty(prop, val);
	}
}