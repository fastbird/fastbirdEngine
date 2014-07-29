#pragma once
#ifndef _UIManager_header_include__
#define _UIManager_header_include__

#include <UI/IUIManager.h>

namespace fastbird
{
	class IWinBase;
	class UIManager : public IUIManager
	{
		friend class IUIManager;
		UIManager();
		virtual ~UIManager();	

		public:

		virtual void Shutdown();

		// IUIManager Interfaces
		virtual void Update();
		virtual IWinBase* AddWindow(int posX, int posY, int width, int height, ComponentType::Enum type);
		virtual IWinBase* AddWindow(float posX, float posY, float width, float height, ComponentType::Enum type);
		virtual void DeleteWindow(IWinBase* pWnd);
		virtual void SetFocusUI(IWinBase* pWnd);
		virtual bool IsFocused(const IWinBase* pWnd) const;
		virtual void DirtyRenderList();

		// IInputListener Interfaces
		virtual void OnInput(IMouse* pMouse, IKeyboard* pKeyboard);
		virtual void EnableInputListener(bool enable);
		virtual bool IsEnabledInputLIstener() const;
		virtual HCURSOR GetMouseCursorOver() const;
		virtual void DisplayMsg(const std::string& msg, ...);

	protected:
		virtual void OnDeleteWinBase(IWinBase* winbase);

	private:
		virtual IWinBase* CreateComponent(ComponentType::Enum type);

	private:
		bool mInputListenerEnable;

		typedef std::list<IWinBase*> WINDOWS;
		 WINDOWS mWindows;
		 bool mNeedToRegisterUIObject;
		 IWinBase* mFocusWnd;
	
	};
}

#endif //_UIManager_header_include__