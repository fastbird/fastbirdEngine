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

#include "StdAfx.h"
#include "UIManager.h"
#include "FBSystemLib/ModuleHandler.h"
#include "IUIEditor.h"
#include "Wnd.h"
#include "TextField.h"
#include "StaticText.h"
#include "Button.h"
#include "KeyboardCursor.h"
#include "ImageBox.h"
#include "CheckBox.h"
#include "ListBox.h" 
#include "ListItem.h"
#include "FileSelector.h"
#include "Scroller.h"
#include "RadioBox.h"
#include "HexagonalContextMenu.h"
#include "CardScroller.h"
#include "VerticalGauge.h"
#include "HorizontalGauge.h"
#include "NumericUpDown.h"
#include "DropDown.h"
#include "TextBox.h"
#include "ColorRampComp.h"
#include "NamedPortrait.h"
#include "UIAnimation.h"
#include "UICommands.h"
#include "PropertyList.h"
#include "TabWindow.h"
#include "UIObject.h"
#include "FBRenderer/TextureAtlas.h"
#include "FBRenderer/Texture.h"
#include "FBAudioPlayer/AudioManager.h"
using namespace fb;
namespace fb{
	static float gTooltipFontSize = 20;
	static float gDelayForTooltip = 0.7f;
	static const int TooltipTextYPosition = 4;
	extern float gTextSizeMod;
}
class UIManager::Impl{
public:
	UIManager* mSelf;
	bool mInputListenerEnable;
	std::string mUIFolder;
	typedef std::vector<WinBaseWeakPtr> WinBasesWeak;
	typedef std::list<WinBasePtr> WINDOWS;
	typedef std::list<WinBaseWeakPtr> WINDOWSWeak;
	VectorMap<HWindowId, WINDOWS> mWindows;
	std::map<std::string, WinBases> mLuaUIs;
	std::map<std::string, WinBases> mCppUIs;

	std::map<HWindowId, std::vector<UIObject*>> mRenderUIs;
	WinBaseWeakPtr mFocusWnd;
	WinBaseWeakPtr mKeyboardFocus;
	WinBaseWeakPtr mNewFocusWnd;
	WinBaseWeakPtr mMouseOvered;
	WinBaseWeakPtr mMouseDragStartedUI;
	ContainerWeakPtr mMouseOveredContainer;
	VectorMap<HWindowId, bool> mNeedToRegisterUIObject;
	bool mMouseIn;
	WinBasePtr mTooltipUI;
	WinBaseWeakPtr mTooltipTextBox;
	std::wstring mTooltipText;

	WinBasePtr mPopup;
	std::function< void(void*) > mPopupCallback;
	int mPopupResult;

	lua_State* mL;


	bool mPosSizeEventEnabled;
	bool mLockFocus;
	int mIgnoreInput;
	WinBaseWeakPtr mModalWindow;

	ListBoxWeakPtr mCachedListBox;
	WinBasesWeak mAlwaysOnTopWindows;
	WINDOWSWeak mMoveToBottomReserved;
	WINDOWSWeak mMoveToTopReserved;
	std::vector < WinBaseWeakPtr > mSetFocusReserved;

	std::vector<std::string> mHideUIExcepts;

	VectorMap<std::string, UIAnimationPtr> mAnimations;
	float mDelayForTooltip;

	UICommandsPtr mUICommands;
	ModuleHandle mUIEditorModuleHandle;
	ComponentType::Enum mLocatingComp;
	IUIEditor* mUIEditor;

	DragBox mDragBox;
	TextManipulatorPtr mTextManipulator;

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
	VectorMap<Vec2I, TexturePtr> mAlphaInfoTexture;
	TexturePtr mAtlasStaging;

	std::vector<std::string> mDeleteLuaUIPending;
	std::set<WinBaseWeakPtr, std::owner_less<WinBaseWeakPtr>> mAlwaysMouseOverCheckComps;

	Vec2 mPrevTooltipNPos;

	KeyboardCursorPtr mKeyboardCursor;
	std::vector<std::string> mSounds;	

	//---------------------------------------------------------------------------
	Impl(UIManager* self)
		: mSelf(self)
		, mInputListenerEnable(true)		
		, mMouseIn(false)
		, mPopup(0)
		, mPopupCallback(std::function< void(void*) >())
		, mPopupResult(0)
		, mPosSizeEventEnabled(true), mIgnoreInput(false)		
		, mLockFocus(false), mDelayForTooltip(0)
		, mUIEditorModuleHandle(0), mLocatingComp(ComponentType::NUM)
		, mUIEditor(0)
		, mUIFolder("data/ui")
		, mMultiLocating(false), mAtlasStaging(0)
		, mPrevTooltipNPos(-1, -1)
	{	
		auto filepath = "_FBUI.log";
		FileSystem::BackupFile(filepath, 5, "Backup_Log");
		Logger::Init(filepath);
		mSounds.assign(UISounds::Num, std::string());
	}

	~Impl(){
		mTextManipulator = 0;

		/*if (gFBEnv->pConsole)
			gFBEnv->pConsole->ProcessCommand("KillUIEditor");*/

		mUIEditor = 0;
		mUIEditorModuleHandle = 0;
		WinBase::FinalizeMouseCursor();
		if (mUIEditorModuleHandle)
		{
			ModuleHandler::UnloadModule(mUIEditorModuleHandle);
		}
		mUICommands = 0;		
		mAnimations.clear();

		DeleteWindow(mPopup);
		Shutdown();
		assert(mWindows.empty());
		Logger::Release();
	}

	void Initialize(){
		mL = LuaUtils::GetLuaState();
		/*gFBEnv->pEngine->AddInputListener(this,
		fb::IInputListener::INPUT_LISTEN_PRIORITY_UI, 0);*/
		mKeyboardCursor = KeyboardCursor::Create();
		RegisterLuaFuncs(mL);
		RegisterLuaEnums(mL);
		mUICommands = UICommands::Create();
		SetStyle("blue");
		PrepareTooltipUI();
		WinBase::InitMouseCursor();
		//gFBEnv->pEngine->RegisterFileChangeListener(this);
		//gFBEnv->pRenderer->AddRenderListener(this);
		mTextManipulator = TextManipulator::Create();
		LuaObject textSizeMod(mL, "ui_textSizeMod");
		if (textSizeMod.IsValid()){
			gTextSizeMod = textSizeMod.GetFloat(0);
		}
	}

	WinBasePtr CreateComponent(ComponentType::Enum type){
		WinBasePtr pWnd = 0;
		switch (type)
		{
		case ComponentType::Window:
			pWnd = Wnd::Create();
			break;
		case ComponentType::TextField:
			pWnd = TextField::Create();
			break;
		case ComponentType::StaticText:
			pWnd = StaticText::Create();
			break;
		case ComponentType::Button:
			pWnd = Button::Create();
			break;
		case ComponentType::ImageBox:
			pWnd = ImageBox::Create();
			break;
		case ComponentType::CheckBox:
			pWnd = CheckBox::Create();
			break;
		case ComponentType::ListBox:
			pWnd = ListBox::Create();
			break;
		case ComponentType::ListItem:
			pWnd = ListItem::Create();
			break;
		case ComponentType::FileSelector:
			pWnd = FileSelector::Create();
			break;
		case ComponentType::Scroller:
			pWnd = Scroller::Create();
			break;
		case ComponentType::RadioBox:
			pWnd = RadioBox::Create();
			break;
		case ComponentType::Hexagonal:
			pWnd = HexagonalContextMenu::Create();
			break;
		case ComponentType::CardScroller:
			pWnd = CardScroller::Create();
			break;
		case ComponentType::VerticalGauge:
			pWnd = VerticalGauge::Create();
			break;
		case ComponentType::HorizontalGauge:
			pWnd = HorizontalGauge::Create();
			break;
		case ComponentType::NumericUpDown:
			pWnd = NumericUpDown::Create();
			break;
		case ComponentType::DropDown:
			pWnd = DropDown::Create();
			break;
		case ComponentType::TextBox:
			pWnd = TextBox::Create();
			break;
		case ComponentType::ColorRamp:
			pWnd = ColorRampComp::Create();
			break;
		case ComponentType::NamedPortrait:
			pWnd = NamedPortrait::Create();
			break;
		case ComponentType::PropertyList:
			pWnd = PropertyList::Create();
			break;
		case ComponentType::TabWindow:
			pWnd = TabWindow::Create();
			break;
		default:
			assert(0 && "Unknown component");
		}
		return pWnd;
	}

	void OnPopupYes(void* arg){
		assert(mPopup);
		mPopupResult = 1;
		mPopup->SetVisible(false);
		mPopupCallback(this);
	}

	void OnPopupNo(void* arg){
		assert(mPopup);
		mPopupResult = 0;
		mPopup->SetVisible(false);
		mPopupCallback(this);
	}

	const char* FindUIFilenameWithLua(const char* luafilepath){
		for (const auto& it : mLuaUIs)
		{
			const auto& wins = it.second;
			for (const auto& win : wins)
			{
				if (strcmp(win->GetScriptPath(), luafilepath) == 0)
				{
					return win->GetUIFilePath();
				}
			}
		}
		return "";
	}

	const char* FindUINameWithLua(const char* luafilepath){
		for (const auto& it : mLuaUIs)
		{
			const auto& wins = it.second;
			for (const auto& win : wins)
			{
				if (_stricmp(win->GetScriptPath(), luafilepath) == 0)
				{
					return it.first.c_str();
				}
			}
		}
		return "";
	}

	void ShowTooltip(){
		assert(!mTooltipText.empty());
		FontPtr pFont = Renderer::GetInstance().GetFontWithHeight(gTooltipFontSize);
		int width = (int)pFont->GetTextWidth(
			(const char*)mTooltipText.c_str(), mTooltipText.size() * 2);

		const int maxWidth = 450;
		width = std::min(maxWidth, width) + 4;
		mTooltipUI->ChangeSizeX(width + 16);
		auto tooltipTextBox = mTooltipTextBox.lock();
		assert(tooltipTextBox);
		tooltipTextBox->ChangeSizeX(width);
		tooltipTextBox->SetText(mTooltipText.c_str());
		int textWidth = tooltipTextBox->GetTextWidth();
		mTooltipUI->ChangeSizeX(textWidth + 16);
		tooltipTextBox->ChangeSizeX(textWidth + 4);
		int sizeY = tooltipTextBox->GetPropertyAsInt(UIProperty::SIZEY);
		mTooltipUI->ChangeSizeY(sizeY + TooltipTextYPosition*2);
		mTooltipUI->SetVisible(true);
		RefreshTooltipPos();
	}

	void DeleteLuaUIContaning(WinBasePtr wnd){
		for (auto it = mLuaUIs.begin(); it != mLuaUIs.end(); ++it)
		{
			auto& uis = it->second;
			for (auto ui : uis)
			{
				if (ui == wnd)
				{
					mLuaUIs.erase(it);
					return;
				}
			}
		}
	}

	void RefreshTooltipPos(){
		SetTooltipPos(mPrevTooltipNPos, false);
	}


	void Shutdown(){	
		mWindows.clear();
		mLuaUIs.clear();
		mCppUIs.clear();
	}

	void SetSound(UISounds::Enum type, const char* path){
		if (!ValidCStringLength(path))
			return;

		while ((int)mSounds.size() <= type){
			mSounds.push_back(std::string());
		}

		mSounds[type] = path;
	}

	void PlaySound(UISounds::Enum type){
		if ((int)mSounds.size() <= type || mSounds[type].empty()){
			Logger::Log(FB_ERROR_LOG_ARG, "No ui sound is set for (%d)", type);
			return;
		}
		AudioProperty prop;
		prop.mRelative = true;
		AudioManager::GetInstance().PlayAudio(mSounds[type].c_str(), prop);	
	}

