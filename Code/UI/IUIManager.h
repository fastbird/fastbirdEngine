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
		static void InitializeUIManager(lua_State* L);
		static void FinalizeUIManager();
		static IUIManager& GetUIManager();

		virtual void Shutdown() = 0;
		virtual void Update(float elapsedTime) = 0;
		virtual void GatherRenderList() = 0;

		virtual bool ParseUI(const char* filepath, std::vector<IWinBase*>& windows, std::string& uiname, bool luaUI = false) = 0;
		virtual bool AddLuaUI(const char* uiName, LuaObject& data) = 0;
		virtual void DeleteLuaUI(const char* uiName) = 0;
		// in screenspace
		virtual IWinBase* AddWindow(int posX, int posY, int width, int height, ComponentType::Enum type) = 0;
		// in normalized space 0.0f~1.0f
		virtual IWinBase* AddWindow(float posX, float posY, float width, float height, ComponentType::Enum type) = 0;
		virtual void DeleteWindow(IWinBase* pWnd) = 0;
		virtual void SetFocusUI(IWinBase* pWnd) = 0;
		virtual IWinBase* GetFocusUI() const = 0;
		virtual void SetFocusUI(const char* uiName) = 0;
		virtual bool IsFocused(const IWinBase* pWnd) const = 0;
		virtual void DirtyRenderList() = 0;
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
		virtual void OnUIFileChanged(const char* file) = 0;
		virtual IWinBase* FindComp(const char* uiname, const char* compName) const = 0;
		virtual void SetEnablePosSizeEvent(bool enable) = 0;
		virtual bool GetEnablePosSizeEvent() const = 0;

		virtual bool GetVisible(const char* uiname) const = 0;
		virtual void SetVisible(const char* uiname, bool visible) = 0;

		virtual void CloneUI(const char* uiname, const char* newUIname) = 0;
		virtual void IgnoreInput(bool ignore) = 0;

	protected:
		virtual void OnDeleteWinBase(IWinBase* winbase) = 0;

	private:
		static IUIManager* mUIManager;

		friend class Container;
		friend class WinBase;
		friend class Wnd;
		friend class DropDown;
		virtual IWinBase* CreateComponent(ComponentType::Enum type) = 0;
		virtual void DeleteComponent(IWinBase* com) = 0;
	};
}