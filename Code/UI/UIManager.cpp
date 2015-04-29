#include <UI/StdAfx.h>
#include <UI/UIManager.h>
#include <UI/Wnd.h>
#include <UI/TextField.h>
#include <UI/StaticText.h>
#include <UI/Button.h>
#include <UI/KeyboardCursor.h>
#include <UI/ImageBox.h>
#include <UI/CheckBox.h>
#include <UI/ListBox.h>
#include <UI/FileSelector.h>
#include <UI/Scroller.h>
#include <UI/RadioBox.h>
#include <UI/HexagonalContextMenu.h>
#include <UI/CardScroller.h>
#include <UI/VerticalGauge.h>
#include <UI/HorizontalGauge.h>
#include <UI/NumericUpDown.h>
#include <UI/DropDown.h>
#include <UI/TextBox.h>
#include <UI/ColorRampComp.h>
#include <UI/NamedPortrait.h>
#include <UI/UIAnimation.h>
#include <CommonLib/FileSystem.h>
#include <CommonLib/LuaUtils.h>
#include <CommonLib/LuaObject.h>
#include <Engine/IUIObject.h>
#include <Engine/IScriptSystem.h>

namespace fastbird
{

IUIManager* IUIManager::mUIManager = 0;

void IUIManager::InitializeUIManager(lua_State* L)
{
	assert(!mUIManager);
	mUIManager = FB_NEW(UIManager)(L);
	mUIManager->PrepareTooltipUI();
	WinBase::InitMouseCursor();
	
}

void IUIManager::FinalizeUIManager()
{
	WinBase::FinalizeMouseCursor();
	assert(mUIManager);
	FB_SAFE_DEL(mUIManager);

#ifdef USING_FB_MEMORY_MANAGER
	FBReportMemoryForModule();
#endif
}

IUIManager& IUIManager::GetUIManager()
{
	assert(mUIManager);
	return *mUIManager;
}
static float gTooltipFontSize = 26;
static float gDelayForTooltip = 0.7f;
//---------------------------------------------------------------------------
UIManager::UIManager(lua_State* L)
	: mInputListenerEnable(true)
	, mNeedToRegisterUIObject(false)
	, mFocusWnd(0)
	, mMouseIn(false)
	, mPopup(0)
	, mPopupCallback(std::function< void(void*) >())
	, mPopupResult(0)
	, mL(L)
	, mPosSizeEventEnabled(true), mIgnoreInput(false)
	, mCachedListBox(0)
	, mModelWindow(0), mLockFocus(false), mDelayForTooltip(0)
{
	gpTimer = gEnv->pTimer;
	FileSystem::Initialize();
	gEnv->pEngine->AddInputListener(this,
		fastbird::IInputListener::INPUT_LISTEN_PRIORITY_UI, 0);
	KeyboardCursor::InitializeKeyboardCursor();
	RegisterLuaFuncs(L);	
}

UIManager::~UIManager()
{

	for (auto it : mAnimations)
	{
		FB_DELETE(it.second);
	}
	mAnimations.clear();

	DeleteWindow(mPopup);
	// delete lua uis
	for (const auto& it : mLuaUIs)
	{
		for (const auto& ui : it.second)
		{
			DeleteWindow(ui);
		}
	}
	assert(mWindows.empty());
	gEnv->pEngine->RemoveInputListener(this);
	KeyboardCursor::FinalizeKeyboardCursor();
	FileSystem::Finalize();
}

void UIManager::Shutdown()
{
	gEnv->pEngine->UnregisterUIs();
	WINDOWS::iterator it = mWindows.begin(), itEnd = mWindows.end();
	for (; it!=itEnd; it++)
	{
		FB_SAFE_DEL(*it);
	}
	mWindows.clear();
}

//---------------------------------------------------------------------------
void UIManager::Update(float elapsedTime)
{
	for (auto ui : mSetFocusReserved)
	{
		if (mFocusWnd)
			mFocusWnd->OnFocusLost();
		mFocusWnd = ui;
		if (mFocusWnd)
			mFocusWnd->OnFocusGain();

		WINDOWS::iterator f = std::find(mWindows.begin(), mWindows.end(), ui);
		if (f != mWindows.end())
		{
			// insert f at the mWindows.end().
			mWindows.splice(mWindows.end(), mWindows, f);
		}
		if (!ui->IsAlwaysOnTop())
		{
			for (auto& win : mAlwaysOnTopWindows)
			{
				if (!win->GetVisible())
					continue;
				WINDOWS::iterator f = std::find(mWindows.begin(), mWindows.end(), win);
				if (f != mWindows.end())
				{
					// insert f at the mWindows.end().
					mWindows.splice(mWindows.end(), mWindows, f);
				}
			}
		}
		if (!ui->GetRender3D())
			mNeedToRegisterUIObject = true;
	}
	mSetFocusReserved.clear();

	for (auto& ui : mMoveToBottomReserved)
	{
		WINDOWS::iterator f = std::find(mWindows.begin(), mWindows.end(), ui);
		if (f != mWindows.end())
		{
			// insert f at the mWindows.begin().
			if (mWindows.begin() != f)
			{
				mWindows.splice(mWindows.begin(), mWindows, f);
				mNeedToRegisterUIObject = true;
			}			
		}		
	}
	mMoveToBottomReserved.clear();	

	for each(auto& wnd in mWindows)
	{
		if (wnd->GetVisible())
			wnd->OnStartUpdate(elapsedTime);
	}

	if (!mTooltipText.empty())
	{
		mDelayForTooltip -= elapsedTime;
		if (mDelayForTooltip <= 0)
		{
			ShowTooltip();
		}
	}
}

void UIManager::GatherRenderList()
{
	if (mNeedToRegisterUIObject)
	{
		mNeedToRegisterUIObject = false;
		std::vector<IUIObject*> uiObjects;
		uiObjects.reserve(200);
		std::map<std::string, std::vector<IUIObject*>> render3DList;

		bool hideAll = !mHideUIExcepts.empty();
		WINDOWS::iterator it = mWindows.begin(), itEnd = mWindows.end();
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
					assert(strlen((*it)->GetName()) != 0);
					auto& list = render3DList[(*it)->GetName()];
					list.reserve(100);
					(*it)->GatherVisit(list);
					std::sort(uiObjects.begin(), uiObjects.end(), [](IUIObject* a, IUIObject* b){
						return a->GetSpecialOrder() < b->GetSpecialOrder();
					});
				}
				else
				{
					(*it)->GatherVisit(uiObjects);

					std::sort(uiObjects.begin() + start, uiObjects.end(), [](IUIObject* a, IUIObject* b){
						return a->GetSpecialOrder() < b->GetSpecialOrder();
					});
					start = uiObjects.size();
				}
			}				
		}


		if (mPopup&& mPopup->GetVisible())
			mPopup->GatherVisit(uiObjects);

		// rendering order : reverse.
		gEnv->pEngine->RegisterUIs(uiObjects);
		for (auto it : render3DList)
		{
			gEnv->pEngine->Register3DUIs(it.first.c_str(), it.second);
		}
		//uiObjects is invalidated.
	}
}

