/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#pragma once
#ifndef _UIManager_header_include__
#define _UIManager_header_include__

#include "DragBox.h"
#include "RegionTestParam.h"
#include "Styles.h"
#include "UIProperty.h"
#include "ComponentType.h"
#include "Align.h"
#include "UISounds.h"
#include "FBFileMonitor/IFileChangeObserver.h"
#include "FBRenderer/IRendererObserver.h"
#include "FBInputManager/IInputConsumer.h"

namespace tinyxml2{
	class XMLDocument;
}
namespace fb
{
	void RegisterLuaFuncs(lua_State* mL);
	void RegisterLuaEnums(lua_State* mL);
	class TextManipulator;
	class IUIEditor;
	FB_DECLARE_SMART_PTR(Texture);
	FB_DECLARE_SMART_PTR(UICommands);
	FB_DECLARE_SMART_PTR(UIAnimation);
	FB_DECLARE_SMART_PTR(ListBox);
	FB_DECLARE_SMART_PTR(WinBase);		
	FB_DECLARE_SMART_PTR(UIManager);
	class FB_DLL_UI UIManager : public IFileChangeObserver,
		public IRendererObserver, public IInputConsumer
	{
		FB_DECLARE_PIMPL_NON_COPYABLE(UIManager);

	protected:
		UIManager();
		~UIManager();		

	public:
		static UIManagerPtr Create();
		static bool HasInstance();
		static UIManager& GetInstance();

		typedef std::vector<WinBasePtr> WinBases;
		enum POPUP_TYPE
		{
			POPUP_TYPE_YES_NO
		};

		virtual WinBasePtr CreateComponent(ComponentType::Enum type);
		void Shutdown();
		void SetSound(UISounds::Enum type, const char* path);
		void PlaySound(UISounds::Enum type);
		// IFileChangeListeners
		void OnChangeDetected();
		bool OnFileChanged(const char* watchDir, const char* file, const char* loweredExt);

		// IRendererObserver
		void BeforeUIRendering(HWindowId hwndId, HWindow hwnd);
		void RenderUI(HWindowId hwndId, HWindow hwnd);
		void AfterUIRendered(HWindowId hwndId, HWindow hwnd);
		void BeforeDebugHudRendering();
		void AfterDebugHudRendered();
		void OnResolutionChanged(HWindowId hwndId, HWindow hwnd);

		// IUIManager Interfaces
		void Update(float elapsedTime);
		void GatherRenderList();
		bool ParseUI(const char* filepath, WinBases& windows, std::string& uiname, HWindowId hwndId = INVALID_HWND_ID, bool luaUI = false);
		bool SaveUI(const char* uiname, tinyxml2::XMLDocument& doc);
		bool AddLuaUI(const char* uiName, LuaObject& data, HWindowId hwndId = INVALID_HWND_ID);
		void DeleteLuaUI(const char* uiName, bool pending);
		bool IsLoadedUI(const char* uiName);

		WinBasePtr AddWindow(int posX, int posY, int width, int height, ComponentType::Enum type, HWindowId hwndId = INVALID_HWND_ID);
		WinBasePtr AddWindow(const Vec2I& pos, const Vec2I& size, ComponentType::Enum type, HWindowId hwndId = INVALID_HWND_ID);
		WinBasePtr AddWindow(float posX, float posY, float width, float height, ComponentType::Enum type, HWindowId hwndId = INVALID_HWND_ID);
		WinBasePtr AddWindow(ComponentType::Enum type, HWindowId hwndId = INVALID_HWND_ID);

		void DeleteWindow(WinBasePtr pWnd);
		void DeleteWindowsFor(HWindowId hwndId);
		void SetFocusUI(WinBasePtr pWnd);
		WinBasePtr GetFocusUI() const;
		WinBasePtr GetKeyboardFocusUI() const;
		WinBasePtr GetNewFocusUI() const;
		void SetFocusUI(const char* uiName);
		bool IsFocused(const WinBasePtr pWnd) const;
		void DirtyRenderList(HWindowId hwndId);

		void SetUIProperty(const char* uiname, const char* compname, const char* prop, const char* val, bool updatePosSize = false);
		void SetUIProperty(const char* uiname, const char* compname, UIProperty::Enum prop, const char* val, bool updatePosSize = false);
		void SetEnableComponent(const char* uiname, const char* compname, bool enable);

		// IInputListener Interfaces
		void ConsumeInput(IInputInjectorPtr injector);		
		void ProcessMouseInput(IInputInjectorPtr injector);
		void EnableInputListener(bool enable);
		bool IsEnabledInputLIstener() const;
		HCURSOR GetMouseCursorOver() const;
		void SetMouseCursorOver();
		void DisplayMsg(const std::string& msg, ...);
		bool IsMouseInUI() const;

		void SetTooltipString(const std::wstring& ts);
		void SetTooltipPos(const Vec2& npos, bool checkNewPos = true);
		void CleanTooltip();

		void PopupDialog(WCHAR* msg, POPUP_TYPE type, std::function< void(void*) > func);
		int GetPopUpResult() const;

		lua_State* GetLuaState() const;
		WinBasePtr FindComp(const char* uiname, const char* compName) const;
		void FindUIWnds(const char* uiname, WinBases& outV) const;
		bool CacheListBox(const char* uiname, const char* compName);
		ListBoxPtr GetCachedListBox() const;
		void SetEnablePosSizeEvent(bool enable);
		bool GetEnablePosSizeEvent() const;
		void SetVisible(const char* uiname, bool visible);
		void LockFocus(bool lock);
		bool GetVisible(const char* uiname) const;
		void CloseAllLuaUI();

		void CloneUI(const char* uiname, const char* newUIname);
		void IgnoreInput(bool ignore, WinBasePtr modalWindow);
		void ToggleVisibleLuaUI(const char* uisname);

		void RegisterAlwaysOnTopWnd(WinBasePtr win);
		void UnRegisterAlwaysOnTopWnd(WinBasePtr win);

		void MoveToBottom(const char* moveToBottom);
		void MoveToBottom(WinBasePtr moveToBottom);
		void MoveToTop(WinBasePtr moveToTop);
		void HideUIsExcept(const std::vector<std::string>& excepts);

		void HighlightUI(const char* uiname);
		void StopHighlightUI(const char* uiname);

		UIAnimationPtr GetGlobalAnimation(const char* animName);
		UIAnimationPtr GetGlobalAnimationOrCreate(const char* animName);
		void PrepareTooltipUI();

		UICommandsPtr GetUICommands() const;
		void SetUIEditorModuleHandle(ModuleHandle moduleHandle);
		ModuleHandle GetUIEditorModuleHandle() const;
		
		WinBasePtr WinBaseWithPoint(const Vec2I& pt, const RegionTestParam& param);
		WinBasePtr WinBaseWithPointCheckAlways(const Vec2I& pt, const RegionTestParam& param);
		TextManipulator* GetTextManipulator() const;
		
		const char* GetUIPath(const char* uiname) const;
		const char* GetUIScriptPath(const char* uiname) const;

		void SuppressPropertyWarning(bool suppress);

		void SetStyle(const char* style);

		const char* GetBorderRegion(const char* key) const;
		const char* GetWndBorderRegion(const char* key) const;
		const char* GetStyleString(Styles::Enum s) const;

		TexturePtr GetBorderAlphaInfoTexture(const Vec2I& size, bool& callmeLater);

		void AddAlwaysMouseOverCheck(WinBasePtr comp);
		void RemoveAlwaysMouseOverCheck(WinBasePtr comp);

		//-------------------------------------------------------------------
		// For UI Editing
		//-------------------------------------------------------------------	
		void SetUIEditor(IUIEditor* editor);
		IUIEditor* GetUIEditor() const;
		void StartLocatingComponent(ComponentType::Enum c);
		void CancelLocatingComponent();
		void ChangeFilepath(WinBasePtr root, const char* newfile);
		void CopyCompsAtMousePos(const std::vector<WinBasePtr>& src);
	};
}

#endif //_UIManager_header_include__