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
	{
		mDown = (Button*)AddChild(0.0, 0.0, 0.33333f, 1.0f, ComponentType::Button);
		mDown->SetSizeX(12);
		mDown->SetText(L"-");		
		mDown->SetProperty(UIProperty::NO_BACKGROUND, "true");
		mDown->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK,
			std::bind(&NumericUpDown::OnDown, this, std::placeholders::_1));

		mUp = (Button*)AddChild(1.0, 0.0, 0.33333f, 1.0f, ComponentType::Button);
		mUp->SetSizeX(12);
		mUp->SetProperty(UIProperty::ALIGNH, "right");
		mUp->SetProperty(UIProperty::NO_BACKGROUND, "true");
		mUp->SetText(L"+");
		
		mUp->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK,
			std::bind(&NumericUpDown::OnUp, this, std::placeholders::_1));

		mNumber = (StaticText*)AddChild(0.5, 0.0, 0.33333f, 1.0f, ComponentType::StaticText);
		mNumber->SetProperty(UIProperty::ALIGNH, "center");
		mNumber->SetProperty(UIProperty::TEXT_ALIGN, "center");

		
		
	

		WCHAR buffer[100];
		swprintf_s(buffer, L"%d", mValue);
		mNumber->SetText(buffer);
	}

	NumericUpDown::~NumericUpDown()
	{
	}

	void NumericUpDown::Reposition()
	{

	}

	void NumericUpDown::OnPosChanged()
	{
		__super::OnPosChanged();
		//Reposition();
	}

	void NumericUpDown::OnSizeChanged()
	{
		__super::OnSizeChanged();
		//Reposition();
	}

	void NumericUpDown::GatherVisit(std::vector<IUIObject*>& v)
	{
		__super::GatherVisit(v);
	}

	void NumericUpDown::SetNumber(int number)
	{
		mValue = number;
		WCHAR buffer[100];
		swprintf_s(buffer, L"%d", mValue);
		mNumber->SetText(buffer);

		mUp->SetEnable(mValue < mMax);
		mDown->SetEnable(mValue > mMin);
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
			if (mValue == mMin)
				mDown->SetEnable(false);
			
			mUp->SetEnable(mValue < mMax);

			OnEvent(EVENT_NUMERIC_DOWN);
		}
	}

	void NumericUpDown::OnUp(void* arg)
	{
		if (mValue < mMax)
		{
			SetNumber(mValue + 1);
			if (mValue == mMax)
				mUp->SetEnable(false);
			
			mDown->SetEnable(mValue > mMin);
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
}