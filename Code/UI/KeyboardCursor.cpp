#include <UI/StdAfx.h>
#include <UI/KeyboardCursor.h>

namespace fastbird
{

//---------------------------------------------------------------------------------------
KeyboardCursor* KeyboardCursor::mInstance = 0;

void KeyboardCursor::InitializeKeyboardCursor()
{
	if (!mInstance)
		mInstance = FB_NEW(KeyboardCursor);
}

KeyboardCursor& KeyboardCursor::GetKeyboardCursor()
{
	assert(mInstance); 
	return *mInstance;
}
void KeyboardCursor::FinalizeKeyboardCursor()
{
	FB_SAFE_DEL(mInstance);
}


//---------------------------------------------------------------------------------------
KeyboardCursor::KeyboardCursor()
	: mVisible(false)
{	
	mUIObject = gFBEnv->pEngine->CreateUIObject(false, 
		gFBEnv->pEngine->GetWindowSize(gFBEnv->pEngine->GetMainWndHandle()));
	mUIObject->SetMaterial("es/materials/KeyboardCursor.material");
	mUIObject->SetDebugString("KeyboardCursor");
}

KeyboardCursor::~KeyboardCursor()
{
	gFBEnv->pEngine->DeleteUIObject(mUIObject);
}

//---------------------------------------------------------------------
void KeyboardCursor::SetPos(const Vec2I& pos){
	mUIObject->SetUIPos(pos);
}

void KeyboardCursor::SetSize(const Vec2I& size){
	mUIObject->SetUISize(size);
}

IUIObject* KeyboardCursor::GetUIObject() const
{
	return mUIObject;
}

void KeyboardCursor::SetHwndId(HWND_ID hwndId)
{
	mUIObject->SetRenderTargetSize(gFBEnv->pRenderer->GetRenderTargetSize(hwndId));
}

void KeyboardCursor::SetScissorRegion(const RECT& r)
{
	mUIObject->SetUseScissor(true, r);
}

}