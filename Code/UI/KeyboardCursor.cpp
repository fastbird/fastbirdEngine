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
	mUIObject = IUIObject::CreateUIObject(false, Vec2I(gEnv->pRenderer->GetWidth(), gEnv->pRenderer->GetHeight()));
	mUIObject->SetMaterial("es/materials/KeyboardCursor.material");
	mUIObject->SetDebugString("KeyboardCursor");
}

KeyboardCursor::~KeyboardCursor()
{
	FB_RELEASE(mUIObject);
}

//---------------------------------------------------------------------------------------
void KeyboardCursor::SetNPos(const Vec2& pos)
{
	mUIObject->SetNPos(pos);
}

void KeyboardCursor::SetNSize(const Vec2& size)
{
	mUIObject->SetNSize(size);
}

IUIObject* KeyboardCursor::GetUIObject() const
{
	return mUIObject;
}

}