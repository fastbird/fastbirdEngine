#include <UI/StdAfx.h>
#include "Wnd.h"
#include <Engine/GlobalEnv.h>

namespace fastbird
{

Wnd::Wnd()
{
	mUIObject = IUIObject::CreateUIObject(false);
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
}

Wnd::~Wnd()
{

}

void Wnd::GatherVisit(std::vector<IUIObject*>& v)
{
	if (!mVisible)
		return;

	v.push_back(mUIObject);
	__super::GatherVisit(v);
}

bool Wnd::OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard)
{
	if (!mVisible)
		return false;

	bool mouseIn = __super::OnInputFromHandler(mouse, keyboard);

	if (!WinBase::GetFocus())
		return mouseIn;

	if (keyboard->IsValid())
	{
		char c = (char)keyboard->GetChar();
		keyboard->Invalidate();
	}

	return mouseIn;
}

}