	// IFileChangeListeners
	bool OnFileChanged(const char* watchDir, const char* file, const char* loweredExtension){
		assert(file);
		std::string lower(file);
		ToLowerCase(lower);

		std::string extension(loweredExtension);
		std::string uiname;
		std::string filepath = lower;
		if (extension==".lua")
		{
			uiname = FindUINameWithLua(lower.c_str());
			filepath = FindUIFilenameWithLua(lower.c_str());

			if (mUIEditor && !uiname.empty()){
				auto itFind = mLuaUIs.find(uiname);
				if (itFind != mLuaUIs.end())
				{
					// Platform dependent.
					if (MessageBox((HWND)Renderer::GetInstance().GetMainWindowHandle(), "Lua script has changed. Do you want save the current .ui and apply the script?",
						"Warning", MB_YESNO) == IDNO)
						return false;

					tinyxml2::XMLDocument doc;
					SaveUI(uiname.c_str(), doc);
					if (!itFind->second.empty()){
						doc.SaveFile(itFind->second[0]->GetUIFilePath());
					}
				}
			}

		}
		else if (extension==".ui")
		{
			uiname = FileSystem::GetName(lower.c_str());
		}
		else
			return false;

		if (uiname.empty())
		{
			LuaObject loadUIFunc;
			loadUIFunc.FindFunction(mL, "LoadLuaUI");
			loadUIFunc.PushToStack();
			auto uiFilePath = FileSystem::ReplaceExtension(file, "ui");
			if (FileSystem::Exists(uiFilePath.c_str()))
			{
				LuaUtils::pushstring(uiFilePath.c_str());
				loadUIFunc.CallWithManualArgs(1, 0);
				uiname = FileSystem::GetName(uiFilePath.c_str());
				SetVisible(uiname.c_str(), true);
			}
			else if (extension==".lua")
			{
				if (strstr(file, "save\\save") == 0 &&
					strstr(file, "configGame.lua") == 0 &&
					strstr(file, "configEngine.lua") == 0){
					int error = LuaUtils::DoFile(file);
					if (error)
					{
						char buf[1024];
						sprintf_s(buf, "\n%s/%s", FileSystem::GetCurrentDir(), LuaUtils::tostring(-1));
						Error(FB_ERROR_LOG_ARG, buf);
						assert(0);
					}
				}
			}
			return true;
		}

		HWindowId hwndId = -1;
		auto itFind = mLuaUIs.find(uiname);
		if (itFind != mLuaUIs.end())
		{
			if (GetVisible(uiname.c_str()))
			{
				SetVisible(uiname.c_str(), false);
			}

			auto& windows = itFind->second;
			if (!windows.empty())
			{
				hwndId = windows[0]->GetHwndId();
				while (!windows.empty()){
					DeleteWindow(windows.back());
					windows.pop_back();
				}
			}
			mLuaUIs.erase(itFind);
		}
		std::vector<WinBasePtr> temp;
		std::string name;
		ParseUI(filepath.c_str(), temp, name, hwndId, true);
		mLuaUIs[uiname] = temp;
		SetVisible(uiname.c_str(), false);
		SetVisible(uiname.c_str(), true); // for OnVisible UI Event.
		return true;
	}

	// IRenderListener
	void RenderUI(HWindowId hwndId, HWindow hwnd){
		if (!mUICommands->r_UI)
			return;
		auto uis = mRenderUIs.find(hwndId);
		for (auto& ui : uis->second){
			ui->PreRender();
			ui->Render();
		}
		if (hwndId == 1)
			mDragBox.Render();
		Renderer::GetInstance().ClearFontScissor();
	}

	void BeforeDebugHudRendering(){
		if (mUIEditor)
		{
			mUIEditor->DrawFocusBoxes();
		}
	}

	void AfterDebugHudRendered(){
		
	}

	void OnResolutionChanged(HWindowId hwndId, HWindow hwnd){
		auto it = mWindows.Find(hwndId);
		if (it != mWindows.end()){
			auto& windows = it->second;
			for (auto& it : windows){
				it->OnResolutionChanged(hwndId);
			}
		}
	}


	// IUIManager Interfaces
	void Update(float elapsedTime){
		for (auto & ui : mDeleteLuaUIPending){
			DeleteLuaUI(ui.c_str(), false);

		}
		mDeleteLuaUIPending.clear();
		for (auto& it : mSetFocusReserved)
		{
			auto ui = it.lock();
			if (!ui)
				continue;
			auto focusRoot = ui->GetRootWnd();
			auto hwndId = focusRoot->GetHwndId();
			auto& windows = mWindows[hwndId];
			WINDOWS::iterator f = std::find(windows.begin(), windows.end(), focusRoot);
			if (f != windows.end())
			{
				// insert f at the mWindows.end().
				windows.splice(windows.end(), windows, f);
			}
			if (!ui->IsAlwaysOnTop())
			{
				for (auto& winIt : mAlwaysOnTopWindows)
				{
					auto win = winIt.lock();
					if (!win || !win->GetVisible())
						continue;
					WINDOWS::iterator f = std::find(windows.begin(), windows.end(), win);
					if (f != windows.end())
					{
						// insert f at the mWindows.end().
						windows.splice(windows.end(), windows, f);
					}
				}
			}
			if (!ui->GetRender3D()){
				mNeedToRegisterUIObject[hwndId] = true;
			}
		}
		mSetFocusReserved.clear();

		for (auto& uiIt : mMoveToBottomReserved)
		{
			auto ui = uiIt.lock();
			if (!ui)
				continue;
			auto hwndId = ui->GetHwndId();
			auto& windows = mWindows[hwndId];
			WINDOWS::iterator f = std::find(windows.begin(), windows.end(), ui);
			if (f != windows.end()){
				// insert f at the mWindows.begin().
				if (windows.begin() != f){
					windows.splice(windows.begin(), windows, f);
					mNeedToRegisterUIObject[hwndId] = true;
				}
			}
		}
		mMoveToBottomReserved.clear();

		for (auto& uiIt : mMoveToTopReserved)
		{
			auto ui = uiIt.lock();
			if (!ui)
				continue;
			auto hwndId = ui->GetHwndId();
			auto& windows = mWindows[hwndId];
			WINDOWS::iterator f = std::find(windows.begin(), windows.end(), ui);
			if (f != windows.end()){
				// insert f at the mWindows.begin().
				auto lastIt = windows.end();
				lastIt--;
				if (lastIt != f){
					windows.splice(windows.end(), windows, f);
					mNeedToRegisterUIObject[hwndId] = true;
				}
			}
		}
		mMoveToTopReserved.clear();

		for(auto& it : mWindows){
			auto& windows = it.second;
			for (auto& wnd : windows){
				if (wnd->GetVisible())
					wnd->OnStartUpdate(elapsedTime);
			}
		}

		if (!mTooltipText.empty()){
			mDelayForTooltip -= elapsedTime;
			if (mDelayForTooltip <= 0 && !mTooltipUI->GetVisible()){
				ShowTooltip();
			}
		}
	}

	void GatherRenderList(){
		for (auto& it : mNeedToRegisterUIObject){
			if (it.second){

				HWindowId hwndId = it.first;
				auto& uiObjects = mRenderUIs[it.first];				
				uiObjects.clear();
				//std::map<std::string, std::vector<UIObject*>> render3DList;
				it.second = false;
				{
					auto& windows = mWindows[hwndId];					
					bool hideAll = !mHideUIExcepts.empty();
					WINDOWS::iterator it = windows.begin(), itEnd = windows.end();
					size_t start = 0;
					for (; it != itEnd; it++)
					{
						if (hideAll)
						{
							if (std::find(mHideUIExcepts.begin(), mHideUIExcepts.end(), (*it)->GetName()) == mHideUIExcepts.end())
								continue;
						}
						if ((*it)->GetVisible())
						{
							if ((*it)->GetRender3D())
							{
								/*assert(strlen((*it)->GetName()) != 0);
								auto& list = render3DList[(*it)->GetName()];
								list.reserve(100);
								(*it)->GatherVisit(list);
								std::stable_sort(uiObjects.begin(), uiObjects.end(), [](IUIObject* a, IUIObject* b){
									return a->GetSpecialOrder() < b->GetSpecialOrder();
								});*/
							}
							else
							{
								(*it)->GatherVisit(uiObjects);

								std::stable_sort(uiObjects.begin() + start, uiObjects.end(), [](UIObject* a, UIObject* b){
									return a->GetSpecialOrder() < b->GetSpecialOrder();
								});
								start = uiObjects.size();
							}
						}
					}


					if (mPopup&& mPopup->GetVisible())
						mPopup->GatherVisit(uiObjects);
				}
				/*for (auto it : render3DList)
				{
					gFBEnv->pRenderer->Register3DUIs(hwndId, it.first.c_str(), it.second);
				}*/
			}
		}
	}

	bool ParseUI(const char* filepath, WinBases& windows, std::string& uiname, HWindowId hwndId = INVALID_HWND_ID, bool luaUI = false){
		LUA_STACK_CLIPPER c(mL);
		if (hwndId == INVALID_HWND_ID)
		{
			hwndId = Renderer::GetInstance().GetMainWindowHandleId();
		}
		tinyxml2::XMLDocument doc;
		int err = doc.LoadFile(filepath);
		char buf[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, buf);
		if (err)
		{
			Error("parsing ui file(%s) failed.", filepath);
			if (doc.GetErrorStr1())
				Error(doc.GetErrorStr1());
			if (doc.GetErrorStr2())
				Error(doc.GetErrorStr2());
			return false;
		}

		tinyxml2::XMLElement* pRoot = doc.RootElement();
		if (!pRoot)
		{
			assert(0);
			return false;
		}

		if (pRoot->Attribute("name"))
			uiname = pRoot->Attribute("name");

		std::string lowerUIname = uiname;
		ToLowerCase(lowerUIname);

		auto itFind = mLuaUIs.find(lowerUIname);
		if (itFind != mLuaUIs.end())
		{
			if (GetVisible(lowerUIname.c_str()))
			{
				SetVisible(lowerUIname.c_str(), false);
			}

			for (const auto& ui : itFind->second)
			{
				DeleteWindow(ui);
			}
			mLuaUIs.erase(itFind);
		}

		std::string scriptPath;
		const char* sz = pRoot->Attribute("script");
		if (sz && strlen(sz) != 0)
		{
			if (LuaUtils::DoFile(sz))
			{
				return false;				
			}
			scriptPath = sz;
			ToLowerCase(scriptPath);
		}

		bool render3d = false;
		sz = pRoot->Attribute("render3d");
		if (sz)
		{
			render3d = StringConverter::ParseBool(sz);
		}
		Vec2I renderTargetSize(100, 100);
		sz = pRoot->Attribute("renderTargetSize");
		if (sz)
		{
			renderTargetSize = StringMathConverter::ParseVec2I(sz);
		}

		tinyxml2::XMLElement* pComp = pRoot->FirstChildElement("component");
		while (pComp)
		{
			sz = pComp->Attribute("type");
			if (!sz)
			{
				Error("Component doesn't have type attribute. ignored");
				assert(0);
				continue;
			}

			ComponentType::Enum type = ComponentType::ConvertToEnum(sz);

			WinBasePtr p = AddWindow(type, hwndId);
			if (p)
			{
				if (render3d)
				{
					p->SetRender3D(true, renderTargetSize);
				}
				p->SetUIFilePath(filepath);
				if (!scriptPath.empty())
				{
					p->SetScriptPath(scriptPath.c_str());
					scriptPath.clear();
				}
				windows.push_back(p);
				p->ParseXML(pComp);
			}

			pComp = pComp->NextSiblingElement("component");
		}

		auto animElem = pRoot->FirstChildElement("Animation");
		while (animElem)
		{
			auto pAnim = UIAnimation::Create();
			pAnim->LoadFromXML(animElem);
			std::string name = pAnim->GetName();
			auto it = mAnimations.Find(name);
			if (it != mAnimations.end())
			{
				Log("UI global animation %s is replaced", name.c_str());
				mAnimations.erase(it);				
			}
			mAnimations[pAnim->GetName()] = pAnim;

			animElem = animElem->NextSiblingElement("Animation");
		}

		assert(!uiname.empty());
		if (luaUI && !uiname.empty())
		{
			mLuaUIs[lowerUIname].clear();
			for (const auto& topWindow : windows)
			{
				mLuaUIs[lowerUIname].push_back(topWindow);
			}
		}
		else{
			mCppUIs[lowerUIname].clear();
			for (const auto& topWindow : windows)
			{
				mCppUIs[lowerUIname].push_back(topWindow);
			}
		}

		for (const auto& topWindow : windows)
		{
			auto eventHandler = std::dynamic_pointer_cast<EventHandler>(topWindow);
			if (eventHandler)
				eventHandler->OnEvent(UIEvents::EVENT_ON_LOADED);
		}

		return true;
	}