//---------------------------------------------------------------------------
bool UIManager::ParseUI(const char* filepath, WinBases& windows, std::string& uiname, bool luaUI)
{

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
	if (sz)
	{
		int error = luaL_dofile(mL, sz);
		if (error)
		{
			char buf[1024];
			sprintf_s(buf, "\n%s/%s", GetCWD(), lua_tostring(mL, -1));
			Error(DEFAULT_DEBUG_ARG, buf);
			assert(0);
			return false;
		}
		scriptPath = sz;
		ToLowerCase(scriptPath);
	}

	bool render3d = false;
	sz = pRoot->Attribute("render3d");
	if (sz)
	{
		render3d = StringConverter::parseBool(sz);
	}
	Vec2I renderTargetSize(100, 100);
	sz = pRoot->Attribute("renderTargetSize");
	if (sz)
	{
		renderTargetSize = StringConverter::parseVec2I(sz);
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

		ComponentType::Enum type = ComponentType::ConverToEnum(sz);

		IWinBase* p = AddWindow(type);
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
		IUIAnimation* pAnim = FB_NEW(UIAnimation);
		pAnim->LoadFromXML(animElem);
		std::string name = pAnim->GetName();
		auto it = mAnimations.Find(name);
		if (it  != mAnimations.end())
		{
			FB_DELETE(it->second);
			mAnimations.erase(it);
			Log("UI global animation %s is replaced", name.c_str());
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

		for (const auto& topWindow : windows)
		{
			auto eventHandler = dynamic_cast<EventHandler*>(topWindow);
			if (eventHandler)
				eventHandler->OnEvent(IEventHandler::EVENT_ON_LOADED);
		}

	}

	return true;
}

//---------------------------------------------------------------------------

void UIManager::CloneUI(const char* uiname, const char* newUIname)
{
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
	if (sz)
	{
		Error(DEFAULT_DEBUG_ARG, "cannot clone scriptable ui");
		return;
	}

	bool render3d = false;
	sz = pRoot->Attribute("render3d");
	if (sz)
	{
		render3d = StringConverter::parseBool(sz);
	}
	Vec2I renderTargetSize(100, 100);
	sz = pRoot->Attribute("renderTargetSize");
	if (sz)
	{
		renderTargetSize = StringConverter::parseVec2I(sz);
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

		ComponentType::Enum type = ComponentType::ConverToEnum(sz);

		IWinBase* p = AddWindow(type);
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
		auto eventHandler = dynamic_cast<EventHandler*>(topWindow);
		if (eventHandler)
			eventHandler->OnEvent(IEventHandler::EVENT_ON_LOADED);
	}
}

//---------------------------------------------------------------------------
void UIManager::IgnoreInput(bool ignore, IWinBase* modalWindow)
{
	ignore ? mIgnoreInput++ : mIgnoreInput--;
	mModelWindow = modalWindow;
}

void UIManager::ToggleVisibleLuaUI(const char* uiname)
{
	bool visible = GetVisible(uiname);
	visible = !visible;
	SetVisible(uiname, visible);
}

//---------------------------------------------------------------------------
bool UIManager::AddLuaUI(const char* uiName, LuaObject& data)
{
	std::string lower = uiName;
	ToLowerCase(lower);
	auto it = mLuaUIs.find(uiName);
	if (it != mLuaUIs.end())
	{
		Error("Already registered!");
		return false;
	}
	std::string typeText = data.GetField("type_").GetString();
	auto type = ComponentType::ConverToEnum(typeText.c_str());
	
	IWinBase* p = AddWindow(0.f, 0.f, 0.1f, 0.1f, type);
	assert(p);
	p->ParseLua(data);

	mLuaUIs[lower].push_back(p);
	return p!=0;
}

void UIManager::DeleteLuaUI(const char* uiName)
{
	std::string lower = uiName;
	ToLowerCase(lower);
	auto it = mLuaUIs.find(lower);
	if (it != mLuaUIs.end())
	{
		for (auto& win : it->second)
		{
			DeleteWindow(win);
		}
		mLuaUIs.erase(it);
	}
	DirtyRenderList();
}

bool UIManager::IsLoadedUI(const char* uiName)
{
	std::string lower = uiName;
	ToLowerCase(lower);
	auto it = mLuaUIs.find(lower);
	return it != mLuaUIs.end();
}

//---------------------------------------------------------------------------
IWinBase* UIManager::AddWindow(int posX, int posY, int width, int height, ComponentType::Enum type)
{
	IWinBase* pWnd = CreateComponent(type);
	
	if (pWnd !=0)
	{
		mWindows.push_back(pWnd);
		pWnd->SetSize(Vec2I(width, height));
		pWnd->SetPos(Vec2I(posX, posY));
	}
	mNeedToRegisterUIObject = true;
	return pWnd;
}

IWinBase* UIManager::AddWindow(float posX, float posY, float width, float height, ComponentType::Enum type)
{
	IWinBase* pWnd = CreateComponent(type);

	if (pWnd!=0)
	{
		mWindows.push_back(pWnd);
		pWnd->SetNSize(Vec2(width, height));
		pWnd->SetNPos(Vec2(posX, posY));
	}
	mNeedToRegisterUIObject = true;
	return pWnd;
}

IWinBase* UIManager::AddWindow(ComponentType::Enum type)
{
	IWinBase* pWnd = CreateComponent(type);

	if (pWnd != 0)
	{
		mWindows.push_back(pWnd);
	}
	mNeedToRegisterUIObject = true;
	return pWnd;
}

void UIManager::DeleteWindow(IWinBase* pWnd)
{
	if (!pWnd)
		return;
	OnDeleteWinBase(pWnd);
	mWindows.erase(std::remove(mWindows.begin(), mWindows.end(), pWnd), mWindows.end());
	FB_SAFE_DEL(pWnd);
	DirtyRenderList();	
}

// deleting component or wnd
void UIManager::OnDeleteWinBase(IWinBase* winbase)
{
	if (!winbase)
		return;


	mNeedToRegisterUIObject = true;
	if (winbase->GetFocus(true))
		mFocusWnd = 0;

	if (winbase->GetRender3D())
	{
		gFBEnv->pEngine->Unregister3DUIs(winbase->GetName());
	}
}

void UIManager::SetFocusUI(IWinBase* ui)
{
	if (!ui || !ui->GetVisible())
		return;
	if (ValueNotExistInVector(mSetFocusReserved, ui))
		mSetFocusReserved.push_back(ui);
}

IWinBase* UIManager::GetFocusUI() const
{
	return mFocusWnd;
}

void UIManager::SetFocusUI(const char* uiName)
{
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
	}
	
}

