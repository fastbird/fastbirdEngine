#pragma once

#include <UI/Container.h>

namespace fastbird
{
	class IUIObject;
	class Button;
	class StaticText;
	class NumericUpDown : public Container
	{
	public:
		NumericUpDown();
		virtual ~NumericUpDown();
		
		// IWinBase
		virtual void GatherVisit(std::vector<IUIObject*>& v);
		virtual void OnCreated();
		virtual ComponentType::Enum GetType() const { return ComponentType::NumericUpDown; }
		virtual bool SetProperty(UIProperty::Enum prop, const char* val);
		virtual bool GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly);

		virtual void SetNumber(int number);
		virtual void SetMinMax(int min, int max);
		virtual int GetValue() const { return mValue; }

		virtual void SetEnableUp(bool enable);
		virtual void SetEnableDown(bool enable);
		virtual void SetEnable(bool enable);


	protected:
		void InitializeButtons();

		void OnDown(void* arg);
		void OnUp(void* arg);
		

	private:
		Button* mDown;
		Button* mUp;
		int mValue;
		int mMin;
		int mMax;
		int mShiftStep;
		int mStep;
	};

}