	bool SaveUI(const char* uiname, tinyxml2::XMLDocument& doc){
		std::string lower = uiname;
		ToLowerCase(lower);
		auto it = mLuaUIs.find(lower);
		WinBases* topWindows = 0;
		if (it != mLuaUIs.end()) {
			topWindows = &it->second;
		}
		else{
			auto it = mCppUIs.find(lower);
			if (it != mCppUIs.end())
				topWindows = &it->second;
		}
		if (!topWindows){
			Error(FB_ERROR_LOG_ARG, FormatString("no ui found with the name %s", uiname));
			return false;
		}
		if (topWindows->empty())
		{
			Error(FB_ERROR_LOG_ARG, "doesn't have any window!");
			return false;
		}
		if (topWindows->size() != 1)
		{
			Error(FB_ERROR_LOG_ARG, "Only supporting an window per ui.");
			return false;
		}
		auto window = (*topWindows)[0];
		tinyxml2::XMLElement* root = doc.NewElement("UI");
		doc.InsertEndChild(root);
		if (!root)
		{
			assert(0);
			return false;
		}

		root->SetAttribute("name", uiname);
		root->SetAttribute("script", window->GetScriptPath());


		tinyxml2::XMLElement* pComp = doc.NewElement("component");
		root->InsertEndChild(pComp);
		window->Save(*pComp);
		return true;
	}

	bool AddLuaUI(const char* uiName, LuaObject& data, HWindowId hwndId = INVALID_HWND_ID){
		if (hwndId == INVALID_HWND_ID)
		{
			hwndId = Renderer::GetInstance().GetMainWindowHandleId();
		}
		std::string lower = uiName;
		ToLowerCase(lower);
		auto it = mLuaUIs.find(lower.c_str());
		if (it != mLuaUIs.end())
		{
			Error("Already registered!");
			return false;
		}
		std::string typeText = data.GetField("type_").GetString();
		auto type = ComponentType::ConvertToEnum(typeText.c_str());

		WinBasePtr p = AddWindow(0.f, 0.f, 0.1f, 0.1f, type, hwndId);
		assert(p);
		p->ParseLua(data);

		mLuaUIs[lower].push_back(p);
		return p != 0;
	}

	void DeleteLuaUI(const char* uiName, bool pending){
		if (pending)
		{
			mDeleteLuaUIPending.push_back(uiName);
		}
		std::string lower = uiName;
		ToLowerCase(lower);
		auto it = mLuaUIs.find(lower);
		if (it != mLuaUIs.end())
		{
			for (auto& win : it->second)
			{
				auto eventHandler = std::dynamic_pointer_cast<EventHandler>(win);
				if (eventHandler)
					eventHandler->OnEvent(UIEvents::EVENT_ON_UNLOADED);
				DirtyRenderList(win->GetHwndId());
				DeleteWindow(win);
			}
			mLuaUIs.erase(it);
		}
	}

	bool IsLoadedUI(const char* uiName){
		std::string lower = uiName;
		ToLowerCase(lower);
		auto it = mLuaUIs.find(lower);
		return it != mLuaUIs.end();
	}


	WinBasePtr AddWindow(int posX, int posY, int width, int height, ComponentType::Enum type, HWindowId hwndId = INVALID_HWND_ID){
		assert(hwndId != 0);
		if (hwndId == INVALID_HWND_ID)
		{
			hwndId = Renderer::GetInstance().GetMainWindowHandleId();
		}

		WinBasePtr pWnd = CreateComponent(type);
		if (pWnd != 0)
		{
			pWnd->SetHwndId(hwndId);
			auto& windows = mWindows[hwndId];
			windows.push_back(pWnd);
			pWnd->ChangeSize(Vec2I(width, height));
			pWnd->ChangePos(Vec2I(posX, posY));
			mNeedToRegisterUIObject[hwndId] = true;
			pWnd->OnCreated();
		}

		return pWnd;
	}

	WinBasePtr AddWindow(const Vec2I& pos, const Vec2I& size, ComponentType::Enum type, HWindowId hwndId = INVALID_HWND_ID){
		return AddWindow(pos.x, pos.y, size.x, size.y, type, hwndId);
	}

	WinBasePtr AddWindow(float posX, float posY, float width, float height, ComponentType::Enum type, HWindowId hwndId = INVALID_HWND_ID){
		assert(hwndId != 0);
		if (hwndId == INVALID_HWND_ID)
		{
			hwndId = Renderer::GetInstance().GetMainWindowHandleId();
		}
		WinBasePtr pWnd = CreateComponent(type);
		if (pWnd != 0)
		{
			pWnd->SetHwndId(hwndId);
			auto& windows = mWindows[hwndId];
			windows.push_back(pWnd);
			pWnd->ChangeNSize(Vec2(width, height));
			pWnd->ChangeNPos(Vec2(posX, posY));
			mNeedToRegisterUIObject[hwndId] = true;
			pWnd->OnCreated();
		}
		return pWnd;
	}

	WinBasePtr AddWindow(ComponentType::Enum type, HWindowId hwndId = INVALID_HWND_ID){
		assert(hwndId != 0);
		if (hwndId == INVALID_HWND_ID)
		{
			hwndId = Renderer::GetInstance().GetMainWindowHandleId();
		}

		WinBasePtr pWnd = CreateComponent(type);

		if (pWnd != 0)
		{
			pWnd->SetHwndId(hwndId);
			auto& windows = mWindows[hwndId];
			windows.push_back(pWnd);
			mNeedToRegisterUIObject[hwndId] = true;
			pWnd->OnCreated();
		}

		return pWnd;
	}


	void DeleteWindow(WinBasePtr pWnd){
		if (!pWnd)
			return;
		if (pWnd == mKeyboardFocus.lock())
		{
			mKeyboardFocus.reset();
		}
		if (pWnd == mModalWindow.lock()){
			mModalWindow.reset();
		}		
		auto hwndId = pWnd->GetHwndId();
		assert(hwndId != -1);
		auto& windows = mWindows[hwndId];
		windows.erase(std::remove(windows.begin(), windows.end(), pWnd), windows.end());
		DirtyRenderList(hwndId);
	}

	void DeleteWindowsFor(HWindowId hwndId){
		auto it = mWindows.Find(hwndId);
		if (it == mWindows.end())
			return;
		auto& windows = it->second;
		for (auto wnd : windows)
		{
			DeleteLuaUIContaning(wnd);
		}
		windows.clear();
		mWindows.erase(it);
		DirtyRenderList(hwndId);
	}

	void SetFocusUI(WinBasePtr ui){
		if (mLockFocus)
			return;
		if (mKeyboardFocus == ui)
			return;
		for (auto& reservedUI : mSetFocusReserved)
		{
			if (ui == reservedUI.lock())
				return;
		}
		mNewFocusWnd = ui;

		auto keyboardFocus = mKeyboardFocus.lock();
		if (keyboardFocus)
			keyboardFocus->OnFocusLost();
		auto focus = mFocusWnd.lock();
		if (focus)
			focus->OnFocusLost();
		auto dropDown = DropDown::sCurrentDropDown.lock();
		if (dropDown){
			dropDown->OnFocusLost();
		}

		auto focusRoot = ui ? ui->GetRootWnd() : 0;

		mFocusWnd = focusRoot;
		mKeyboardFocus = ui;

		focus = mFocusWnd.lock();
		if (focus)
			focus->OnFocusGain();
		keyboardFocus = mKeyboardFocus.lock();
		if (keyboardFocus)
			keyboardFocus->OnFocusGain();
		if (ui && !ValueExistsInVector(mSetFocusReserved, ui))
			mSetFocusReserved.push_back(ui);
	}

	WinBasePtr GetFocusUI() const{
		return mFocusWnd.lock();
	}

	WinBasePtr GetKeyboardFocusUI() const{
		return mKeyboardFocus.lock();
	}

	WinBasePtr GetNewFocusUI() const { 
		return mNewFocusWnd.lock(); 
	}

	void SetFocusUI(const char* uiName){
		if (!uiName)
			return;
		std::string lower(uiName);
		ToLowerCase(lower);
		auto it = mLuaUIs.find(lower);
		if (it == mLuaUIs.end())
		{
			Error("Cannot find ui with a name, %s", uiName);
			return;
		}
		if (it->second.empty())
		{
			Error("UI doesn't have any elements, %s", uiName);
			return;
		}

		for (auto& ui : it->second)
		{
			SetFocusUI(ui);
			break;
		}
	}

	bool IsFocused(const WinBasePtr pWnd) const{
		return pWnd == mFocusWnd.lock() || pWnd == mKeyboardFocus.lock();
	}

	void DirtyRenderList(HWindowId hwndId){
		if (hwndId == INVALID_HWND_ID){
			for (auto& it : mNeedToRegisterUIObject)
			{
				it.second = true;
			}
		}
		else{
			mNeedToRegisterUIObject[hwndId] = true;
		}
	}


	void SetUIProperty(const char* uiname, const char* compname, const char* prop, const char* val, bool updatePosSize = false){
		SetUIProperty(uiname, compname, UIProperty::ConvertToEnum(prop), val, updatePosSize);
	}

	void SetUIProperty(const char* uiname, const char* compname, UIProperty::Enum prop, const char* val, bool updatePosSize = false){
		if_assert_fail(uiname && compname && val)
			return;

		auto comp = FindComp(uiname, compname);
		if (comp)
		{
			comp->SetProperty(prop, val);
			if (updatePosSize){
				comp->OnSizeChanged();
				comp->OnPosChanged(false);
			}
		}
		else
		{
			Error("Cannot find ui comp(%s) in ui(%s) to set uiproperty(%s).", compname, uiname, UIProperty::ConvertToString(prop));
		}
	}

	void SetEnableComponent(const char* uiname, const char* compname, bool enable){
		auto comp = FindComp(uiname, compname);
		if (comp)
		{
			comp->SetEnable(enable);
		}
		else
		{
			Error("Cannot find ui comp(%s) in ui(%s) to set enable flag.", compname, uiname);
		}		
	}

	HWindowId GetForegroundWindowId(){
		return Renderer::GetInstance().GetWindowHandleId(ForegroundWindow());
	}

	bool IsMainWindowForeground(){
		return IsWindowForeground(Renderer::GetInstance().GetMainWindowHandle());
	}

	// IInputListener Interfaces
	void ConsumeInput(IInputInjectorPtr injector){
		/*if (gFBEnv->pConsole->GetEngineCommand()->UI_Debug){
			DebugUI();
		}*/
		if (!injector->IsValid(InputDevice::Keyboard) &&
			!injector->IsValid(InputDevice::Mouse)){
			return;
		}		
		auto& renderer = Renderer::GetInstance();
		bool mainWindowForeground = IsMainWindowForeground();
		if (mUIEditor && mainWindowForeground)
		{
			if (mLocatingComp != ComponentType::NUM)
			{
				if (mMultiLocating  && mLocatingComp != ComponentType::NUM && !injector->IsKeyDown(VK_SHIFT))
				{
					mLocatingComp = ComponentType::NUM;
					mUIEditor->OnCancelComponent();
					mMultiLocating = false;
				}
				else{
					OnInputForLocating(injector);

					return;
				}
			}
		}

		auto hwndId = GetForegroundWindowId();
		auto windows = mWindows[hwndId];

		if (injector->IsValid(InputDevice::Keyboard) && injector->IsKeyPressed(VK_ESCAPE)) {
			WINDOWS::reverse_iterator it = windows.rbegin(), itEnd = windows.rend();
			int i = 0;
			for (; it != itEnd; ++it) {
				if ((*it)->GetVisible() && (*it)->GetCloseByEsc()) {
					(*it)->SetVisible(false);
					injector->Invalidate(InputDevice::Keyboard);
					break;
				}
			}
		}

		if (mUIEditor){
			if (mainWindowForeground) {
				if (injector->IsValid(InputDevice::Keyboard) && injector->IsKeyPressed(VK_DELETE)) {
					mUIEditor->TryToDeleteCurComp();
					injector->Invalidate(InputDevice::Keyboard);
				}
				DragUI();
				mUIEditor->ProcessKeyInput();
			}
		}

		int x, y;
		if (injector->IsRDragStarted(x, y)){
			mMouseIn = false;
			return;
		}

		mMouseIn = false;
		auto keyboardFocus = mKeyboardFocus.lock();
		if (keyboardFocus && keyboardFocus->GetHwndId() != hwndId){
			injector->InvalidTemporary(InputDevice::Mouse, true);
			keyboardFocus->OnInputFromHandler(injector);
			injector->InvalidTemporary(InputDevice::Mouse, false);
		}

		ProcessMouseInput(injector);

		if (mIgnoreInput && !mUIEditor) {
			return;
		}

		windows = mWindows[hwndId];
		WINDOWS::reverse_iterator it = windows.rbegin();
		int i = 0;
		for (; it != windows.rend(); ++it)
		{
			if ((*it)->GetVisible() && !(*it)->GetVisualOnly())
			{
				mMouseIn = (*it)->OnInputFromHandler(injector) || mMouseIn;
			}

			if (!injector->IsValid(InputDevice::Mouse) && !injector->IsValid(InputDevice::Keyboard))
				break;
		}

		if (injector->IsValid(InputDevice::Keyboard) && injector->GetChar() == VK_TAB)
		{
			if (UIManager::GetInstance().GetKeyboardFocusUI())
			{
				injector->PopChar();
				UIManager::GetInstance().GetKeyboardFocusUI()->TabPressed();
			}
		}


		if (mMouseIn && EventHandler::sLastEventProcess != gpTimer->GetFrame() && injector->IsLButtonClicked())
		{
			LuaObject mouseInvalided;
			mouseInvalided.FindFunction(mL, "OnMouseInvalidatedInUI");
			if (mouseInvalided.IsValid())
			{
				mouseInvalided.Call();
			}
		}
	}