bool UIManager::IsFocused(const IWinBase* pWnd) const
{
	return pWnd == mFocusWnd;
}

void UIManager::DirtyRenderList()
{
	mNeedToRegisterUIObject = true;
}

void UIManager::SetUIProperty(const char* uiname, const char* compname, const char* prop, const char* val)
{
	SetUIProperty(uiname, compname, UIProperty::ConverToEnum(prop), val);
}
void UIManager::SetUIProperty(const char* uiname, const char* compname, UIProperty::Enum prop, const char* val)
{
	if_assert_fail(uiname && compname && val)
		return;

	auto comp = FindComp(uiname, compname);
	if (comp)
	{
		comp->SetProperty(prop, val);
	}
	else
	{
		Error("Cannot find ui comp(%s) in ui(%s) to set uiproperty(%s).", compname, uiname, UIProperty::ConvertToString(prop));
	}
}

//---------------------------------------------------------------------------
IWinBase* UIManager::CreateComponent(ComponentType::Enum type)
{
	IWinBase* pWnd = 0;
	switch(type)
	{
	case ComponentType::Window:
		pWnd = FB_NEW(Wnd);
		break;
	case ComponentType::TextField:
		pWnd = FB_NEW(TextField);
		break;
	case ComponentType::StaticText:
		pWnd = FB_NEW(StaticText);
		break;
	case ComponentType::Button:
		pWnd = FB_NEW(Button);
		break;
	case ComponentType::ImageBox:
		pWnd = FB_NEW(ImageBox);
		break;
	case ComponentType::CheckBox:
		pWnd = FB_NEW(CheckBox);
		break;
	case ComponentType::ListBox:
		pWnd = FB_NEW(ListBox);
		break;
	case ComponentType::ListItem:
		pWnd = FB_NEW(ListItem);
		break;
	case ComponentType::FileSelector:
		pWnd = FB_NEW(FileSelector);
		break;
	case ComponentType::Scroller:
		pWnd = FB_NEW(Scroller);
		break;
	case ComponentType::RadioBox:
		pWnd = FB_NEW(RadioBox);
		break;
	case ComponentType::Hexagonal:
		pWnd = FB_NEW(HexagonalContextMenu);
		break;
	case ComponentType::CardScroller:
		pWnd = FB_NEW(CardScroller);
		break;
	case ComponentType::CardItem:
		pWnd = FB_NEW(CardItem);
		break;
	case ComponentType::VerticalGauge:
		pWnd = FB_NEW(VerticalGauge);
		break;
	case ComponentType::HorizontalGauge:
		pWnd = FB_NEW(HorizontalGauge);
		break;
	case ComponentType::NumericUpDown:
		pWnd = FB_NEW(NumericUpDown);
		break;
	case ComponentType::DropDown:
		pWnd = FB_NEW(DropDown);
		break;
	case ComponentType::TextBox:
		pWnd = FB_NEW(TextBox);
		break;
	case ComponentType::ColorRamp:
		pWnd = FB_NEW(ColorRampComp);
		break;
	case ComponentType::NamedPortrait:
		pWnd = FB_NEW(NamedPortrait);
		break;
	default:
		assert(0 && "Unknown component");
	}
	return pWnd;
}

