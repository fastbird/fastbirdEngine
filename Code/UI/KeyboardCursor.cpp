#include <UI/StdAfx.h>
#include <UI/KeyboardCursor.h>

namespace fastbird
{

//---------------------------------------------------------------------------------------
KeyboardCursor* KeyboardCursor::mInstance = 0;

void KeyboardCursor::InitializeKeyboardCursor()
{
	if (!mInstance)
		mInstance = new KeyboardCursor();
}

KeyboardCursor& KeyboardCursor::GetKeyboardCursor()
{
	assert(mInstance); 
	return *mInstance;
}
void KeyboardCursor::FinalizeKeyboardCursor()
{
	SAFE_DELETE(mInstance);
}


//---------------------------------------------------------------------------------------
KeyboardCursor::KeyboardCursor()
	: mVisible(false)
{
	mUIObject = IUIObject::CreateUIObject();
	mUIObject->SetMaterial("es/materials/KeyboardCursor.material");
	mUIObject->SetDebugString("KeyboardCursor");
}

KeyboardCursor::~KeyboardCursor()
{
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