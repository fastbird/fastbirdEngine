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
		virtual ComponentType::Enum GetType() const { return ComponentType::NumericUpDown; }
		virtual void GatherVisit(std::vector<IUIObject*>& v);
		virtual void OnSizeChanged();
		virtual void OnPosChanged();

		virtual void SetNumber(int number);
		virtual void SetMinMax(int min, int max);

		virtual void SetEnableUp(bool enable);
		virtual void SetEnableDown(bool enable);

	protected:
		void Reposition();

		void OnDown(void* arg);
		void OnUp(void* arg);
		

	private:
		Button* mDown;
		Button* mUp;
		StaticText* mNumber;
		int mValue;
		int mMin;
		int mMax;
	};

}