#include <UI/StdAfx.h>
#include <UI/StaticText.h>
#include <UI/KeyboardCursor.h>
#include <Engine/GlobalEnv.h>
#include <UI/IUIManager.h>

namespace fastbird
{

StaticText::StaticText()
	: WinBase()
	, mCursorPos(0)
	, mPasswd(false)
{
	mUIObject = gFBEnv->pEngine->CreateUIObject(false, GetRenderTargetSize());
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	mUIObject->SetTextColor(mTextColor);
}

StaticText::~StaticText()
{
}

void StaticText::GatherVisit(std::vector<IUIObject*>& v)
{
	if (!mVisibility.IsVisible())
		return;
	v.push_back(mUIObject);	
}

}