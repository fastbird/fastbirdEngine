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

	void SetWindowPos(HWindow handle, Vec2ITuple pos) {
#if defined(_PLATFORM_WINDOWS_)
		RECT originalRect;
		HWND hwnd = (HWND)handle;
		GetWindowRect(hwnd, &originalRect);
		auto resolX = originalRect.right - originalRect.left;
		auto resolY = originalRect.bottom - originalRect.top;
		RECT rect;
		rect.left = std::get<0>(pos);
		rect.top = std::get<1>(pos);
		rect.right = rect.left + resolX;
		rect.bottom = rect.top + resolY;		
		SetWindowPos(hwnd, 0, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 0);
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

	Vec2ITuple GetForgroundWindowClientSize(){
		auto wnd = GetForegroundWindow();
		return GetWindowClientSize((HWindow)wnd);
	}

	Vec2ITuple GetForgroundWindowSize(){
#if defined(_PLATFORM_WINDOWS_)
		auto wnd = GetForegroundWindow();
		RECT rect;
		GetWindowRect(wnd, &rect);
		return Vec2ITuple(rect.right - rect.left, rect.bottom - rect.top);
#else
		assert(0 && "Not implemented");
#endif
	}

	unsigned GetWindowStyle(HWindow handle){
		return GetWindowLongPtr((HWND)handle, GWL_STYLE);
	}

	std::string OpenFile(HWindow hwnd, char* filter){
		char masterDir[512];
		GetCurrentDirectory(512, masterDir);
		static std::string sLastDir = masterDir;
		char szFile[260];
		OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = (HWND)hwnd;
		ofn.lpstrFile = szFile;
		// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
		// use the contents of szFile to initialize itself.
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter; // "All\0*.*\0Text\0*.TXT\0\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = sLastDir.c_str();
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if (GetOpenFileName(&ofn)){
			char newDirectory[512];
			GetCurrentDirectory(512, newDirectory);
			sLastDir = newDirectory;
			SetCurrentDirectory(masterDir);
			char relative[512];
			PathRelativePathTo(relative, masterDir, FILE_ATTRIBUTE_DIRECTORY, szFile, FILE_ATTRIBUTE_NORMAL);
			return std::string(relative);
		}
		char newDirectory[512];
		GetCurrentDirectory(512, newDirectory);
		sLastDir = newDirectory;
		SetCurrentDirectory(masterDir);
		return std::string();
	}
}