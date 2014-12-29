#pragma once
#include <UI/WinBase.h>

namespace fastbird
{
	class IUIObject;

	class HorizontalGauge : public WinBase
	{
	public:
		HorizontalGauge();
		virtual ~HorizontalGauge();

		// IWinBase
		virtual ComponentType::Enum GetType() const { return ComponentType::HorizontalGauge; }
		virtual void GatherVisit(std::vector<IUIObject*>& v);
		virtual void OnStartUpdate(float elapsedTime);

		virtual void SetPercentage(float p);
		virtual float GetPercentage() const { return mPercentage; }
		virtual void SetMaximum(float m);
		virtual void Blink(bool blink);
		virtual void SetGaugeColor(const Color& color);
		virtual void SetGaugeColorEmpty(const Color& color);
		virtual void SetBlinkColor(const Color& color);

		virtual bool SetProperty(UIProperty::Enum prop, const char* val);

	private:
		float mPercentage;
		float mMaximum;
		Color mGaugeColor;
		Color mGaugeColorEmpty;
		bool mGaugeColorEmptySet;
		Color mBlinkColor;
		bool mBlink;
		float mBlinkSpeed;


	};
}
