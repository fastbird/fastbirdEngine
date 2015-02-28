#pragma once

#include <UI/Container.h>

namespace fastbird
{
	class IUIObject;
	class Button;
	class StaticText;

	class ColorRampComp : public Container
	{
	public:
		ColorRampComp();
		virtual ~ColorRampComp();

		// IWinBase
		virtual ComponentType::Enum GetType() const { return ComponentType::ColorRamp; }
		virtual void GatherVisit(std::vector<IUIObject*>& v);
		virtual bool SetProperty(UIProperty::Enum prop, const char* val);
		virtual bool GetProperty(UIProperty::Enum prop, char val[]);
		virtual void OnChildHasDragged();
		// alread added as a child

		void SetColorRampValues(const char* values);
		void GetColorRampValues(char val[]);
		void GetColorRampValuesFloats(std::vector<float>& values);
		void SetColorRampValuesFloats(const std::vector<float>& values);

	private:
		std::vector<Button*> mBars;
		std::vector<StaticText*> mTexts;

	};

}