//---------------------------------------------------------------------------
void UIManager::DeleteComponent(IWinBase* com)
{
	FB_DELETE(com);
}

//---------------------------------------------------------------------------
void UIManager::OnInput(IMouse* pMouse, IKeyboard* pKeyboard)
{
	if (!pMouse->IsValid() && !pKeyboard->IsValid())
		return;

	if (pKeyboard->IsValid() && pKeyboard->IsKeyPressed(VK_ESCAPE))
	{
		WINDOWS::reverse_iterator it = mWindows.rbegin(), itEnd = mWindows.rend();
		int i = 0;
		for (; it != itEnd; ++it)
		{
			if ((*it)->GetVisible() && (*it)->GetCloseByEsc())
			{
				(*it)->SetVisible(false);
				break;
			}
		}
	}

	if (mIgnoreInput)
	{
		if (mModelWindow)
			mModelWindow->OnInputFromHandler(pMouse, pKeyboard);
		return;
	}

	if (pMouse->IsValid() && pMouse->IsLButtonClicked())
	{
		auto it = mWindows.rbegin(), itEnd = mWindows.rend();
		IWinBase* focusWnd = 0;
		Vec2 mousepos = pMouse->GetNPos();
		for (; it!=itEnd && !focusWnd; it++)
		{
			focusWnd = (*it)->FocusTest(pMouse);
		}
		if (mFocusWnd!=focusWnd)
		{
			SetFocusUI(focusWnd);
		}
	}

	mMouseIn = false;
	WINDOWS::reverse_iterator it = mWindows.rbegin(), itEnd = mWindows.rend();
	int i = 0;
	for (; it != itEnd; ++it)
	{
		if ((*it)->GetVisible())
		{
			mMouseIn = (*it)->OnInputFromHandler(pMouse, pKeyboard) || mMouseIn;
		}

		if (!pMouse->IsValid() && !pKeyboard->IsValid())
			break;
	}

	if (mMouseIn && EventHandler::sLastEventProcess != gpTimer->GetFrame() && pMouse->IsLButtonClicked())
	{		
		LuaObject mouseInvalided;
		mouseInvalided.FindFunction(IUIManager::GetUIManager().GetLuaState(), "OnMouseInvalidatedInUI");
		if (mouseInvalided.IsValid())
		{
			mouseInvalided.Call();
		}
		else
		{
			Log(DEFAULT_DEBUG_ARG, "info : lua function OnMouseInvalidatedInUI is not found.");
		}
	}
}
void UIManager::EnableInputListener(bool enable)
{
	mInputListenerEnable = enable;
}
bool UIManager::IsEnabledInputLIstener() const
{
	return mInputListenerEnable;
}

