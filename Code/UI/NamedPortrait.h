#pragma once

#include <UI/Container.h>

namespace fastbird
{

	class NamedPortrait : public Container
	{
	public:
		NamedPortrait();
		~NamedPortrait();

		// IWinBase
		virtual void OnCreated();
		virtual ComponentType::Enum GetType() const { return ComponentType::NamedPortrait; }
		virtual bool SetProperty(UIProperty::Enum prop, const char* val);

		virtual void SetTexture(ITexture* texture);
		virtual void GatherVisit(std::vector<IUIObject*>& v);

	private:
		IWinBase* mImageBox;
		IWinBase* mTextBox;
	};
}