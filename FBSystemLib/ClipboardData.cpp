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
#include "ClipboardData.h"
namespace fb{
	std::string GetClipboardDataAsString(HWindow hwnd)
	{
		if (OpenClipboard((HWND)hwnd))
		{
			unsigned t = EnumClipboardFormats(0);
			while (t != CF_TEXT && t != 0)
			{
				t = EnumClipboardFormats(t);
			}
			if (t == CF_TEXT)
			{
				auto handle = GetClipboardData(CF_TEXT);
				if (handle)
				{
					auto data = GlobalLock(handle);
					std::string str((const char*)data);
					GlobalUnlock(handle);
					CloseClipboard();
					return str;
				}
			}
			CloseClipboard();
		}
		return std::string();
	}

	void SetClipboardStringData(HWindow hwnd, const char* data){
		if (OpenClipboard((HWND)hwnd))
		{
			EmptyClipboard();

			auto buf = GlobalAlloc(GMEM_MOVEABLE, strlen(data) + 1);
			if (buf == NULL){
				CloseClipboard();
				return;
			}
			auto dest = GlobalLock(buf);
			memcpy(dest, data, strlen(data) + 1);
			GlobalUnlock(buf);
			SetClipboardData(CF_TEXT, buf);
			CloseClipboard();
		}
	}
}