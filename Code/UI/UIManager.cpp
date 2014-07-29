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

namespace fastbird
{

IUIManager* IUIManager::mUIManager = 0;

void IUIManager::InitializeUIManager()
{
	assert(!mUIManager);
	mUIManager = new UIManager;
	WinBase::InitMouseCursor();
}

void IUIManager::FinalizeUIManager()
{
	WinBase::FinalizeMouseCursor();
	assert(mUIManager);
	SAFE_DELETE(mUIManager);
}

IUIManager& IUIManager::GetUIManager()
{
	assert(mUIManager);
	return *mUIManager;
}

//---------------------------------------------------------------------------
UIManager::UIManager()
	: mInputListenerEnable(true)
	, mNeedToRegisterUIObject(false)
	, mFocusWnd(0)
{
	gEnv->pEngine->AddInputListener(this,
		fastbird::IInputListener::INPUT_LISTEN_PRIORITY_UI, 0);
	KeyboardCursor::InitializeKeyboardCursor();
	
}

UIManager::~UIManager()
{
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
		delete (*it);
	}
	mWindows.clear();
}

//---------------------------------------------------------------------------
void UIManager::Update()
{
	for each(auto wnd in mWindows)
	{
		wnd->OnStartUpdate();
	}
	if (mNeedToRegisterUIObject)
	{
		mNeedToRegisterUIObject = false;
		std::vector<IUIObject*> uiObjects;
		uiObjects.reserve(50);
		WINDOWS::iterator it = mWindows.begin(), itEnd = mWindows.end();
		for (; it!=itEnd; it++)
		{
			if ((*it)->GetVisible())
				(*it)->GatherVisit(uiObjects);
		}
		// rendering order : reverse.
		gEnv->pEngine->RegisterUIs(uiObjects);
		//uiObjects is invalidated.
	}
}

//---------------------------------------------------------------------------
IWinBase* UIManager::AddWindow(int posX, int posY, int width, int height, ComponentType::Enum type)
{
	IWinBase* pWnd = CreateComponent(type);
	
	if (pWnd !=0)
	{
		mWindows.push_front(pWnd);
		pWnd->SetPos(Vec2I(posX, posY));
		pWnd->SetSize(Vec2I(width, height));
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
		pWnd->SetNPos(Vec2(posX, posY));
		pWnd->SetNSize(Vec2(width, height));
	}
	mNeedToRegisterUIObject = true;
	return pWnd;
}

void UIManager::DeleteWindow(IWinBase* pWnd)
{
	OnDeleteWinBase(pWnd);
	delete pWnd;
	mWindows.erase(std::remove(mWindows.begin(), mWindows.end(), pWnd), mWindows.end());
	
}

// deleting component or wnd
void UIManager::OnDeleteWinBase(IWinBase* winbase)
{
	mNeedToRegisterUIObject = true;
	if (winbase==mFocusWnd)
	{
		mFocusWnd = 0;
	}
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
		mWindows.splice(mWindows.begin(), mWindows, f);
	}
	mNeedToRegisterUIObject = true;
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
		pWnd = new Wnd();
		break;
	case ComponentType::TextField:
		pWnd = new TextField();
		break;
	case ComponentType::StaticText:
		pWnd = new StaticText();
		break;
	case ComponentType::Button:
		pWnd = new Button();
		break;
	case ComponentType::ImageBox:
		pWnd = new ImageBox();
		break;
	case ComponentType::CheckBox:
		pWnd = new CheckBox();
		break;
	case ComponentType::ListBox:
		pWnd = new ListBox();
		break;
	case ComponentType::ListItem:
		pWnd = new ListItem();
		break;
	case ComponentType::FileSelector:
		pWnd = new FileSelector;
		break;
	case ComponentType::Scroller:
		pWnd = new Scroller;
		break;
	case ComponentType::RadioBox:
		pWnd = new RadioBox;
		break;
	default:
		assert(0 && "Unknown component");
	}
	return pWnd;
}

//---------------------------------------------------------------------------
void UIManager::OnInput(IMouse* pMouse, IKeyboard* pKeyboard)
{
	if (!pMouse->IsValid() && !pKeyboard->IsValid())
		return;

	if (pMouse->IsValid() && pMouse->IsLButtonClicked())
	{
		WINDOWS::iterator it = mWindows.begin(), itEnd = mWindows.end();
		IWinBase* focusWnd = 0;
		Vec2 mousepos = pMouse->GetNPos();
		for (; it!=itEnd && !focusWnd; it++)
		{
			focusWnd = (*it)->FocusTest(mousepos);
		}
		if (mFocusWnd!=focusWnd)
		{
			SetFocusUI(focusWnd);
		}
	}

	WINDOWS::iterator it = mWindows.begin(),itEnd = mWindows.end();
	for (; it!=itEnd; it++)
	{
		if ((*it)->GetVisible())
			(*it)->OnInputFromHandler(pMouse, pKeyboard);

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

} // namespace fastbird

