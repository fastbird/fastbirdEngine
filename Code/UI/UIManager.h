#pragma once
#ifndef _UIManager_header_include__
#define _UIManager_header_include__

#include <UI/IUIManager.h>

namespace fastbird
{
	class IWinBase;
	class IUIObject;
	class UIManager : public IUIManager
	{
		friend class IUIManager;
		UIManager(lua_State* L);
		virtual ~UIManager();	

	public:
		
		static UIManager* GetUIManagerStatic();

		virtual void Shutdown();

		// IUIManager Interfaces
		virtual void Update(float elapsedTime);
		virtual bool ParseUI(const char* filepath, std::vector<IWinBase*>& windows, std::string& uiname, bool luaUI = false);
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
		virtual void SetMouseCursorOver();
		virtual void DisplayMsg(const std::string& msg, ...);
		virtual bool IsMouseInUI() const { return mMouseIn; }		

		virtual void SetTooltipString(const std::wstring& ts);
		virtual void SetTooltipPos(const Vec2& npos);
		virtual void CleanTooltip();

		virtual void PopupDialog(WCHAR* msg, POPUP_TYPE type, std::function< void(void*) > func);
		virtual int GetPopUpResult() const{
			return mPopupResult;
		}

		virtual lua_State* GetLuaState() const { return mL; }
		virtual IWinBase* FindComp(const char* uiname, const char* compName) const;
		void SetVisible(const char* uiname, bool visible);
		bool GetVisible(const char* uiname) const;
		virtual void OnUIFileChanged(const char* file);

	protected:
		virtual void OnDeleteWinBase(IWinBase* winbase);

	private:
		virtual IWinBase* CreateComponent(ComponentType::Enum type);
		virtual void DeleteComponent(IWinBase* com);
		void OnPopupYes(void* arg);
		void OnPopupNo(void* arg);
		void RegisterLuaFuncs();
		const char* FindUIFilenameWithLua(const char* luafilepath);
		const char* FindUINameWithLua(const char* luafilepath);

	private:
		bool mInputListenerEnable;

		typedef std::list<IWinBase*> WINDOWS;
		 WINDOWS mWindows;		 
		 IWinBase* mFocusWnd;
		 bool mNeedToRegisterUIObject;
		 bool mMouseIn;
		 IUIObject* mTooltipUI;
		 std::wstring mTooltipText;

		 IWinBase* mPopup;
		 std::function< void(void*) > mPopupCallback;
		 int mPopupResult;

		 lua_State* mL;

		 std::map<std::string, std::vector<IWinBase*>> mLuaUIs;
	
	};
}

#endif //_UIManager_header_include__