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

namespace fb
{
	FB_DECLARE_SMART_PTR(UIObject);	
	FB_DECLARE_SMART_PTR(KeyboardCursor);
	class FB_DLL_UI KeyboardCursor
	{	
	protected:
		KeyboardCursor();
		~KeyboardCursor();

	public:		

		static KeyboardCursorPtr Create();
		static KeyboardCursor& GetInstance();

		UIObjectPtr GetUIObject() const;

		void SetPos(const Vec2I& pos);
		void SetSize(const Vec2I& size);
		void SetHwndId(HWindowId hwndId);
		void SetScissorRegion(const Rect& r);

	private:
		UIObjectPtr mUIObject;
		bool mVisible;
		HWindowId mHwndId;
	};
}