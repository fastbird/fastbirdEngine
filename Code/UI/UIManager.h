#pragma once
#ifndef _UIManager_header_include__
#define _UIManager_header_include__

#include <UI/IUIManager.h>
#include <UI/DragBox.h>
#include <UI/RegionTestParam.h>
#include <Engine/IFileChangeListener.h>
#include <Engine/IRenderListener.h>

namespace fastbird
{
	void RegisterLuaFuncs(lua_State* mL);
	void RegisterLuaEnums(lua_State* mL);

	class IWinBase;
	class IUIObject;
	class ListBox;
	class UICommands;
	class Container;
	class UIManager : public IUIManager, public IFileChangeListener,
		public IRenderListener
	{
	public:

		typedef std::vector<IWinBase*> WinBases;


	private:

		bool mInputListenerEnable;
		std::string mUIFolder;

		typedef std::list<IWinBase*> WINDOWS;
		VectorMap<HWND_ID, WINDOWS> mWindows;
		std::map<std::string, WinBases> mLuaUIs;

		IWinBase* mFocusWnd;
		IWinBase* mKeyboardFocus;
		Container* mMouseOveredContainer;
		VectorMap<HWND_ID, bool> mNeedToRegisterUIObject;
		bool mMouseIn;
		IWinBase* mTooltipUI;
		IWinBase* mTooltipTextBox;
		std::wstring mTooltipText;

		IWinBase* mPopup;
		std::function< void(void*) > mPopupCallback;
		int mPopupResult;

		lua_State* mL;

		
		bool mPosSizeEventEnabled;
		bool mLockFocus;
		int mIgnoreInput;
		IWinBase* mModalWindow;

		ListBox* mCachedListBox;
		WinBases mAlwaysOnTopWindows;
		WINDOWS mMoveToBottomReserved;
		std::vector < IWinBase* > mSetFocusReserved;

		std::vector<std::string> mHideUIExcepts;

		VectorMap<std::string, IUIAnimation*> mAnimations;
		float mDelayForTooltip;

		UICommands* mUICommands;
		HMODULE mUIEditorModuleHandle;
		ComponentType::Enum mLocatingComp;
		IUIEditor* mUIEditor;

		DragBox mDragBox;
		TextManipulator* mTextManipulator;

		// for locating
		bool mMultiLocating;

		// styles
		std::string mStyle;
		VectorMap<std::string, std::string> mBorderRegions;
		VectorMap<std::string, std::string> mWindowRegions;
		std::string mBorderAlphaRegion;
		std::string mWindowAlphaRegion;

		std::string mStyleStrings[Styles::Num];

		// alpha textures
		VectorMap<Vec2I, SmartPtr<ITexture>> mAlphaInfoTexture;
		SmartPtr<ITexture> mAtlasStaging;


	protected:

		virtual void OnDeleteWinBase(IWinBase* winbase);


	private:

		virtual IWinBase* CreateComponent(ComponentType::Enum type);
		virtual void DeleteComponent(IWinBase* com);
		void OnPopupYes(void* arg);
		void OnPopupNo(void* arg);
		const char* FindUIFilenameWithLua(const char* luafilepath);
		const char* FindUINameWithLua(const char* luafilepath);
		void ShowTooltip();
		void DeleteLuaUIContaning(IWinBase* wnd);

	public:

		UIManager(lua_State* L);
		virtual ~UIManager();

		static UIManager* GetUIManagerStatic();

		virtual void Shutdown();

		// IFileChangeListeners
		virtual bool OnFileChanged(const char* file);

		// IRenderListener
		virtual void BeforeUIRendering(HWND_ID hwndId);
		virtual void BeforeDebugHudRendered(HWND_ID hwndId);
		virtual void AfterDebugHudRendered(HWND_ID hwndId);

		// IUIManager Interfaces
		virtual void Update(float elapsedTime);
		virtual void GatherRenderList();
		virtual bool ParseUI(const char* filepath, WinBases& windows, std::string& uiname, HWND_ID hwndId = INVALID_HWND_ID, bool luaUI = false);
		virtual void SaveUI(const char* uiname, tinyxml2::XMLDocument& doc);
		virtual bool AddLuaUI(const char* uiName, LuaObject& data, HWND_ID hwndId = INVALID_HWND_ID);
		virtual void DeleteLuaUI(const char* uiName);
		virtual bool IsLoadedUI(const char* uiName);

		virtual IWinBase* AddWindow(int posX, int posY, int width, int height, ComponentType::Enum type, HWND_ID hwndId = INVALID_HWND_ID);
		virtual IWinBase* AddWindow(const Vec2I& pos, const Vec2I& size, ComponentType::Enum type, HWND_ID hwndId = INVALID_HWND_ID);
		virtual IWinBase* AddWindow(float posX, float posY, float width, float height, ComponentType::Enum type, HWND_ID hwndId = INVALID_HWND_ID);
		virtual IWinBase* AddWindow(ComponentType::Enum type, HWND_ID hwndId = INVALID_HWND_ID);

		virtual void DeleteWindow(IWinBase* pWnd);
		virtual void DeleteWindowsFor(HWND_ID hwndId);
		virtual void SetFocusUI(IWinBase* pWnd);
		virtual IWinBase* GetFocusUI() const;
		virtual IWinBase* GetKeyboardFocusUI() const;
		virtual void SetFocusUI(const char* uiName);
		virtual bool IsFocused(const IWinBase* pWnd) const;
		virtual void DirtyRenderList(HWND_ID hwndId);

		virtual void SetUIProperty(const char* uiname, const char* compname, const char* prop, const char* val);
		virtual void SetUIProperty(const char* uiname, const char* compname, UIProperty::Enum prop, const char* val);

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
		virtual void FindUIWnds(const char* uiname, WinBases& outV) const;
		virtual bool CacheListBox(const char* uiname, const char* compName);
		virtual ListBox* GetCachedListBox() const{
			return mCachedListBox;
		}
		virtual void SetEnablePosSizeEvent(bool enable) { mPosSizeEventEnabled = enable; }
		virtual bool GetEnablePosSizeEvent() const { return mPosSizeEventEnabled; }
		virtual void SetVisible(const char* uiname, bool visible);
		virtual void LockFocus(bool lock);
		virtual bool GetVisible(const char* uiname) const;
		virtual void CloseAllLuaUI();

		virtual void CloneUI(const char* uiname, const char* newUIname);
		virtual void IgnoreInput(bool ignore, IWinBase* modalWindow);
		virtual void ToggleVisibleLuaUI(const char* uisname);

		virtual void RegisterAlwaysOnTopWnd(IWinBase* win);
		virtual void UnRegisterAlwaysOnTopWnd(IWinBase* win);

		virtual void MoveToBottom(const char* moveToBottom);
		virtual void MoveToBottom(IWinBase* moveToBottom);
		virtual void HideUIsExcept(const std::vector<std::string>& excepts);

		virtual void HighlightUI(const char* uiname);
		virtual void StopHighlightUI(const char* uiname);

		virtual IUIAnimation* GetGlobalAnimation(const char* animName);
		virtual IUIAnimation* GetGlobalAnimationOrCreate(const char* animName);
		virtual void PrepareTooltipUI();

		virtual UICommands* GetUICommands() const { return mUICommands; }
		virtual void SetUIEditorModuleHandle(HMODULE moduleHandle){ mUIEditorModuleHandle = moduleHandle; }
		virtual HMODULE GetUIEditorModuleHandle() const { return mUIEditorModuleHandle; }
		
		virtual IWinBase* WinBaseWithPoint(const Vec2I& pt, const RegionTestParam& param);
		virtual TextManipulator* GetTextManipulator() const { return mTextManipulator; }
		
		virtual const char* GetUIPath(const char* uiname) const;
		virtual const char* GetUIScriptPath(const char* uiname) const;

		virtual void SuppressPropertyWarning(bool suppress);

		virtual void SetStyle(const char* style);

		virtual const char* GetBorderRegion(const char* key) const;
		virtual const char* GetWndBorderRegion(const char* key) const;
		virtual const char* GetStyleString(Styles::Enum s) const;

		virtual ITexture* GetBorderAlphaInfoTexture(const Vec2I& size, bool& callmeLater);

		//-------------------------------------------------------------------
		// For UI Editing
		//-------------------------------------------------------------------
	public:
		virtual void SetUIEditor(IUIEditor* editor);
		virtual IUIEditor* GetUIEditor() const { return mUIEditor; }
		virtual void StartLocatingComponent(ComponentType::Enum c);
		virtual void CancelLocatingComponent();
		virtual void ChangeFilepath(IWinBase* root, const char* newfile);
		virtual void CopyCompsAtMousePos(const std::vector<IWinBase*>& src);

	private:
		void OnInputForLocating(IMouse* pMouse, IKeyboard* pKeyboard);
		void LocateComponent();
		std::string GetUniqueUIName() const;
		std::string GetBackupName(const std::string& name) const;
		void BackupFile(const char* filename);
		void DragUI();
		void AlignUI();

		void DebugUI();
	};
}

#endif //_UIManager_header_include__