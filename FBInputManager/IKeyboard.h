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
#ifndef _fastbird_IKeyboard_header_included_
#define _fastbird_IKeyboard_header_included_

#include "IInputDevice.h"
#include "KeyboardEvent.h"
namespace fb
{
	FB_DECLARE_SMART_PTR(IKeyboard);
	class IKeyboard : public IInputDevice
	{
	public:
		enum KEYBOARD_FLAG
		{
			KEYBOARD_FLAG_KEY_BREAK = 0x01,
			KEYBOARD_FLAG_KEY_E0 = 0x02,
			KEYBOARD_FLAG_KEY_E1 = 0x04,
			KEYBOARD_FLAG_KEY_MAKE = 0,
		};
		virtual ~IKeyboard(){}
		virtual void PushEvent(HWindow hWnd, const KeyboardEvent& keyboardEvent) = 0;
		virtual void PushChar(HWindow hWnd, unsigned keycode, TIME_PRECISION gameTimeInSec) = 0;
		virtual bool IsKeyDown(unsigned short keycode) const = 0;
		virtual bool IsKeyPressed(unsigned short keycode) const = 0;
		virtual bool IsKeyUp(unsigned short keycode) const = 0;
		virtual unsigned GetChar() = 0;
		virtual void PopChar() = 0;
		virtual void ClearBuffer() = 0;
		virtual void ClearKeydown() = 0;

		virtual void OnKillFocus() = 0;		
		virtual void OnGetFocus() = 0;
	};
}


#endif //_fastbird_IKeyboard_header_included_