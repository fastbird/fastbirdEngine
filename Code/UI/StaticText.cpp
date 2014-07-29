#include <UI/StdAfx.h>
#include <UI/StaticText.h>
#include <UI/KeyboardCursor.h>
#include <Engine/GlobalEnv.h>
#include <UI/IUIManager.h>

namespace fastbird
{

const float StaticText::LEFT_GAP = 0.001f;
const float StaticText::BOTTOM_GAP = 0.004f;

StaticText::StaticText()
	: WinBase()
	, mCursorPos(0)
	, mPasswd(false)
{
	mUIObject = IUIObject::CreateUIObject();
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ToString(GetType());
	mUIObject->SetTextColor(mTextColor);
	mUIObject->SetNoDrawBackground(true);
}

StaticText::~StaticText()
{
}

void StaticText::GatherVisit(std::vector<IUIObject*>& v)
{
	v.push_back(mUIObject);	
}

void StaticText::OnPosChanged()
{
	WinBase::OnPosChanged();
	mUIObject->SetTextStartNPos(Vec2(mWNPos.x, mWNPos.y + mWNSize.y - BOTTOM_GAP));
}

void StaticText::OnSizeChanged()
{
	WinBase::OnSizeChanged();
	mUIObject->SetTextStartNPos(Vec2(mWNPos.x, mWNPos.y + mWNSize.y - BOTTOM_GAP));
}
}