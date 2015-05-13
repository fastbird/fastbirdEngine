#pragma once
#include <UI/UIProperty.h>
namespace fastbird
{
	class IWinBase;
	class IUIEditor
	{
	public:
		virtual void OnComponentSelected(IWinBase* comp) {}
		virtual void OnPosChanged(IWinBase* comp){}
		virtual void OnSizeChanged(IWinBase* comp){}
	};
}