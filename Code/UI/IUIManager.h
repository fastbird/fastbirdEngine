#pragma once

#include <Engine/IInputListener.h>
#include <UI/ComponentType.h>
#include <UI/Align.h>
#include <UI/IWinBase.h>

namespace fastbird
{
	class IWinBase;
	class ListBox;
	class IUIAnimation;
	class UICommands;
	class IUIEditor;
	class TextManipulator;
	class ITexture;
	class IUIManager : public IInputListener
	{
	public:
		virtual void Shutdown() = 0;
		virtual void Update(float elapsedTime) = 0;
		virtual void GatherRenderList() = 0;

		virtual bool ParseUI(const char* filepath, std::vector<IWinBase*>& windows, std::string& uiname, 
			HWND_ID hwndId = INVALID_HWND_ID, bool luaUI = false) = 0;
		virtual void SaveUI(const char* uiname, tinyxml2::XMLDocument& doc) = 0;
		virtual bool AddLuaUI(const char* uiName, LuaObject& data, HWND_ID hwndId = INVALID_HWND_ID) = 0;
		virtual void DeleteLuaUI(const char* uiName) = 0;
		virtual bool IsLoadedUI(const char* uiName) = 0;

		// in screenspace
		virtual IWinBase* AddWindow(int posX, int posY, int width, int height, ComponentType::Enum type, HWND_ID hwndId = INVALID_HWND_ID) = 0;
		virtual IWinBase* AddWindow(const Vec2I& pos, const Vec2I& size, ComponentType::Enum type, HWND_ID hwndId = INVALID_HWND_ID) = 0;
		// in normalized space 0.0f~1.0f
		virtual IWinBase* AddWindow(float posX, float posY, float width, float height, ComponentType::Enum type, HWND_ID hwndId = INVALID_HWND_ID) = 0;

		virtual void DeleteWindow(IWinBase* pWnd) = 0;
		virtual void DeleteWindowsFor(HWND_ID hwndId) = 0;
		virtual void SetFocusUI(IWinBase* pWnd) = 0;
		virtual IWinBase* GetFocusUI() const = 0;
		virtual IWinBase* GetKeyboardFocusUI() const = 0;
		virtual void SetFocusUI(const char* uiName) = 0;
		virtual bool IsFocused(const IWinBase* pWnd) const = 0;
		virtual void DirtyRenderList(HWND_ID hwndId) = 0;
		virtual HCURSOR GetMouseCursorOver() const = 0;
		virtual void SetMouseCursorOver() = 0;
		virtual bool IsMouseInUI() const = 0;
		virtual void DisplayMsg(const std::string& msg, ...) = 0;
		virtual void SetTooltipString(const std::wstring& ts) = 0;
		virtual void SetTooltipPos(const Vec2& npos) = 0;
		virtual void CleanTooltip() = 0;
		enum POPUP_TYPE
		{
			POPUP_TYPE_YES_NO
		};
		virtual void PopupDialog(WCHAR* msg, POPUP_TYPE type, std::function< void(void*) > func)=0;
		virtual int GetPopUpResult() const = 0;
		virtual lua_State* GetLuaState() const = 0;
		virtual IWinBase* FindComp(const char* uiname, const char* compName) const = 0;
		virtual bool CacheListBox(const char* uiname, const char* compName) = 0;
		virtual ListBox* GetCachedListBox() const = 0;
		virtual void SetEnablePosSizeEvent(bool enable) = 0;
		virtual bool GetEnablePosSizeEvent() const = 0;

		virtual bool GetVisible(const char* uiname) const = 0;
		virtual void SetVisible(const char* uiname, bool visible) = 0;
		virtual void SetUIProperty(const char* uiname, const char* compname, const char* UIProperty, const char* val) = 0;
		virtual void SetUIProperty(const char* uiname, const char* compname, UIProperty::Enum UIProperty, const char* val) = 0;
		virtual void LockFocus(bool lock) = 0;
		virtual void CloseAllLuaUI()=0;

		virtual void CloneUI(const char* uiname, const char* newUIname) = 0;
		virtual void IgnoreInput(bool ignore, IWinBase* modalWindow) = 0;
		virtual void ToggleVisibleLuaUI(const char* uisname) = 0;

		virtual void RegisterAlwaysOnTopWnd(IWinBase* win) =0;
		virtual void UnRegisterAlwaysOnTopWnd(IWinBase* win) = 0;

		virtual void MoveToBottom(const char* moveToBottom) = 0;
		virtual void HideUIsExcept(const std::vector<std::string>& excepts)=0;

		virtual void HighlightUI(const char* uiname)=0;
		virtual void StopHighlightUI(const char* uiname)=0;

		virtual IUIAnimation* GetGlobalAnimation(const char* animName) = 0;
		virtual IUIAnimation* GetGlobalAnimationOrCreate(const char* animName) = 0;
		virtual void PrepareTooltipUI() = 0;
		virtual UICommands* GetUICommands() const = 0;
		virtual void SetUIEditorModuleHandle(HMODULE moduleHandle) = 0;
		virtual HMODULE GetUIEditorModuleHandle() const = 0;
		virtual TextManipulator* GetTextManipulator() const = 0;
		virtual IWinBase* WinBaseWithPoint(const Vec2I& pt, const RegionTestParam& param) = 0;

		virtual const char* GetUIPath(const char* uiname) const = 0;
		virtual const char* GetUIScriptPath(const char* uiname) const = 0;
		
		virtual void SuppressPropertyWarning(bool suppress) = 0;

		virtual void CopyCompsAtMousePos(const std::vector<IWinBase*>& src) = 0;
		virtual const char* GetBorderRegion(const char* key) const = 0;
		virtual const char* GetWndBorderRegion(const char* key) const = 0;
		virtual ITexture* GetBorderAlphaInfoTexture(const Vec2I& size, bool& callmeLater) = 0;

	protected:
		virtual void OnDeleteWinBase(IWinBase* winbase) = 0;

	private:
		friend class Container;
		friend class WinBase;
		friend class Wnd;
		friend class DropDown;
		friend class Button;
		friend class TextField;
		friend class PropertyList;
		friend class ListBox;
		virtual IWinBase* CreateComponent(ComponentType::Enum type) = 0;
		virtual void DeleteComponent(IWinBase* com) = 0;



		//-------------------------------------------------------------------
		// For UI Editing
		//-------------------------------------------------------------------
	public:
		virtual void SetUIEditor(IUIEditor* editor) = 0;
		virtual IUIEditor* GetUIEditor() const = 0;
		virtual void StartLocatingComponent(ComponentType::Enum c) = 0;
		virtual void CancelLocatingComponent() = 0;
		virtual void ChangeFilepath(IWinBase* root, const char* newfile) = 0;

	};
}

extern fastbird::IUIManager* gFBUIManager;