HCURSOR UIManager::GetMouseCursorOver() const
{
	return WinBase::GetMouseCursorOver();
}

void UIManager::SetMouseCursorOver()
{
	SetCursor(WinBase::GetMouseCursorOver());
}

void UIManager::DisplayMsg(const std::string& msg, ...)
{
	char buf[2048] = {0};
	va_list args;
	va_start(args, msg);
	vsprintf_s(buf, 2048, msg.c_str(), args);
	va_end(args);

	if (strlen(buf)>0)
	{
		Log(buf);
		if (gEnv->pRenderer)
		{
			gEnv->pRenderer->DrawTextForDuration(4.0f, Vec2I(100, 200), 
				buf, fastbird::Color::White);
		}
	}
}

void UIManager::SetTooltipString(const std::wstring& ts)
{
	if (mTooltipText == ts)
	{
		return;
	}
	mTooltipText = ts;
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

void UIManager::ShowTooltip()
{
	Log("Info: void UIManager::ShowTooltip()");
	assert(!mTooltipText.empty());	
	IFont* pFont = gEnv->pRenderer->GetFont();
	pFont->SetHeight(gTooltipFontSize);
	int width = (int)gEnv->pRenderer->GetFont()->GetTextWidth(
		(const char*)mTooltipText.c_str(), mTooltipText.size() * 2);
	pFont->SetBackToOrigHeight();

	const int maxWidth = 350;
	width = std::min(maxWidth, width) + 4;
	mTooltipUI->SetSizeX(width + 16);
	mTooltipTextBox->SetSizeX(width);
	mTooltipTextBox->SetText(mTooltipText.c_str());
	int textWidth = mTooltipTextBox->GetTextWidth();
	mTooltipUI->SetSizeX(textWidth + 16);
	mTooltipTextBox->SetSizeX(textWidth+4);
	int sizeY = mTooltipTextBox->GetPropertyAsInt(UIProperty::SIZEY);
	mTooltipUI->SetSizeY(sizeY + 8);
	mTooltipUI->SetVisible(true);
}

void UIManager::SetTooltipPos(const Vec2& npos)
{
	if (!mTooltipUI->GetVisible())
		return;

	Vec2 backPos = npos;
	if (backPos.y > 0.9f)
	{
		backPos.y -= (gTooltipFontSize * 2.0f+10) / (float)gEnv->pRenderer->GetHeight();
	}
	const Vec2& nSize = mTooltipUI->GetNSize();
	if (backPos.x + nSize.x>1.0f)
	{
		backPos.x -= backPos.x + nSize.x - 1.0f;
	}
	backPos.y += gTooltipFontSize / (float)gEnv->pRenderer->GetHeight();
	mTooltipUI->SetNPos(backPos);
}

void UIManager::CleanTooltip()
{
	if (!mTooltipText.empty())
	{
		mTooltipText.clear();
		mTooltipUI->SetVisible(false);
	}
}

void UIManager::PopupDialog(WCHAR* msg, POPUP_TYPE type, std::function< void(void*) > func)
{
	if (!mPopup)
	{
		mPopup = CreateComponent(ComponentType::Window);
		mPopup->SetNPos(Vec2(0.5f, 0.5f));
		mPopup->SetNSize(Vec2(0.3f, 0.1f));
		mPopup->SetProperty(UIProperty::ALIGNH, "center");
		mPopup->SetProperty(UIProperty::ALIGNV, "middel");
		mPopup->SetProperty(UIProperty::TEXT_ALIGN, "center");
		mPopup->SetProperty(UIProperty::TEXT_VALIGN, "middle");
		mPopup->SetText(msg);

		WinBase* yes = (WinBase*)mPopup->AddChild(0.49f, 0.99f, 0.25f, 0.1f, ComponentType::Button);
		yes->SetName("yes");
		yes->SetAlign(ALIGNH::RIGHT, ALIGNV::BOTTOM);
		yes->SetText(L"Yes");
		yes->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK,
			std::bind(&UIManager::OnPopupYes, this, std::placeholders::_1));

		WinBase* no = (WinBase*)mPopup->AddChild(0.51f, 0.99f, 0.25f, 0.1f, ComponentType::Button);
		no->SetName("no");
		no->SetAlign(ALIGNH::LEFT, ALIGNV::BOTTOM);
		no->SetText(L"No");
		no->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK,
			std::bind(&UIManager::OnPopupNo, this, std::placeholders::_1));
	}
	
	mPopupCallback = func;
	mPopup->SetVisible(true);
}

