#include <UI/StdAfx.h>
#include "Wnd.h"
#include <Engine/GlobalEnv.h>

namespace fastbird
{

Wnd::Wnd()
{
	mUIObject = IUIObject::CreateUIObject();
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ToString(GetType());
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

void Wnd::OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard)
{
	if (!mVisible)
		return;

	__super::OnInputFromHandler(mouse, keyboard);

	if (!mVisible || !WinBase::GetFocus())
		return;

	if (keyboard->IsValid())
	{
		char c = (char)keyboard->GetChar();
		keyboard->Invalidate();
	}
}

}