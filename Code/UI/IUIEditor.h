#pragma once
#include <UI/UIProperty.h>
namespace fastbird
{
	class IWinBase;
	class IUIEditor
	{
	public:
		virtual void OnComponentSelected(IWinBase* comp) {}
		virtual IWinBase* GetCurSelected() const { return 0; }
		virtual unsigned GetNumCurEditing() const { return 0; }
		virtual IWinBase* GetCurSelected(unsigned index) const { return 0; }
		virtual void OnCancelComponent() {}
		virtual void OnPosChanged(IWinBase* comp){}
		virtual void OnSizeChanged(IWinBase* comp){}
		virtual void DrawFocusBoxes(){}
		virtual void TryToDeleteCurComp() {}
		virtual HWND_ID GetHwndId() const { return -1; }

		// process for Ctrl + left, right, up, down. and Ctrl+keypad 5
		virtual void AlignUI(){}
		
	};
}