	void ProcessMouseInput(IInputInjectorPtr injector){
		auto hwndId = GetForegroundWindowId();
		bool isMainForeground = IsMainWindowForeground();
		//Select
		if (injector->IsValid(InputDevice::Mouse)) {
			auto mousePos = injector->GetMousePos();
			RegionTestParam rparam;
			rparam.mOnlyContainer = false;
			rparam.mIgnoreScissor = true; // only for FBUIEditor.
			rparam.mTestChildren = true;
			rparam.mNoRuntimeComp = mUIEditor && isMainForeground && !injector->IsKeyDown(VK_LMENU) ? true : false;
			rparam.mCheckMouseEvent = !mUIEditor || injector->IsKeyDown(VK_LMENU) || !isMainForeground;
			rparam.mHwndId = hwndId;
			auto modalWindow = mModalWindow.lock();
			rparam.mRestrictToThisWnd = modalWindow && modalWindow->GetVisible() ? modalWindow : 0;

			if (mUIEditor && injector->IsLButtonClicked() && injector->IsKeyDown(VK_CONTROL)){
				auto it = mUIEditor->GetSelectedComps();
				std::vector<WinBasePtr> del;
				while (it.HasMoreElement()){
					auto comp = it.GetNext();
					if (comp->IsIn(mousePos, rparam.mIgnoreScissor)){
						del.push_back(comp);
					}
				}
				for (auto comp : del){
					mUIEditor->OnComponentDeselected(comp);
				}
				return;
			}


			if (mUIEditor && injector->IsLButtonClicked() && !injector->IsKeyDown(VK_MENU)){
				auto it = mUIEditor->GetSelectedComps();
				while (it.HasMoreElement()){
					rparam.mExceptions.push_back(it.GetNext());
				}
			}
			auto focusWnd = WinBaseWithPointCheckAlways(mousePos, rparam);
			if (!focusWnd){
				rparam.mIgnoreScissor = false;
				focusWnd = WinBaseWithPoint(mousePos, rparam);
			}
			auto mouseOvered = mMouseOvered.lock();
			if (focusWnd != mouseOvered){
				if (mouseOvered){
					mouseOvered->OnMouseOut(injector);
				}
				mMouseOvered = focusWnd;
				mouseOvered = mMouseOvered.lock();
				if (mouseOvered){
					mouseOvered->OnMouseIn(injector);
				}
			}
			else if (focusWnd){
				//focusWnd == mMouseOvered
				if (mouseOvered)
					mouseOvered->OnMouseHover(injector);
			}

			auto dragStartedUI = mMouseDragStartedUI.lock();
			if (focusWnd && injector->IsLButtonDown() && !dragStartedUI){
				mMouseDragStartedUI = focusWnd;
			}
			if (injector->IsLButtonDown() && focusWnd){
				focusWnd->OnMouseDown(injector);
			}
			dragStartedUI = mMouseDragStartedUI.lock();
			if (dragStartedUI){
				dragStartedUI->OnMouseDrag(injector);
			}

			if (injector->IsLButtonClicked()){
				auto keyboardFocus = mKeyboardFocus.lock();
				if (keyboardFocus != focusWnd)
				{
					if (mUIEditor || !focusWnd || (!focusWnd->GetNoMouseEventAlone() && !focusWnd->GetNoFocusByClick()))
						SetFocusUI(focusWnd);
				}
				keyboardFocus = mKeyboardFocus.lock();
				bool editorFocused = !isMainForeground;
				if (mUIEditor && (!injector->IsKeyDown(VK_MENU) && !editorFocused))
				{
					if (mUIEditor->GetCurSelected() != keyboardFocus || injector->IsKeyDown(VK_SHIFT) || mUIEditor->GetNumCurEditing() > 1)
						mUIEditor->OnComponentSelected(keyboardFocus);
				}
				else{
					auto mouseOvered = mMouseOvered.lock();
					if (mouseOvered){
						mouseOvered->OnMouseClicked(injector);
					}
				}
			}
			else if (injector->IsLButtonDoubleClicked()){
				auto mouseOvered = mMouseOvered.lock();
				if (mouseOvered){
					mouseOvered->OnMouseDoubleClicked(injector);
				}
			}
			else if (injector->IsRButtonClicked()){
				auto mouseOvered = mMouseOvered.lock();
				if (mouseOvered){
					mouseOvered->OnMouseRButtonClicked(injector);
				}
			}

			if (!injector->IsLButtonDown()){
				mMouseDragStartedUI.reset();
			}
		}
	}

	void EnableInputListener(bool enable){
		mInputListenerEnable = enable;
	}

	bool IsEnabledInputLIstener() const{
		return mInputListenerEnable;
	}

	HCURSOR GetMouseCursorOver() const{
		return WinBase::GetMouseCursorOver();
	}

	void SetMouseCursorOver(){
		SetCursor(WinBase::GetMouseCursorOver());
	}

	void DisplayMsg(const std::string& msg, ...){
		char buf[2048] = { 0 };
		va_list args;
		va_start(args, msg);
		vsprintf_s(buf, 2048, msg.c_str(), args);
		va_end(args);

		if (strlen(buf)>0)
		{
			Log(buf);
			Renderer::GetInstance().QueueDrawTextForDuration(4.0f, Vec2I(100, 200),
					buf, fb::Color::White);			
		}
	}

	bool IsMouseInUI() const { 
		return mMouseIn; 
	}


	void SetTooltipString(const std::wstring& ts){
		if (mTooltipText == ts)
		{
			return;
		}
		mTooltipText = ts;
		HWindowId hwndId = Renderer::GetInstance().GetWindowHandleId(WindowFromMousePosition());
		if (hwndId != INVALID_HWND_ID)
			mTooltipUI->SetHwndId(GetForegroundWindowId());

		if (mTooltipText.empty())
		{
			mTooltipUI->SetVisible(false);
		}
		else
		{
			if (mTooltipUI->GetVisible())
			{
				ShowTooltip();
			}
			else
			{
				mDelayForTooltip = gDelayForTooltip;
			}
		}
	}

	void SetTooltipPos(const Vec2& npos, bool checkNewPos = true){
		//if (!mTooltipUI->GetVisible())
		//		return;

		if (checkNewPos && mPrevTooltipNPos == npos)
			return;
		mPrevTooltipNPos = npos;
		HWindowId hwndId = mTooltipUI->GetHwndId();
		auto& renderer = Renderer::GetInstance();
		auto hWnd = renderer.GetWindowHandle(hwndId);
		if (hwndId == -1){
			hwndId = GetForegroundWindowId();
			mTooltipUI->SetHwndId(hwndId);
		}
		assert(hwndId != -1);
		const auto& size = renderer.GetRenderTargetSize(hwndId);
		Vec2 backPos = npos;
		if (backPos.y > 0.9f)
		{
			backPos.y -= (gTooltipFontSize * 2.0f + 10) / (float)size.y;
		}
		const Vec2& nSize = mTooltipUI->GetNSize();
		if (backPos.x + nSize.x>1.0f)
		{
			float mod = backPos.x + nSize.x - 1.0f + (4.0f / size.x);
			backPos.x -= mod;
		}
		backPos.y += gTooltipFontSize / (float)size.y;
		mTooltipUI->ChangeNPos(backPos);
	}

	void CleanTooltip(){
		if (!mTooltipText.empty())
		{
			mTooltipText.clear();
			mTooltipUI->SetVisible(false);
		}
	}


	void PopupDialog(WCHAR* msg, POPUP_TYPE type, std::function< void(void*) > func){
		if (!mPopup)
		{
			mPopup = CreateComponent(ComponentType::Window);
			mPopup->SetHwndId(Renderer::GetInstance().GetMainWindowHandleId());
			mPopup->ChangeNPos(Vec2(0.5f, 0.5f));
			mPopup->ChangeNSize(Vec2(0.3f, 0.1f));
			mPopup->SetProperty(UIProperty::ALIGNH, "center");
			mPopup->SetProperty(UIProperty::ALIGNV, "middel");
			mPopup->SetProperty(UIProperty::TEXT_ALIGN, "center");
			mPopup->SetProperty(UIProperty::TEXT_VALIGN, "middle");
			mPopup->SetText(msg);

			auto yes = mPopup->AddChild(0.49f, 0.99f, 0.25f, 0.1f, ComponentType::Button);
			yes->SetRuntimeChild(true);
			yes->SetName("yes");
			yes->SetAlign(ALIGNH::RIGHT, ALIGNV::BOTTOM);
			yes->SetText(L"Yes");
			yes->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
				std::bind(&Impl::OnPopupYes, this, std::placeholders::_1));

			auto no = mPopup->AddChild(0.51f, 0.99f, 0.25f, 0.1f, ComponentType::Button);
			no->SetRuntimeChild(true);
			no->SetName("no");
			no->SetAlign(ALIGNH::LEFT, ALIGNV::BOTTOM);
			no->SetText(L"No");
			no->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
				std::bind(&Impl::OnPopupNo, this, std::placeholders::_1));
		}

