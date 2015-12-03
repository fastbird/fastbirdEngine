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

#include "stdafx.h"
#include "System.h"

namespace fb{
	int GetNumProcessors(){
#if defined(_PLATFORM_WINDOWS_)
		SYSTEM_INFO SI;
		GetSystemInfo(&SI);
		return SI.dwNumberOfProcessors;
#else
		assert(0 && "Not implemented");
		return 4;
#endif
	}

	bool IsWindowForeground(HWindow window){
#if defined(_PLATFORM_WINDOWS_)
		return GetForegroundWindow() == (HWND)window;
#else
		assert(0 && "Not implemented");
		return true;
#endif
	}

	HWindow ForegroundWindow(){
#if defined(_PLATFORM_WINDOWS_)
		return (HWindow)GetForegroundWindow();
#else
		assert(0 && "Not implemented");
		return -1;
#endif
	}

	HWindow WindowFromMousePosition(){
#if defined(_PLATFORM_WINDOWS_)
		POINT pt;
		GetCursorPos(&pt);
		return (HWindow)WindowFromPoint(pt);
#else
		assert(0 && "Not implemented");
		return -1;
#endif
	}

	void ChangeWindowSize(HWindow handle, Vec2ITuple resol){
#if defined(_PLATFORM_WINDOWS_)
		RECT originalRect;
		HWND hwnd = (HWND)handle;
		GetWindowRect(hwnd, &originalRect);
		RECT rect;
		rect.left = originalRect.left;
		rect.top = originalRect.right;
		rect.right = rect.left + std::get<0>(resol);
		rect.bottom = rect.top + std::get<1>(resol);
		AdjustWindowRect(&rect, GetWindowLongPtr(hwnd, GWL_STYLE), FALSE);
		SetWindowPos(hwnd, 0, originalRect.left, originalRect.top, rect.right - rect.left, rect.bottom - rect.top, 0);		
#else
		assert(0 && "Not implemented");		
#endif
	}

	Vec2ITuple GetWindowClientSize(HWindow handle){
#if defined(_PLATFORM_WINDOWS_)
		RECT rect;
		GetClientRect((HWND)handle, &rect);
		return Vec2ITuple{ rect.right - rect.left, rect.bottom - rect.top };
#else
#endif
	}

	unsigned GetWindowStyle(HWindow handle){
		return GetWindowLongPtr((HWND)handle, GWL_STYLE);
	}
}