void UIManager::OnPopupYes(void* arg)
{
	assert(mPopup);
	mPopupResult = 1;
	mPopup->SetVisible(false);
	mPopupCallback(this);
}

void UIManager::OnPopupNo(void* arg)
{
	assert(mPopup);
	mPopupResult = 0;
	mPopup->SetVisible(false);
	mPopupCallback(this);
}

IWinBase* UIManager::FindComp(const char* uiname, const char* compName) const
{
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

void UIManager::FindUIWnds(const char* uiname, WinBases& outV) const
{
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

bool UIManager::CacheListBox(const char* uiname, const char* compName)
{
	mCachedListBox = dynamic_cast<ListBox*>(FindComp(uiname, compName));
	return mCachedListBox != 0;
}

void UIManager::SetVisible(const char* uiname, bool visible)
{
	assert(uiname);
	std::string lower(uiname);
	ToLowerCase(lower);
	auto itFind = mLuaUIs.find(lower);
	if (itFind == mLuaUIs.end())
	{
		Error("UIManager::SetVisible(): UI(%s) is not found.", uiname);
		return;
	}
	for (const auto& comp : itFind->second)
	{
		comp->SetVisible(visible);
		// unregistering 3d uis should be handled in the game code.
		/*if (comp->GetRender3D() && !visible)
		{
			gEnv->pEngine->Unregister3DUIs(comp->GetName());
		}*/
	}


}

void UIManager::LockFocus(bool lock)
{
	mLockFocus = lock;
}

bool UIManager::GetVisible(const char* uiname) const
{
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

void UIManager::CloseAllLuaUI()
{
	for (auto& it : mLuaUIs)
	{
		for (auto& ui : it.second)
		{
			ui->SetVisible(false);
		}
	}
}

const char* UIManager::FindUIFilenameWithLua(const char* luafilepath)
{
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

const char* UIManager::FindUINameWithLua(const char* luafilepath)
{
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

void UIManager::OnUIFileChanged(const char* file)
{
	assert(file);
	std::string lower(file);
	ToLowerCase(lower);

	auto extension = GetFileExtension(lower.c_str());
	std::string uiname;
	std::string filepath = lower;
	if (strcmp(extension, "lua") == 0)
	{
		uiname = FindUINameWithLua(lower.c_str());
		filepath = FindUIFilenameWithLua(lower.c_str());
	}
	else if (strcmp(extension, "ui") == 0)
	{
		uiname = GetFileNameWithoutExtension(lower.c_str());
	}
	else
		return;

	if (uiname.empty())
	{
		LuaObject loadUIFunc;
		loadUIFunc.FindFunction(mL, "LoadLuaUI");
		loadUIFunc.PushToStack();
		auto uiFilePath = ReplaceExtension(file, "ui");
		if (FileSystem::IsFileExisting(uiFilePath.c_str()))
		{
			lua_pushstring(mL, uiFilePath.c_str());
			loadUIFunc.CallWithManualArgs(1, 0);
			uiname = GetFileNameWithoutExtension(uiFilePath.c_str());
			SetVisible(uiname.c_str(), true);
		}
		else if (strcmp(extension, "lua") == 0)
		{
			int error = luaL_dofile(mL, file);
			if (error)
			{
				char buf[1024];
				sprintf_s(buf, "\n%s/%s", GetCWD(), lua_tostring(mL, -1));
				Error(DEFAULT_DEBUG_ARG, buf);
				assert(0);
			}
		}
		return;
	}
		


	auto itFind = mLuaUIs.find(uiname);
	if (itFind != mLuaUIs.end())
	{
		if (GetVisible(uiname.c_str()))
		{
			SetVisible(uiname.c_str(), false);
		}

		for (const auto& ui : itFind->second)
		{
			DeleteWindow(ui);
		}
		mLuaUIs.erase(itFind);
	}
	std::vector<IWinBase*> temp;
	std::string name;
	UIManager::GetUIManagerStatic()->ParseUI(filepath.c_str(), temp, name, true);
	mLuaUIs[uiname] = temp;
	UIManager::GetUIManagerStatic()->SetVisible(uiname.c_str(), false);
	UIManager::GetUIManagerStatic()->SetVisible(uiname.c_str(), true); // for OnVisible UI Event.
}

UIManager* UIManager::GetUIManagerStatic()
{
	return (UIManager*)&IUIManager::GetUIManager();
}

void UIManager::RegisterAlwaysOnTopWnd(IWinBase* win)
{
	if (ValueNotExistInVector(mAlwaysOnTopWindows, win))
		mAlwaysOnTopWindows.push_back(win);
}

void UIManager::UnRegisterAlwaysOnTopWnd(IWinBase* win)
{
	DeleteValuesInVector(mAlwaysOnTopWindows, win);
}

void UIManager::MoveToBottom(const char* moveToBottom)
{
	auto ui = FindComp(moveToBottom, 0);
	if (ui)
	{
		if (ValueNotExistInVector(mMoveToBottomReserved, ui))
			mMoveToBottomReserved.push_back(ui);
	}	
}

void UIManager::HideUIsExcept(const std::vector<std::string>& excepts)
{
	mHideUIExcepts = excepts;
	DirtyRenderList();
}


void UIManager::HighlightUI(const char* uiname)
{
	WinBases topWnds;
	FindUIWnds(uiname, topWnds);
	for (auto& it : topWnds)
	{
		it->StartHighlight(2.0f);
	}
}

void UIManager::StopHighlightUI(const char* uiname)
{
	WinBases topWnds;
	FindUIWnds(uiname, topWnds);
	for (auto& it : topWnds)
	{
		it->StopHighlight();
	}
}

IUIAnimation* UIManager::GetGlobalAnimation(const char* animName)
{
	auto it = mAnimations.Find(animName);
	if (it == mAnimations.end())
		return 0;

	return it->second;
}

IUIAnimation* UIManager::GetGlobalAnimationOrCreate(const char* animName)
{
	auto anim = GetGlobalAnimation(animName);
	if (!anim)
	{
		anim = FB_NEW(UIAnimation);
		anim->SetName(animName);
		mAnimations.Insert(std::make_pair(animName, anim));
	}
	return anim;
}

void UIManager::PrepareTooltipUI()
{
	LuaObject tooltip;
	tooltip.NewTable(mL);
	tooltip.SetField("name", "MouseTooltip");
	tooltip.SetField("type_", "Window");
	tooltip.SetField("pos", Vec2I(0, 0));
	tooltip.SetField("size", Vec2I(366, (int)gTooltipFontSize+8));
	tooltip.SetField("HIDE_ANIMATION", "1");
	tooltip.SetField("SHOW_ANIMATION", "1");
	tooltip.SetField("NO_BACKGROUND", "false");
	tooltip.SetField("BACK_COLOR", "0, 0, 0, 0.8f");
	tooltip.SetField("ALWAYS_ON_TOP", "true");
	tooltip.SetField("USE_BORDER", "true");
	auto tooltipChildren = tooltip.SetFieldTable("children");
	auto children1 = tooltipChildren.SetSeqTable(1);
	children1.SetField("name", "TooltipTextBox");
	children1.SetField("type_", "TextBox");
	children1.SetField("pos", Vec2I(6, 4));
	children1.SetField("size", Vec2I(350, (int)gTooltipFontSize));
	children1.SetField("TEXTBOX_MATCH_HEIGHT", "true");
	children1.SetField("TEXT_SIZE", StringConverter::toString(gTooltipFontSize).c_str());	
	bool success = AddLuaUI("MouseTooltip", tooltip);
	if (!success)
	{
		Error(DEFAULT_DEBUG_ARG, "Cannot create MouseTooltip UI.");
	}
	else
	{
		WinBases wnds;
		FindUIWnds("MouseTooltip", wnds);
		assert(!wnds.empty());
		mTooltipUI = wnds[0];
		mTooltipTextBox = FindComp("MouseTooltip", "TooltipTextBox");
		assert(mTooltipTextBox);
	}
}

} // namespace fastbird