mPopupCallback = func;
mPopup->SetVisible(true);
	}

	int GetPopUpResult() const{
		return mPopupResult;
	}

	lua_State* GetLuaState() const {
		return mL;
	}

	WinBasePtr FindComp(const char* uiname, const char* compName) const{
		assert(uiname);
		std::string lower(uiname);
		ToLowerCase(lower);
		auto itFind = mLuaUIs.find(lower);
		if (itFind == mLuaUIs.end())
			return 0;

		if ((compName == 0 || strlen(compName) == 0) && !itFind->second.empty())
			return itFind->second[0];

		for (const auto& comp : itFind->second)
		{
			if (strcmp(comp->GetName(), compName) == 0)
			{
				return comp;
			}

			auto ret = comp->GetChild(compName, true);
			if (ret)
				return ret;
		}

		return 0;
	}

	void FindUIWnds(const char* uiname, WinBases& outV) const{
		assert(uiname);
		std::string lower(uiname);
		ToLowerCase(lower);
		auto itFind = mLuaUIs.find(lower);
		if (itFind == mLuaUIs.end())
			return;

		for (const auto& comp : itFind->second)
		{
			if (strcmp(comp->GetName(), uiname) == 0)
			{
				outV.push_back(comp);
			}
		}
	}

	bool CacheListBox(const char* uiname, const char* compName){
		mCachedListBox = std::dynamic_pointer_cast<ListBox>(FindComp(uiname, compName));
		return !mCachedListBox.expired();
	}

	ListBoxPtr GetCachedListBox() const{
		return mCachedListBox.lock();
	}

	void SetEnablePosSizeEvent(bool enable) {
		mPosSizeEventEnabled = enable;
	}

	bool GetEnablePosSizeEvent() const {
		return mPosSizeEventEnabled;
	}

	void SetVisible(const char* uiname, bool visible){
		assert(uiname);
		std::string lower(uiname);
		ToLowerCase(lower);
		auto itFind = mLuaUIs.find(lower);
		WinBases* windows = 0;
		if (itFind != mLuaUIs.end())
		{
			windows = &itFind->second;
		}
		else{
			auto itFind = mCppUIs.find(lower);
			if (itFind != mCppUIs.end()){
				windows = &itFind->second;
			}
		}
		if (!windows){
			Log("UIManager::SetVisible(): UI(%s) is not found.", uiname);
			return;
		}

		bool affected = false;
		for (auto it : *windows){
			affected = it->SetVisible(visible) || affected;
		}

/*
		if (affected){
			if (visible){
				char buffer[512];
				std::string openSound = mSounds[UISounds::WindowOpen];
				if ((*windows->begin())->GetProperty(UIProperty::OPEN_SOUND, buffer, 512, false)){
					bool play = true;
					if (strlen(buffer))
						openSound = buffer;
					else if (!mProcessingButtonClick)
						play = false;
					if (_stricmp(buffer, "None") == 0)
						openSound.clear();

					if (play && !openSound.empty()){
						AudioManager::GetInstance().PlayAudio(openSound.c_str());
					}
				}
			}
			else{
				char buffer[512];
				std::string closeSound = mSounds[UISounds::WindowClose];
				if ((*windows->begin())->GetProperty(UIProperty::CLOSE_SOUND, buffer, 512, false)){
					bool play = true;
					if (strlen(buffer))
						closeSound = buffer;
					else if (!mProcessingButtonClick)
						play = false;
					if (_stricmp(buffer, "None") == 0)
						closeSound.clear();

					if (play && !closeSound.empty()){
						AudioManager::GetInstance().PlayAudio(closeSound.c_str());
					}
				}				
			}
		}*/
	}

	void LockFocus(bool lock){
		mLockFocus = lock;
	}

	bool GetVisible(const char* uiname) const{
		assert(uiname);
		std::string lower(uiname);
		ToLowerCase(lower);
		auto itFind = mLuaUIs.find(lower.c_str());
		if (itFind == mLuaUIs.end())
		{
			return false;
		}
		bool visible = false;
		for (const auto& comp : itFind->second)
		{
			visible = visible || comp->GetVisible();
		}
		return visible;
	}

	void CloseAllLuaUI(){
		for (auto& it : mLuaUIs)
		{
			for (auto& ui : it.second)
			{
				ui->SetVisible(false);
			}
		}
	}


	void CloneUI(const char* uiname, const char* newUIname){
		assert(uiname);
		assert(newUIname);
		std::string lower = uiname;
		ToLowerCase(lower);
		auto it = mLuaUIs.find(lower);
		if (it == mLuaUIs.end())
		{
			assert(0);
			return;
		}

		std::string strNewUIName = newUIname;
		ToLowerCase(strNewUIName);
		auto newIt = mLuaUIs.find(strNewUIName);
		if (newIt != mLuaUIs.end())
		{
			assert(0 && "already have the ui with the new name");
			return;
		}
		assert(!it->second.empty());
		const char* filepath = it->second[0]->GetUIFilePath();
		HWindowId hwndId = it->second[0]->GetHwndId();
		LUA_STACK_CLIPPER c(mL);
		tinyxml2::XMLDocument doc;
		int err = doc.LoadFile(filepath);
		if (err)
		{
			Error("parsing ui file(%s) failed.", filepath);
			if (doc.GetErrorStr1())
				Error(doc.GetErrorStr1());
			if (doc.GetErrorStr2())
				Error(doc.GetErrorStr2());
			return;
		}

		tinyxml2::XMLElement* pRoot = doc.RootElement();
		if (!pRoot)
		{
			assert(0);
			return;
		}

		std::string scriptPath;
		const char* sz = pRoot->Attribute("script");
		if (sz && strlen(sz)>0)
		{
			Error(FB_ERROR_LOG_ARG, "cannot clone scriptable ui");
			return;
		}

		bool render3d = false;
		sz = pRoot->Attribute("render3d");
		if (sz)
		{
			render3d = StringConverter::ParseBool(sz);
		}
		Vec2I renderTargetSize(100, 100);
		sz = pRoot->Attribute("renderTargetSize");
		if (sz)
		{
			renderTargetSize = StringMathConverter::ParseVec2I(sz);
		}

		tinyxml2::XMLElement* pComp = pRoot->FirstChildElement("component");
		unsigned idx = 0;
		WinBases windows;
		while (pComp)
		{
			sz = pComp->Attribute("type");
			if (!sz)
			{
				Error("Component doesn't have type attribute. ignored");
				assert(0);
				continue;
			}

			ComponentType::Enum type = ComponentType::ConvertToEnum(sz);

			WinBasePtr p = AddWindow(type, hwndId);
			if (p)
			{
				if (render3d)
				{
					p->SetRender3D(true, renderTargetSize);
				}
				p->SetUIFilePath(filepath);
				if (!scriptPath.empty())
				{
					p->SetScriptPath(scriptPath.c_str());
					scriptPath.clear();
				}
				p->ParseXML(pComp);
				windows.push_back(p);
				if (idx == 0)
				{
					p->SetName(newUIname);
				}
				idx++;
			}

			pComp = pComp->NextSiblingElement("component");
		}

		auto& uis = mLuaUIs[strNewUIName];
		for (const auto& topWindow : windows)
		{
			uis.push_back(topWindow);
		}

		for (const auto& topWindow : windows)
		{
			auto eventHandler = std::dynamic_pointer_cast<EventHandler>(topWindow);
			if (eventHandler)
				eventHandler->OnEvent(UIEvents::EVENT_ON_LOADED);
		}
	}

	void IgnoreInput(bool ignore, WinBasePtr modalWindow){
		ignore ? mIgnoreInput++ : mIgnoreInput--;
		mModalWindow = modalWindow;
	}

	void ToggleVisibleLuaUI(const char* uiname){
		bool visible = GetVisible(uiname);
		visible = !visible;
		SetVisible(uiname, visible);
	}

	void RegisterAlwaysOnTopWnd(WinBasePtr win){
		if (!ValueExistsInVector(mAlwaysOnTopWindows, win))
			mAlwaysOnTopWindows.push_back(win);
	}

	void UnRegisterAlwaysOnTopWnd(WinBasePtr win){
		DeleteValuesInVector(mAlwaysOnTopWindows, win);
	}


	void MoveToBottom(const char* moveToBottom){
		auto ui = FindComp(moveToBottom, 0);
		if (ui)
		{
			if (!ValueExistsInVector(mMoveToBottomReserved, ui))
				mMoveToBottomReserved.push_back(ui);
			DeleteValuesInVector(mMoveToTopReserved, ui);
		}
	}

	void MoveToBottom(WinBasePtr moveToBottom){
		if (moveToBottom){
			if (!ValueExistsInVector(mMoveToBottomReserved, moveToBottom))
				mMoveToBottomReserved.push_back(moveToBottom);
			DeleteValuesInVector(mMoveToTopReserved, moveToBottom);
		}
	}

	void MoveToTop(WinBasePtr moveToTop){
		if (moveToTop){
			if (!ValueExistsInVector(mMoveToTopReserved, moveToTop))
				mMoveToTopReserved.push_back(moveToTop);
			DeleteValuesInVector(mMoveToBottomReserved, moveToTop);
		}
	}

	void HideUIsExcept(const std::vector<std::string>& excepts){
		mHideUIExcepts = excepts;
		DirtyRenderList(INVALID_HWND_ID);
	}


	void HighlightUI(const char* uiname){
		WinBases topWnds;
		FindUIWnds(uiname, topWnds);
		for (auto& it : topWnds)
		{
			it->StartHighlight(2.0f);
		}
	}

	void StopHighlightUI(const char* uiname){
		WinBases topWnds;
		FindUIWnds(uiname, topWnds);
		for (auto& it : topWnds)
		{
			it->StopHighlight();
		}
	}


	UIAnimationPtr GetGlobalAnimation(const char* animName){
		auto it = mAnimations.Find(animName);
		if (it == mAnimations.end())
			return 0;

		return it->second;
	}

	UIAnimationPtr GetGlobalAnimationOrCreate(const char* animName){
		auto anim = GetGlobalAnimation(animName);
		if (!anim)
		{
			anim = UIAnimation::Create();
			anim->SetName(animName);
			mAnimations.Insert(std::make_pair(animName, anim));
		}
		return anim;
	}

	void PrepareTooltipUI(){
		LuaObject tooltip;
		tooltip.NewTable(mL);
		tooltip.SetField("name", "MouseTooltip");
		tooltip.SetField("type_", "Window");
		tooltip.SetField("pos", Vec2I(0, 0));
		tooltip.SetField("size", Vec2I(366, (int)gTooltipFontSize + 8));
		tooltip.SetField("HIDE_ANIMATION", "1");
		tooltip.SetField("SHOW_ANIMATION", "1");
		tooltip.SetField("NO_BACKGROUND", "false");
		tooltip.SetField("BACK_COLOR", "0, 0, 0, 0.8f");
		tooltip.SetField("ALWAYS_ON_TOP", "true");
		tooltip.SetField("USE_BORDER", "true");
		tooltip.SetField("NO_MOUSE_EVENT", "true");
		tooltip.SetField("WND_NO_FOCUS", "true");
		auto tooltipChildren = tooltip.SetFieldTable("children");
		auto children1 = tooltipChildren.SetSeqTable(1);
		children1.SetField("name", "TooltipTextBox");
		children1.SetField("type_", "TextBox");
		children1.SetField("pos", Vec2I(6, TooltipTextYPosition));
		children1.SetField("size", Vec2I(350, (int)gTooltipFontSize));
		children1.SetField("TEXTBOX_MATCH_HEIGHT", "true");
		children1.SetField("TEXT_VALIGN", "top");
		children1.SetField("TEXT_SIZE", StringConverter::ToString(gTooltipFontSize).c_str());
		bool success = AddLuaUI("MouseTooltip", tooltip, Renderer::GetInstance().GetMainWindowHandleId());
		if (!success)
		{
			Error(FB_ERROR_LOG_ARG, "Cannot create MouseTooltip UI.");
		}
		else
		{
			WinBases wnds;
			FindUIWnds("MouseTooltip", wnds);
			assert(!wnds.empty());
			mTooltipUI = wnds[0];
			mTooltipTextBox = FindComp("MouseTooltip", "TooltipTextBox");			
			//mTooltipTextBox->SetProperty(UIProperty::USE_BORDER, "true");
		}
	}


	UICommandsPtr GetUICommands() const {
		return mUICommands; 
	}
	void SetUIEditorModuleHandle(ModuleHandle moduleHandle){
		mUIEditorModuleHandle = moduleHandle; 
	}
	ModuleHandle GetUIEditorModuleHandle() const {
		return mUIEditorModuleHandle; 
	}

	WinBasePtr WinBaseWithPoint(const Vec2I& pt, const RegionTestParam& param){
		auto hwndId = GetForegroundWindowId();
		auto windows = mWindows[hwndId];
		auto it = windows.rbegin(), itEnd = windows.rend();
		for (; it != itEnd; it++){
			auto wnd = *it;
			if (!wnd->GetVisible()){
				continue;
			}
			if (param.mHwndId != wnd->GetHwndId())
				continue;
			if (param.mCheckMouseEvent && wnd->GetNoMouseEvent())
				continue;
			if (param.mRestrictToThisWnd && param.mRestrictToThisWnd != wnd)
				continue;

			auto found = wnd->WinBaseWithPoint(pt, param);
			if (found)
				return found;

			if (!param.mCheckMouseEvent || !wnd->GetNoMouseEventAlone()){
				if (wnd->IsIn(pt, param.mIgnoreScissor))
					return wnd;
			}
		}

		return 0;
	}

	WinBasePtr WinBaseWithPointCheckAlways(const Vec2I& pt, const RegionTestParam& param){
		for (auto it = mAlwaysMouseOverCheckComps.begin(); it != mAlwaysMouseOverCheckComps.end(); /**/){
			IteratingWeakContainer(mAlwaysMouseOverCheckComps, it, wnd);			
			if (!wnd->GetVisible() || wnd->GetVisualOnly()){
				continue;
			}
			if (param.mHwndId != wnd->GetHwndId())
				continue;
			if (param.mCheckMouseEvent && wnd->GetNoMouseEvent())
				continue;
			if (param.mRestrictToThisWnd && param.mRestrictToThisWnd != wnd)
				continue;

			auto found = wnd->WinBaseWithPoint(pt, param);
			if (found)
				return found;

			if (!param.mCheckMouseEvent || !wnd->GetNoMouseEventAlone()){
				if (wnd->IsIn(pt, param.mIgnoreScissor))
					return wnd;
			}
		}

		return 0;
	}

	TextManipulator* GetTextManipulator() const { 
		return mTextManipulator.get(); 
	}

	const char* GetUIPath(const char* uiname) const{
		std::string lower = FileSystem::GetName(uiname);
		ToLowerCase(lower);

		auto it = mLuaUIs.find(lower.c_str());
		if (it == mLuaUIs.end())
		{
			Error(FB_ERROR_LOG_ARG, FormatString("cannot find the ui %s", uiname));
			return "";
		}
		auto& windows = it->second;
		if (windows.empty())
			return "";
		return windows[0]->GetUIFilePath();
	}

	const char* GetUIScriptPath(const char* uiname) const{
		std::string lower(uiname);
		ToLowerCase(lower);
		auto it = mLuaUIs.find(lower.c_str());
		if (it == mLuaUIs.end())
			return "";
		auto& windows = it->second;
		if (windows.empty())
			return "";
		return windows[0]->GetScriptPath();
	}


	void SuppressPropertyWarning(bool suppress){
		WinBase::SuppressPropertyWarning(suppress);
	}


	void SetStyle(const char* style){
		if (mStyle == style)
			return;
		mAlphaInfoTexture.clear();
		mBorderAlphaRegion.clear();
		mWindowAlphaRegion.clear();

		char buf[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, buf);
		mStyle = style;
		tinyxml2::XMLDocument doc;
		int err = doc.LoadFile("EssentialEngineData/ui/style.xml");
		if (err){
			Error("parsing style file(EssentialEngineData/ui/style.xml) failed.");
			if (doc.GetErrorStr1())
				Error(doc.GetErrorStr1());
			if (doc.GetErrorStr2())
				Error(doc.GetErrorStr2());
			return;
		}

		auto root = doc.RootElement();
		if (!root)
			return;

		auto styleElem = root->FirstChildElement("Style");
		while (styleElem){
			auto sz = styleElem->Attribute("name");
			if (sz && _stricmp(sz, style) == 0){
				auto borderElem = styleElem->FirstChildElement("Border");
				if (borderElem){
					const char* atts[] = {
						"lt", "t", "rt", "l", "r", "lb", "b", "rb"
					};

					unsigned num = ARRAYCOUNT(atts);
					for (unsigned i = 0; i < num; ++i){
						auto sz = borderElem->Attribute(atts[i]);
						if (sz){
							mBorderRegions[atts[i]] = sz;
						}
					}
					sz = borderElem->Attribute("alphaInfo");
					if (sz){
						mBorderAlphaRegion = sz;
					}
				}

				auto winElem = styleElem->FirstChildElement("WindowFrame");
				if (winElem){
					const char* atts[] = {
						"lt", "t", "mt", "rt", "l", "r", "lb", "b", "rb"
					};
					unsigned num = ARRAYCOUNT(atts);
					for (unsigned i = 0; i < num; ++i){
						auto sz = winElem->Attribute(atts[i]);
						if (sz){
							mWindowRegions[atts[i]] = sz;
						}
					}
					sz = winElem->Attribute("alphaInfo");
					if (sz){
						mWindowAlphaRegion = sz;
					}
				}

				auto listBoxElem = styleElem->FirstChildElement("ListBox");
				if (listBoxElem){
					auto sz = listBoxElem->Attribute("headerBackColor");
					if (sz){
						mStyleStrings[Styles::ListBoxHeaderBack] = sz;
					}
					else{
						mStyleStrings[Styles::ListBoxHeaderBack] = "0.1, 0.1, 0.1, 1";
					}

					sz = listBoxElem->Attribute("backColor");
					if (sz){
						mStyleStrings[Styles::ListBoxBack] = sz;
					}
					else{
						mStyleStrings[Styles::ListBoxBack] = "0.0, 0.0, 0.0, 0.5";
					}
				}

				auto staticTextElem = styleElem->FirstChildElement("StaticText");
				if (staticTextElem){
					auto sz = staticTextElem->Attribute("backColor");
					if (sz){
						mStyleStrings[Styles::StaticTextBack] = sz;
					}
					else{
						mStyleStrings[Styles::StaticTextBack] = "0.0 0.0 0.0 0.7";
					}
				}

				break;
			}

			styleElem = styleElem->NextSiblingElement();
		}

		// RecreateAll borders
		for (auto& it : mWindows){
			for (auto& wnd : it.second){
				wnd->RecreateBorders();
			}
		}
	}


	const char* GetBorderRegion(const char* key) const{
		auto it = mBorderRegions.Find(key);
		if (it != mBorderRegions.end()){
			return it->second.c_str();
		}
		return "";
	}

	const char* GetWndBorderRegion(const char* key) const{
		auto it = mWindowRegions.Find(key);
		if (it != mWindowRegions.end()){
			return it->second.c_str();
		}
		return "";
	}

	const char* GetStyleString(Styles::Enum s) const{
		assert(s < Styles::Num);
		return mStyleStrings[s].c_str();
	}


	TexturePtr GetBorderAlphaInfoTexture(const Vec2I& size, bool& callmeLater){
		callmeLater = false;
		if (size.x == 0 || size.y == 0){
			callmeLater = true;
			return 0;
		}
		auto it = mAlphaInfoTexture.Find(size);
		if (it != mAlphaInfoTexture.end()){
			return it->second;
		}
		else{
			if (mBorderAlphaRegion.empty())
				return 0;

			// GenerateAlphaTexture
			auto atlas = Renderer::GetInstance().GetTextureAtlas("EssentialEngineData/textures/ui.xml");
			if (!atlas)
				return 0;

			if (!atlas->GetTexture()->IsReady()){
				callmeLater = true;
				return 0;
			}

			auto alphaRegion = atlas->GetRegion(mBorderAlphaRegion.c_str());
			if (!alphaRegion)
				return 0;

			const auto& atlasSize = atlas->GetTexture()->GetSize();
			if (!mAtlasStaging){
				mAtlasStaging = Renderer::GetInstance().CreateTexture(0, atlasSize.x, atlasSize.y, PIXEL_FORMAT_R8G8B8A8_UNORM,
					BUFFER_USAGE_STAGING, BUFFER_CPU_ACCESS_READ, TEXTURE_TYPE_DEFAULT);
				atlas->GetTexture()->CopyToStaging(mAtlasStaging, 0, 0, 0, 0, 0, 0);
			}
			if (!mAtlasStaging)
				return 0;

			auto mapData = mAtlasStaging->Map(0, MAP_TYPE_READ, MAP_FLAG_NONE);
			if (!mapData.pData)
				return 0;
			unsigned* atlasData = (unsigned*)mapData.pData;

			unsigned* data = (unsigned*)malloc(4 * size.x * size.y);
			memset(data, 0xffffffff, 4 * size.x * size.y);

			unsigned pitchInPixel = mapData.RowPitch / 4;
			// copy lt
			auto ltRegion = atlas->GetRegion(GetBorderRegion("lt"));
			if (ltRegion){
				const auto& ltsize = ltRegion->GetSize();
				if (size.x > ltsize.x && size.y > ltsize.y){
					for (int r = 0; r < ltsize.y; ++r){
						memcpy(data + r * size.x,
							atlasData + (alphaRegion->mStart.y + r) * pitchInPixel + alphaRegion->mStart.x,
							ltsize.x * 4);
					}
				}
			}
			auto rtRegion = atlas->GetRegion(GetBorderRegion("rt"));
			if (rtRegion){
				const auto& rtsize = rtRegion->GetSize();
				if (size.x > rtsize.x && size.y > rtsize.y){
					for (int r = 0; r < rtsize.y; ++r){
						memcpy(data + r * size.x + size.x - rtsize.x,
							atlasData + (alphaRegion->mStart.y + r)*pitchInPixel + alphaRegion->mStart.x + alphaRegion->mSize.x - rtsize.x,
							rtsize.x * 4);
					}
				}
			}
			auto lbRegion = atlas->GetRegion(GetBorderRegion("lb"));
			if (lbRegion){
				const auto& lbsize = lbRegion->GetSize();
				if (size.x > lbsize.x && size.y > lbsize.y){
					for (int r = 0; r < lbsize.y; ++r){
						memcpy(data + (size.y - lbsize.y + r) * size.x,
							atlasData + (alphaRegion->mStart.y + alphaRegion->mSize.y - lbsize.y + r) * pitchInPixel + alphaRegion->mStart.x,
							lbsize.x * 4);
					}
				}
			}

			auto rbRegion = atlas->GetRegion(GetBorderRegion("rb"));
			if (rbRegion){
				const auto& rbsize = rbRegion->GetSize();
				if (size.x > rbsize.x && size.y > rbsize.y){
					for (int r = 0; r < rbsize.y; ++r){
						memcpy(data + (size.y - rbsize.y + r) * size.x + size.x - rbsize.x,
							atlasData + (alphaRegion->mStart.y + alphaRegion->mSize.y - rbsize.y + r) * pitchInPixel
							+ alphaRegion->mStart.x + alphaRegion->mSize.x - rbsize.x,
							rbsize.x * 4);
					}
				}
			}

			auto texture = Renderer::GetInstance().CreateTexture(data, size.x, size.y, PIXEL_FORMAT_R8G8B8A8_UNORM, BUFFER_USAGE_IMMUTABLE, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_DEFAULT);
			mAlphaInfoTexture[size] = texture;
			free(data);
			mAtlasStaging->Unmap(0);
			return texture;
		}
	}


	void AddAlwaysMouseOverCheck(WinBasePtr comp){
		mAlwaysMouseOverCheckComps.insert(comp);
	}

	void RemoveAlwaysMouseOverCheck(WinBasePtr comp){
		auto it = mAlwaysMouseOverCheckComps.find(comp);
		if (it != mAlwaysMouseOverCheckComps.end()){
			mAlwaysMouseOverCheckComps.erase(it);
		}
	}

	//-------------------------------------------------------------------
	// For UI Editing
	//-------------------------------------------------------------------	
	void SetUIEditor(IUIEditor* editor){
		mUIEditor = editor;
	}

	IUIEditor* GetUIEditor() const {
		return mUIEditor; 
	}

	void StartLocatingComponent(ComponentType::Enum c){
		mLocatingComp = c;
	}

	void CancelLocatingComponent(){
		
	}

	void ChangeFilepath(WinBasePtr root, const char* newfile){
		auto name = FileSystem::GetName(root->GetUIFilePath());
		auto it = mLuaUIs.find(name);
		if (it == mLuaUIs.end())
		{
			Error(FB_ERROR_LOG_ARG, FormatString("Cannot find the ui %s", name.c_str()));
			return;
		}
		auto newName = FileSystem::GetName(newfile);
		if (name != newName)
		{
			std::string lowerNewName(newName);
			ToLowerCase(lowerNewName);
			auto newIt = mLuaUIs.find(lowerNewName);
			if (newIt != mLuaUIs.end())
			{
				Error(FB_ERROR_LOG_ARG, FormatString("The new name %s is already used.", lowerNewName.c_str()));
				return;
			}
			if (FileSystem::Exists(newfile))
			{
				FileSystem::BackupFile(newfile, 5, "Backup_UI");				
			}

			auto oldFilepath = root->GetUIFilePath();
			FileMonitor::GetInstance().IgnoreMonitoringOnFile(oldFilepath);
			FileMonitor::GetInstance().IgnoreMonitoringOnFile(newfile);
			FileSystem::Rename(oldFilepath, newfile);			
			mLuaUIs[lowerNewName].swap(it->second);
			mLuaUIs.erase(it);
			root->SetUIFilePath(newfile);
			root->SetName(newName.c_str());
			FileMonitor::GetInstance().ResumeMonitoringOnFile(newfile);
			FileMonitor::GetInstance().ResumeMonitoringOnFile(oldFilepath);
		}
	}

	void CopyCompsAtMousePos(const std::vector<WinBasePtr>& src){
		if (src.empty())
			return;		
		auto injector = InputManager::GetInstance().GetInputInjector();
		auto pt = injector->GetMousePos();
		RegionTestParam param;
		param.mOnlyContainer = true;
		param.mTestChildren = true;
		auto mouseOveredContainer = std::dynamic_pointer_cast<Container>(WinBaseWithPoint(pt, param));
		mMouseOveredContainer = mouseOveredContainer;
		
		if (!IsMainWindowForeground() || !mouseOveredContainer)
			return;

		auto overedUI = mouseOveredContainer->GetWndContentUI();
		if (!overedUI){
			overedUI = mouseOveredContainer;
		}
		auto pos = Vec2I(pt) - overedUI->GetFinalPos();
		auto offset = pos - src[0]->GetAlignedPos();
		std::vector<WinBasePtr> cloned;
		tinyxml2::XMLDocument doc;
		for (auto& s : src){
			auto comp = doc.NewElement("component");
			doc.InsertEndChild(comp);
			s->Save(*comp);
		}
		auto comp = doc.FirstChildElement("component");
		while (comp){
			const char* sz = comp->Attribute("type");
			if (!sz)
			{
				Error("component doesn't have the type attribute.");
				break;
			}
			ComponentType::Enum type = ComponentType::ConvertToEnum(sz);
			auto pWinBase = CreateComponent(type);
			cloned.push_back(pWinBase);
			pWinBase->ParseXML(comp);
			pWinBase->OnCreated();
			comp = comp->NextSiblingElement("component");
		}

		for (auto c : cloned){
			mouseOveredContainer->AddChild(c);
			c->SetVisible(true);
			c->Move(offset);
		}
	}


	// UIEditing
	void OnInputForLocating(IInputInjectorPtr injector){
		if (mLocatingComp == ComponentType::NUM)
			return;

		if (!IsMainWindowForeground())
			return;
		
		Vec2I pt = injector->GetMousePos();
		RegionTestParam rparam;
		rparam.mOnlyContainer = true;
		rparam.mIgnoreScissor = true;
		rparam.mTestChildren = true;
		rparam.mNoRuntimeComp = true;
		rparam.mHwndId = Renderer::GetInstance().GetMainWindowHandleId();
		auto mouseOveredContainer = std::dynamic_pointer_cast<Container>(WinBaseWithPoint(pt, rparam));
		mMouseOveredContainer = mouseOveredContainer;
		if (injector->IsLButtonDown())
		{
			if (!mDragBox.IsStarted())
			{
				mDragBox.Start(pt);
				mDragBox.SetMouseOveredContainer(mouseOveredContainer);
			}
			else
			{
				mDragBox.PushCur(pt);
			}
		}
		else if (mDragBox.IsStarted())
		{
			mDragBox.End(pt);
			if (mLocatingComp != ComponentType::NUM)
			{
				LocateComponent();
			}
			mDragBox.SetMouseOveredContainer(0);
		}

		if (injector->IsRButtonClicked())
		{
			mDragBox.End(pt);
			mLocatingComp = ComponentType::NUM;
			if (mUIEditor)
				mUIEditor->OnCancelComponent();
		}
	}

	void LocateComponent(){
		if (mLocatingComp == ComponentType::NUM)
			return;

		if (!IsMainWindowForeground())
			return;

		auto start = mDragBox.GetMin();
		auto end = mDragBox.GetMax();
		auto size = end - start;
		if (size.x == 0 || size.y == 0)
			return;

		auto cont = std::dynamic_pointer_cast<Container>(mDragBox.GetMouseOveredContainer());
		WinBasePtr win = 0;
		if (cont)
		{
			Vec2I pos = cont->GetWPos();
			pos = start - pos;
			win = cont->AddChild(pos, size, mLocatingComp);
			win->ChangeWPos(start);
		}
		else
		{
			std::string uiname = GetUniqueUIName();
			ToLowerCase(uiname);
			win = AddWindow(start, size, mLocatingComp);
			win->SetName(uiname.c_str());
			mLuaUIs[uiname].push_back(win);
			win->SetUIFilePath(FormatString("%s/%s.ui", mUIFolder.c_str(),
				uiname.c_str()).c_str());
			win->SetSaveNameCheck(true);
		}

		win->SetProperty(UIProperty::NO_BACKGROUND, "false");
		if (mLocatingComp == ComponentType::Window)
			win->SetProperty(UIProperty::BACK_COLOR, "0.05 0.1 0.15 0.8");
		win->SetProperty(UIProperty::VISIBLE, "true");		
		auto injector = InputManager::GetInstance().GetInputInjector();
		if (!injector->IsKeyDown(VK_SHIFT))
		{
			mLocatingComp = ComponentType::NUM;
			if (mUIEditor)
			{
				mUIEditor->OnComponentSelected(win);
				mUIEditor->OnCancelComponent();
			}
		}
		else
		{
			mMultiLocating = true;
		}
	}

	std::string GetUniqueUIName() const{
		std::string candi = "UI1";
		unsigned index = 1;
		do{
			std::string lower(candi);
			ToLowerCase(lower);
			auto itFind = mLuaUIs.find(lower);
			if (itFind == mLuaUIs.end()){
				if (!FileSystem::Exists(
					FormatString("%s/%s.ui", mUIFolder.c_str(), candi.c_str()).c_str())){
					break;
				}
			}
			index += 1;
			candi = FormatString("UI%u", index);
		} while (99999999);

		return candi;
	}

	std::string GetBackupName(const std::string& name) const{
		std::string bakName = name + ".bak";
		if (FileSystem::Exists(bakName.c_str()))
		{
			std::string bakName2 = name + ".bak2";
			FileSystem::Remove(bakName2.c_str());
			FileSystem::Rename(bakName.c_str(), bakName2.c_str());
		}
		return bakName;
	}

	void DragUI(){
		assert(mUIEditor);
		static bool sDragStarted = false;
		static bool sSizingXRight = false;
		static bool sSizingXLeft = false;
		static bool sSizingYTop = false;
		static bool sSizingYBottom = false;
		static Vec2I sExpand(8, 8);
		static int sAreaX = 4;
		static int sAreaY = 4;

		if (!IsMainWindowForeground())
			return;

		auto injector = InputManager::GetInstance().GetInputInjector();		
		if (injector->IsKeyDown(VK_MENU))
			return;

		unsigned num = mUIEditor->GetNumCurEditing();
		if (!num){
			assert(!sDragStarted);
			return;
		}

		Vec2I dragStartPos;
		bool dragStarted = injector->IsDragStarted(dragStartPos.x, dragStartPos.y);
		Vec2I testPos = dragStarted ? dragStartPos : injector->GetMousePos();

		if (!sDragStarted)
		{
			bool in = false;
			for (unsigned i = 0; i < num; ++i){
				auto curUI = mUIEditor->GetCurSelected(i);
				if (!curUI)
					return;

				in = curUI->IsIn(testPos, true, &sExpand);
				if (in) {
					sSizingXLeft = curUI->IsPtOnLeft(testPos, sAreaX);
					sSizingXRight = curUI->IsPtOnRight(testPos, sAreaX);
					sSizingYTop = curUI->IsPtOnTop(testPos, sAreaY);
					sSizingYBottom = curUI->IsPtOnBottom(testPos, sAreaY);
					break;
				}
			}
			if (in){
				if (
					(sSizingXLeft && sSizingYTop) ||
					(sSizingXRight && sSizingYBottom)
					)
				{
					SetCursor(WinBase::sCursorNWSE);
				}
				else if (
					(sSizingXRight && sSizingYTop) ||
					(sSizingXLeft && sSizingYBottom)
					)
				{
					SetCursor(WinBase::sCursorNESW);
				}
				else if (sSizingXLeft || sSizingXRight){
					SetCursor(WinBase::sCursorWE);
				}
				else if (sSizingYTop || sSizingYBottom){
					SetCursor(WinBase::sCursorNS);
				}
				else {
					SetCursor(WinBase::sCursorAll);
				}

				if (dragStarted){
					injector->PopDragEvent();
					sDragStarted = true;
					mUIEditor->BackupSizePos();
				}
			}
		}

		if (sDragStarted){
			Vec2I delta = injector->GetDeltaXY();
			for (unsigned i = 0; i < num; ++i){
				auto curUI = mUIEditor->GetCurSelected(i);
				bool sizing = false;
				if (sSizingXLeft)
				{
					curUI->Move(Vec2I(delta.x, 0));
					curUI->ModifySize(Vec2I(-delta.x, 0));
					sizing = true;
				}
				if (sSizingXRight)
				{
					curUI->ModifySize(Vec2I(delta.x, 0));
					sizing = true;
				}
				if (sSizingYTop)
				{
					curUI->Move(Vec2I(0, delta.y));
					curUI->ModifySize(Vec2I(0, -delta.y));
					sizing = true;
				}
				if (sSizingYBottom)
				{
					curUI->ModifySize(Vec2I(0, delta.y));
					sizing = true;
				}
				if (!sizing)
				{
					curUI->Move(delta);
				}
			}

			if (
				(sSizingXLeft && sSizingYTop) ||
				(sSizingXRight && sSizingYBottom)
				)
			{
				SetCursor(WinBase::sCursorNWSE);
			}
			else if (
				(sSizingXRight && sSizingYTop) ||
				(sSizingXLeft && sSizingYBottom)
				)
			{
				SetCursor(WinBase::sCursorNESW);
			}
			else if (sSizingXLeft || sSizingXRight){

			}
			else if (sSizingYTop || sSizingYBottom){
				SetCursor(WinBase::sCursorNS);
			}
			else{
				SetCursor(WinBase::sCursorAll);
			}

			mUIEditor->OnPosSizeChanged();
			if (injector->IsDragEnded() || injector->IsKeyDown(VK_ESCAPE)){
				if (injector->IsKeyDown(VK_ESCAPE)){
					mUIEditor->RestoreSizePos();
				}
				injector->PopDragEvent();
				sDragStarted = false;
				sSizingXRight = false;
				sSizingXLeft = false;
				sSizingYTop = false;
				sSizingYBottom = false;
			}
		}
	}

	void DebugUI(){
		auto curHwndId = GetForegroundWindowId();		
		Vec2I mousePos(InputManager::GetInstance().GetInputInjector()->GetMousePos());
		for (auto& win : mWindows){
			if (win.first != curHwndId)
				continue;

			for (auto comp : win.second){
				if (comp->GetVisible() && comp->IsIn(mousePos, false)){
					wchar_t buf[1024];
					swprintf_s(buf, L"UIName = %s", AnsiToWide(comp->GetName()));
					Renderer::GetInstance().QueueDrawText(mousePos, buf, Color::SkyBlue);
					break;
				}
			}
		}
	}

};

