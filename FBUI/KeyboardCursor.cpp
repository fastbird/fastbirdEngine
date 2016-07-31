/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#include "StdAfx.h"
#include "KeyboardCursor.h"
#include "UIObject.h"
namespace fb
{

//---------------------------------------------------------------------------------------
KeyboardCursorWeakPtr sKeyboardCursor;
KeyboardCursorPtr KeyboardCursor::Create()
{
	if (sKeyboardCursor.expired()){
		KeyboardCursorPtr p(new KeyboardCursor, [](KeyboardCursor* obj){ delete obj; });
		sKeyboardCursor = p;
		return p;
	}

	return sKeyboardCursor.lock();
}

KeyboardCursor& KeyboardCursor::GetInstance()
{
	if (sKeyboardCursor.expired()){
		Logger::Log(FB_ERROR_LOG_ARG, "Keyboard cursor is already deleted. Program will crash...");
	}
	return *sKeyboardCursor.lock();
}

//---------------------------------------------------------------------------------------
KeyboardCursor::KeyboardCursor()
	: mVisible(false)
{	
	mUIObject = UIObject::Create(Renderer::GetInstance().GetMainRenderTargetSize(), 0);
	mUIObject->SetMaterial("EssentialEngineData/materials/KeyboardCursor.material");
	mUIObject->SetDebugString("KeyboardCursor");
}

KeyboardCursor::~KeyboardCursor()
{	
}

//---------------------------------------------------------------------
void KeyboardCursor::SetPos(const Vec2I& pos){
	mUIObject->SetUIPos(pos);
}

void KeyboardCursor::SetSize(const Vec2I& size){
	mUIObject->SetUISize(size);
}

UIObjectPtr KeyboardCursor::GetUIObject() const
{
	return mUIObject;
}

void KeyboardCursor::SetHwndId(HWindowId hwndId)
{
	mUIObject->SetRenderTargetSize(Renderer::GetInstance().GetRenderTargetSize(hwndId));
}

void KeyboardCursor::SetScissorRegion(const Rect& r)
{
	mUIObject->SetUseScissor(true, r);
}

}