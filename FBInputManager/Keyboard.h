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

#pragma once
#ifndef _fastbird_Keyboard_header_included_
#define _fastbird_Keyboard_header_included_

#include "IKeyboard.h"
#include <queue>
namespace fb
{
	class Keyboard : public IKeyboard
	{
		class KeyboardImpl;
		KeyboardImpl* mImpl;
		Keyboard(const Keyboard&) = delete;
		Keyboard& Keyboard::operator= (const Keyboard&) = delete;

	public:	
		Keyboard();
		~Keyboard();
		//-------------------------------------------------------------------
		// IInputDevice
		//-------------------------------------------------------------------
		virtual void EndFrame(TIME_PRECISION gameTimeInSecond);
		virtual bool IsValid() const;
		virtual void Invalidate(bool buttonClicked = false);
		virtual void InvalidTemporary(bool invalidate);

		virtual void PushEvent(HWindow hWnd, const KeyboardEvent& keyboardEvent);
		virtual void PushChar(HWindow hWnd, unsigned keycode, TIME_PRECISION gameTimeInSec);
		virtual bool IsKeyDown(unsigned short keycode) const;
		virtual bool IsKeyPressed(unsigned short keycode) const;
		virtual bool IsKeyUp(unsigned short keycode) const;
		virtual unsigned GetChar();
		virtual void PopChar();
		virtual void ClearBuffer();

		virtual void OnKillFocus();
		
	};
}

#endif //_fastbird_Keyboard_header_included_