UIManagerWeakPtr sUIManager;
UIManager* sUIManagerRaw = 0;
UIManagerPtr UIManager::Create(){
	if (sUIManager.expired()){
		UIManagerPtr p(new UIManager, [](UIManager* obj){ delete obj; });
		sUIManager = p;
		sUIManagerRaw = p.get();
		p->mImpl->Initialize();
		return p;
	}
	return sUIManager.lock();
}

bool UIManager::HasInstance(){
	return !sUIManager.expired();
}

UIManager& UIManager::GetInstance(){
	if (sUIManager.expired()){
		Logger::Log(FB_ERROR_LOG_ARG, "UIManager is already deleted. Program will crash...");
	}
	return *sUIManagerRaw;
}

//---------------------------------------------------------------------------
UIManager::UIManager()
	: mImpl(new Impl(this))
{	
}

UIManager::~UIManager()
{	
	Logger::Log(FB_DEFAULT_LOG_ARG, "UIManager deleted.");
}

WinBasePtr UIManager::CreateComponent(ComponentType::Enum type){
	return mImpl->CreateComponent(type);
}

void UIManager::Shutdown() {
	mImpl->Shutdown();
}

void UIManager::SetSound(UISounds::Enum type, const char* path){
	mImpl->SetSound(type, path);
}

