#pragma once
#include <UI/WinBase.h>

namespace fastbird
{
class IUIObject;

class VerticalGauge : public WinBase
{
public:
	VerticalGauge();
	virtual ~VerticalGauge();

	// IWinBase
	virtual ComponentType::Enum GetType() const { return ComponentType::VerticalGauge; }
	virtual void GatherVisit(std::vector<IUIObject*>& v);
	virtual void OnStartUpdate(float elapsedTime);

	virtual void SetPercentage(float p);
	virtual void SetMaximum(float m);
	virtual void Blink(bool blink);
	virtual void SetGaugeColor(const Color& color);
	virtual void SetBlinkColor(const Color& color);

	virtual bool SetProperty(UIProperty::Enum prop, const char* val);

private:
	float mPercentage;
	float mMaximum;
	Color mGaugeColor;
	Color mBlinkColor;
	bool mBlink;
	float mBlinkSpeed;


};
}
