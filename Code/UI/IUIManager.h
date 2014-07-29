#pragma once

#include <Engine/IInputListener.h>
#include <UI/ComponentType.h>
#include <UI/Align.h>
#include <UI/IWinBase.h>

namespace fastbird
{
	class IWinBase;

	class CLASS_DECLSPEC_UI IUIManager : public IInputListener
	{
	public:
		static void InitializeUIManager();
		static void FinalizeUIManager();
		static IUIManager& GetUIManager();

		virtual void Shutdown() = 0;
		virtual void Update() = 0;

		// in screenspace
		virtual IWinBase* AddWindow(int posX, int posY, int width, int height, ComponentType::Enum type) = 0;
		// in normalized space 0.0f~1.0f
		virtual IWinBase* AddWindow(float posX, float posY, float width, float height, ComponentType::Enum type) = 0;
		virtual void DeleteWindow(IWinBase* pWnd) = 0;
		virtual void SetFocusUI(IWinBase* pWnd) = 0;
		virtual bool IsFocused(const IWinBase* pWnd) const = 0;
		virtual void DirtyRenderList() = 0;
		virtual HCURSOR GetMouseCursorOver() const = 0;
		virtual void DisplayMsg(const std::string& msg, ...) = 0;

	protected:
		virtual void OnDeleteWinBase(IWinBase* winbase) = 0;

	private:
		static IUIManager* mUIManager;

		friend class Container;
		virtual IWinBase* CreateComponent(ComponentType::Enum type) = 0;
	};
}