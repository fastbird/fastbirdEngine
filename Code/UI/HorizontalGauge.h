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
		virtual float GetMaximum() const { return mMaximum; }
		virtual void Blink(bool blink);
		virtual void SetGaugeColor(const Color& color);
		virtual const Color& GetGaugeColor() const;
		virtual void SetGaugeColorEmpty(const Color& color);
		virtual void SetBlinkColor(const Color& color);
		virtual const Color& GetBlinkColor() const;

		virtual bool SetProperty(UIProperty::Enum prop, const char* val);
		virtual bool GetProperty(UIProperty::Enum prop, char val[], bool notDefaultOnly);

	private:
		float mPercentage;
		float mMaximum;
		Color mGaugeColor;
		Color mGaugeColorEmpty;
		Color mGaugeBorderColor;
		bool mGaugeColorEmptySet;
		Color mBlinkColor;
		bool mBlink;
		float mBlinkSpeed;


	};
}
