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
#include <CommonLib/FileSystem.h>
#include <CommonLib/LuaUtils.h>
#include <CommonLib/LuaObject.h>
#include <Engine/IUIObject.h>

namespace fastbird
{

IUIManager* IUIManager::mUIManager = 0;

void IUIManager::InitializeUIManager(lua_State* L)
{
	assert(!mUIManager);
	mUIManager = FB_NEW(UIManager)(L);
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
	, mPosSizeEventEnabled(true)
{
	gpTimer = gEnv->pTimer;
	gEnv->pEngine->AddInputListener(this,
		fastbird::IInputListener::INPUT_LISTEN_PRIORITY_UI, 0);
	KeyboardCursor::InitializeKeyboardCursor();
	mTooltipUI = IUIObject::CreateUIObject(false, Vec2I(gEnv->pRenderer->GetWidth(), gEnv->pRenderer->GetHeight()));
	mTooltipUI->mOwnerUI = 0;
	mTooltipUI->mTypeString = "TooltipUI";	
	mTooltipUI->SetAlphaBlending(true);
	mTooltipUI->GetMaterial()->SetDiffuseColor(Vec4(0, 0, 0, 0.7f));
	mTooltipUI->SetTextColor(Color(1, 1, 1, 1));
	RegisterLuaFuncs(L);
}

UIManager::~UIManager()
{
	DeleteWindow(mPopup);
	mTooltipUI->Delete();
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
	for each(auto& wnd in mWindows)
	{
		wnd->OnStartUpdate(elapsedTime);
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

		WINDOWS::iterator it = mWindows.begin(), itEnd = mWindows.end();
		size_t start = 0;
		for (; it != itEnd; it++)
		{
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


		if (!mTooltipText.empty())
			uiObjects.push_back(mTooltipUI);

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
bool UIManager::ParseUI(const char* filepath, std::vector<IWinBase*>& windows, std::string& uiname, bool luaUI)
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

	std::string scriptPath;
	const char* sz = pRoot->Attribute("script");
	if (sz)
	{
		int error = luaL_dofile(mL, sz);
		if (error)
		{
			Log(lua_tostring(mL, -1));
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

	assert(!uiname.empty());
	if (luaUI && !uiname.empty())
	{
		std::string lower = uiname;
		ToLowerCase(lower);
		mLuaUIs[lower].clear();
		for (const auto& topWindow : windows)
		{
			mLuaUIs[lower].push_back(topWindow);
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
		assert(0 && "can not clone scriptable ui");
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
	std::vector<IWinBase*> windows;
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
		mWindows.push_front(pWnd);
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
		mWindows.push_front(pWnd);
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
		mWindows.push_front(pWnd);
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
}

void UIManager::SetFocusUI(IWinBase* ui)
{

	if (mFocusWnd)
		mFocusWnd->OnFocusLost();
	mFocusWnd = ui;
	if (mFocusWnd)
		mFocusWnd->OnFocusGain();

	WINDOWS::iterator f = std::find(mWindows.begin(), mWindows.end(), ui);
	if (f!=mWindows.end())
	{
		mWindows.splice(mWindows.end(), mWindows, f);
	}
	mNeedToRegisterUIObject = true;
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

	SetFocusUI(it->second[0]);
}

bool UIManager::IsFocused(const IWinBase* pWnd) const
{
	return pWnd == mFocusWnd;
}

void UIManager::DirtyRenderList()
{
	mNeedToRegisterUIObject = true;
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
	WINDOWS::reverse_iterator it = mWindows.rbegin(),itEnd = mWindows.rend();
	for (; it!=itEnd; it++)
	{
		if ((*it)->GetVisible())
		{
			mMouseIn = (*it)->OnInputFromHandler(pMouse, pKeyboard) || mMouseIn;
		}

		if (!pMouse->IsValid() && !pKeyboard->IsValid())
			break;
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

static float gTooltipFontSize = 26;
void UIManager::SetTooltipString(const std::wstring& ts)
{
	if (mTooltipText == ts)
		return;
	mTooltipText = ts;
	mTooltipUI->SetText(mTooltipText.c_str());
	mNeedToRegisterUIObject = true;
	
	if (!mTooltipText.empty())
	{
		IFont* pFont = gEnv->pRenderer->GetFont();
		pFont->SetHeight(gTooltipFontSize);
		int width = (int)gEnv->pRenderer->GetFont()->GetTextWidth(
			(const char*)mTooltipText.c_str(), mTooltipText.size() * 2);
		pFont->SetBackToOrigHeight();

		Vec2 nsize = { (width + 4) / (float)gEnv->pRenderer->GetWidth(),
			gTooltipFontSize / (float)gEnv->pRenderer->GetHeight() };

		mTooltipUI->SetNSize(nsize);
		mTooltipUI->SetTextSize(gTooltipFontSize);
	}
}

void UIManager::SetTooltipPos(const Vec2& npos)
{
	Vec2 backPos = npos;
	if (backPos.y > 0.9f)
	{
		backPos.y -= gTooltipFontSize * 2.0f / (float)gEnv->pRenderer->GetHeight();
	}
	const Vec2& nSize = mTooltipUI->GetNSize();
	if (backPos.x + nSize.x>1.0f)
	{
		backPos.x -= backPos.x + nSize.x - 1.0f;
	}
	Vec2 textpos = backPos;
	textpos.y += gTooltipFontSize*2 / (float)gEnv->pRenderer->GetHeight();
	mTooltipUI->SetTextStartNPos(textpos);
	backPos.y += gTooltipFontSize / (float)gEnv->pRenderer->GetHeight();
	mTooltipUI->SetNPos(backPos);
}

void UIManager::CleanTooltip()
{
	if (!mTooltipText.empty())
	{
		mTooltipText.clear();
		mNeedToRegisterUIObject = true;
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

void UIManager::SetVisible(const char* uiname, bool visible)
{
	assert(uiname);
	std::string lower(uiname);
	ToLowerCase(lower);
	auto itFind = mLuaUIs.find(lower);
	if (itFind == mLuaUIs.end())
	{
		assert(0);
		return;
	}
	for (const auto& comp : itFind->second)
	{
		if (comp->GetVisible() != visible)
		{
			comp->SetVisible(visible);
			// unregistering 3d uis should be handled in the game code.
			/*if (comp->GetRender3D() && !visible)
			{
				gEnv->pEngine->Unregister3DUIs(comp->GetName());
			}*/
		}
	}


}

bool UIManager::GetVisible(const char* uiname) const
{
	assert(uiname);
	std::string lower(uiname);
	ToLowerCase(lower);
	auto itFind = mLuaUIs.find(lower.c_str());
	if (itFind == mLuaUIs.end())
	{
		assert(0);
		return false;
	}
	bool visible = false;
	for (const auto& comp : itFind->second)
	{
		visible = visible || comp->GetVisible();
	}
	return visible;
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
			if (strcmp(win->GetScriptPath(), luafilepath) == 0)
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
		return;


	auto itFind = mLuaUIs.find(uiname);
	if (itFind != mLuaUIs.end())
	{
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

} // namespace fastbird