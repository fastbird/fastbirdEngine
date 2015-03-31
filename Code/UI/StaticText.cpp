#include <UI/StdAfx.h>
#include <UI/StaticText.h>
#include <UI/KeyboardCursor.h>
#include <Engine/GlobalEnv.h>
#include <UI/IUIManager.h>

namespace fastbird
{

const float StaticText::LEFT_GAP = 0.001f;

StaticText::StaticText()
	: WinBase()
	, mCursorPos(0)
	, mPasswd(false)
{
	mUIObject = IUIObject::CreateUIObject(false, GetRenderTargetSize());
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	mUIObject->SetTextColor(mTextColor);
	mUIObject->SetNoDrawBackground(true);
}

StaticText::~StaticText()
{
}

void StaticText::GatherVisit(std::vector<IUIObject*>& v)
{
	if (!mVisible)
		return;
	v.push_back(mUIObject);	
}

void StaticText::OnPosChanged()
{
	WinBase::OnPosChanged();
	AlignText();
}

void StaticText::OnSizeChanged()
{
	WinBase::OnSizeChanged();
	AlignText();
}
}