void UIManager::PlaySound(UISounds::Enum type){
	mImpl->PlaySound(type);
}

void UIManager::OnChangeDetected(){

}

bool UIManager::OnFileChanged(const char* watchDir, const char* file, const char* loweredExt) {
	return mImpl->OnFileChanged(watchDir, file, loweredExt);
}

void UIManager::BeforeUIRendering(HWindowId hwndId, HWindow hwnd) {
}

void UIManager::RenderUI(HWindowId hwndId, HWindow hwnd){
	mImpl->RenderUI(hwndId, hwnd);
}

void UIManager::AfterUIRendered(HWindowId hwndId, HWindow hwnd){

}

void UIManager::BeforeDebugHudRendering() {
	mImpl->BeforeDebugHudRendering();
}

void UIManager::AfterDebugHudRendered() {
	mImpl->AfterDebugHudRendered();
}

void UIManager::OnResolutionChanged(HWindowId hwndId, HWindow hwnd) {
	mImpl->OnResolutionChanged(hwndId, hwnd);
}

void UIManager::Update(float elapsedTime) {
	mImpl->Update(elapsedTime);
}

void UIManager::GatherRenderList() {
	mImpl->GatherRenderList();
}

bool UIManager::ParseUI(const char* filepath, WinBases& windows, std::string& uiname, HWindowId hwndId, bool luaUI) {
	auto unifiedString = FileSystem::UnifyFilepath(filepath);
	return mImpl->ParseUI(unifiedString.c_str(), windows, uiname, hwndId, luaUI);
}

bool UIManager::SaveUI(const char* uiname, tinyxml2::XMLDocument& doc) {
	return mImpl->SaveUI(uiname, doc);
}

bool UIManager::AddLuaUI(const char* uiName, LuaObject& data, HWindowId hwndId) {
	return mImpl->AddLuaUI(uiName, data, hwndId);
}

void UIManager::DeleteLuaUI(const char* uiName, bool pending) {
	mImpl->DeleteLuaUI(uiName, pending);
}

bool UIManager::IsLoadedUI(const char* uiName) {
	return mImpl->IsLoadedUI(uiName);
}

WinBasePtr UIManager::AddWindow(int posX, int posY, int width, int height, ComponentType::Enum type, HWindowId hwndId) {
	return mImpl->AddWindow(posX, posY, width, height, type, hwndId);
}

WinBasePtr UIManager::AddWindow(const Vec2I& pos, const Vec2I& size, ComponentType::Enum type, HWindowId hwndId) {
	return mImpl->AddWindow(pos, size, type, hwndId);
}

WinBasePtr UIManager::AddWindow(float posX, float posY, float width, float height, ComponentType::Enum type, HWindowId hwndId) {
	return mImpl->AddWindow(posX, posY, width, height, type, hwndId);
}

WinBasePtr UIManager::AddWindow(ComponentType::Enum type, HWindowId hwndId) {
	return mImpl->AddWindow(type, hwndId);
}

void UIManager::DeleteWindow(WinBasePtr pWnd) {
	mImpl->DeleteWindow(pWnd);
}

void UIManager::DeleteWindowsFor(HWindowId hwndId) {
	mImpl->DeleteWindowsFor(hwndId);
}

void UIManager::SetFocusUI(WinBasePtr pWnd) {
	mImpl->SetFocusUI(pWnd);
}

WinBasePtr UIManager::GetFocusUI() const {
	return mImpl->GetFocusUI();
}

WinBasePtr UIManager::GetKeyboardFocusUI() const {
	return mImpl->GetKeyboardFocusUI();
}

WinBasePtr UIManager::GetNewFocusUI() const {
	return mImpl->GetNewFocusUI();
}

void UIManager::SetFocusUI(const char* uiName) {
	mImpl->SetFocusUI(uiName);
}

bool UIManager::IsFocused(const WinBasePtr pWnd) const {
	return mImpl->IsFocused(pWnd);
}

void UIManager::DirtyRenderList(HWindowId hwndId) {
	mImpl->DirtyRenderList(hwndId);
}

void UIManager::SetUIProperty(const char* uiname, const char* compname, const char* prop, const char* val, bool updatePosSize) {
	mImpl->SetUIProperty(uiname, compname, prop, val, updatePosSize);
}

void UIManager::SetUIProperty(const char* uiname, const char* compname, UIProperty::Enum prop, const char* val, bool updatePosSize) {
	mImpl->SetUIProperty(uiname, compname, prop, val, updatePosSize);
}

void UIManager::SetEnableComponent(const char* uiname, const char* compname, bool enable) {
	mImpl->SetEnableComponent(uiname, compname, enable);
}

void UIManager::ConsumeInput(IInputInjectorPtr injector) {
	mImpl->ConsumeInput(injector);
}

void UIManager::ProcessMouseInput(IInputInjectorPtr injector) {
	mImpl->ProcessMouseInput(injector);
}

void UIManager::EnableInputListener(bool enable) {
	mImpl->EnableInputListener(enable);
}

bool UIManager::IsEnabledInputLIstener() const {
	return mImpl->IsEnabledInputLIstener();
}

HCURSOR UIManager::GetMouseCursorOver() const {
	return mImpl->GetMouseCursorOver();
}

void UIManager::SetMouseCursorOver() {
	mImpl->SetMouseCursorOver();
}

void UIManager::DisplayMsg(const std::string& msg, ...) {
	mImpl->DisplayMsg(msg);
}

bool UIManager::IsMouseInUI() const {
	return mImpl->IsMouseInUI();
}

void UIManager::SetTooltipString(const std::wstring& ts) {
	mImpl->SetTooltipString(ts);
}

void UIManager::SetTooltipPos(const Vec2& npos, bool checkNewPos) {
	mImpl->SetTooltipPos(npos, checkNewPos);
}

void UIManager::CleanTooltip() {
	mImpl->CleanTooltip();
}

void UIManager::PopupDialog(WCHAR* msg, POPUP_TYPE type, std::function< void(void*) > func) {
	mImpl->PopupDialog(msg, type, func);
}

int UIManager::GetPopUpResult() const {
	return mImpl->GetPopUpResult();
}

lua_State* UIManager::GetLuaState() const {
	return mImpl->GetLuaState();
}

WinBasePtr UIManager::FindComp(const char* uiname, const char* compName) const {
	return mImpl->FindComp(uiname, compName);
}

void UIManager::FindUIWnds(const char* uiname, WinBases& outV) const {
	mImpl->FindUIWnds(uiname, outV);
}

bool UIManager::CacheListBox(const char* uiname, const char* compName) {
	return mImpl->CacheListBox(uiname, compName);
}

ListBoxPtr UIManager::GetCachedListBox() const {
	return mImpl->GetCachedListBox();
}

void UIManager::SetEnablePosSizeEvent(bool enable) {
	mImpl->SetEnablePosSizeEvent(enable);
}

bool UIManager::GetEnablePosSizeEvent() const {
	return mImpl->GetEnablePosSizeEvent();
}

void UIManager::SetVisible(const char* uiname, bool visible) {
	mImpl->SetVisible(uiname, visible);
}

void UIManager::LockFocus(bool lock) {
	mImpl->LockFocus(lock);
}

bool UIManager::GetVisible(const char* uiname) const {
	return mImpl->GetVisible(uiname);
}

void UIManager::CloseAllLuaUI() {
	mImpl->CloseAllLuaUI();
}

void UIManager::CloneUI(const char* uiname, const char* newUIname) {
	mImpl->CloneUI(uiname, newUIname);
}

void UIManager::IgnoreInput(bool ignore, WinBasePtr modalWindow) {
	mImpl->IgnoreInput(ignore, modalWindow);
}

void UIManager::ToggleVisibleLuaUI(const char* uisname) {
	mImpl->ToggleVisibleLuaUI(uisname);
}

void UIManager::RegisterAlwaysOnTopWnd(WinBasePtr win) {
	mImpl->RegisterAlwaysOnTopWnd(win);
}

void UIManager::UnRegisterAlwaysOnTopWnd(WinBasePtr win) {
	mImpl->UnRegisterAlwaysOnTopWnd(win);
}

void UIManager::MoveToBottom(const char* moveToBottom) {
	mImpl->MoveToBottom(moveToBottom);
}

void UIManager::MoveToBottom(WinBasePtr moveToBottom) {
	mImpl->MoveToBottom(moveToBottom);
}

void UIManager::MoveToTop(WinBasePtr moveToTop) {
	mImpl->MoveToTop(moveToTop);
}

void UIManager::HideUIsExcept(const std::vector<std::string>& excepts) {
	mImpl->HideUIsExcept(excepts);
}

void UIManager::HighlightUI(const char* uiname) {
	mImpl->HighlightUI(uiname);
}

void UIManager::StopHighlightUI(const char* uiname) {
	mImpl->StopHighlightUI(uiname);
}

UIAnimationPtr UIManager::GetGlobalAnimation(const char* animName) {
	return mImpl->GetGlobalAnimation(animName);
}

UIAnimationPtr UIManager::GetGlobalAnimationOrCreate(const char* animName) {
	return mImpl->GetGlobalAnimationOrCreate(animName);
}

void UIManager::PrepareTooltipUI() {
	mImpl->PrepareTooltipUI();
}

UICommandsPtr UIManager::GetUICommands() const {
	return mImpl->GetUICommands();
}

void UIManager::SetUIEditorModuleHandle(ModuleHandle moduleHandle) {
	mImpl->SetUIEditorModuleHandle(moduleHandle);
}

ModuleHandle UIManager::GetUIEditorModuleHandle() const {
	return mImpl->GetUIEditorModuleHandle();
}

WinBasePtr UIManager::WinBaseWithPoint(const Vec2I& pt, const RegionTestParam& param) {
	return mImpl->WinBaseWithPoint(pt, param);
}

WinBasePtr UIManager::WinBaseWithPointCheckAlways(const Vec2I& pt, const RegionTestParam& param) {
	return mImpl->WinBaseWithPointCheckAlways(pt, param);
}

TextManipulator* UIManager::GetTextManipulator() const {
	return mImpl->GetTextManipulator();
}

const char* UIManager::GetUIPath(const char* uiname) const {
	return mImpl->GetUIPath(uiname);
}

const char* UIManager::GetUIScriptPath(const char* uiname) const {
	return mImpl->GetUIScriptPath(uiname);
}

void UIManager::SuppressPropertyWarning(bool suppress) {
	mImpl->SuppressPropertyWarning(suppress);
}

void UIManager::SetStyle(const char* style) {
	mImpl->SetStyle(style);
}

const char* UIManager::GetBorderRegion(const char* key) const {
	return mImpl->GetBorderRegion(key);
}

const char* UIManager::GetWndBorderRegion(const char* key) const {
	return mImpl->GetWndBorderRegion(key);
}

const char* UIManager::GetStyleString(Styles::Enum s) const {
	return mImpl->GetStyleString(s);
}

TexturePtr UIManager::GetBorderAlphaInfoTexture(const Vec2I& size, bool& callmeLater) {
	return mImpl->GetBorderAlphaInfoTexture(size, callmeLater);
}

void UIManager::AddAlwaysMouseOverCheck(WinBasePtr comp) {
	mImpl->AddAlwaysMouseOverCheck(comp);
}

void UIManager::RemoveAlwaysMouseOverCheck(WinBasePtr comp) {
	mImpl->RemoveAlwaysMouseOverCheck(comp);
}

void UIManager::SetUIEditor(IUIEditor* editor) {
	mImpl->SetUIEditor(editor);
}

IUIEditor* UIManager::GetUIEditor() const {
	return mImpl->GetUIEditor();
}

void UIManager::StartLocatingComponent(ComponentType::Enum c) {
	mImpl->StartLocatingComponent(c);
}

void UIManager::CancelLocatingComponent() {
	mImpl->CancelLocatingComponent();
}

void UIManager::ChangeFilepath(WinBasePtr root, const char* newfile) {
	mImpl->ChangeFilepath(root, newfile);
}

void UIManager::CopyCompsAtMousePos(const std::vector<WinBasePtr>& src) {
	mImpl->CopyCompsAtMousePos(src);
}

