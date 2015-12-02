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
#include "IInputConsumer.h"
#include "ITextManipulatorObserver.h"
#include "FBCommonHeaders/Observable.h"
namespace fb{
	class IKeyboard;
	class IMouse;	
	FB_DECLARE_SMART_PTR(TextManipulator);
	class FB_DLL_INPUTMANAGER TextManipulator : public IInputConsumer, public Observable<ITextManipulatorObserver>
	{
		int mCursorPos;
		int mHighlightStart;
		std::wstring* mText;

		TextManipulator();
		~TextManipulator();

	private:
		
		void EndHighlighting() { mHighlightStart = -1; }
		void OnCursorPosChanged();
		void OnTextChanged();
		void Highlighting(bool shiftkey);
		void StartHighlighting();
		

	public:
		
		static TextManipulatorPtr Create();
		// Input Consumer
		virtual void ConsumeInput(IInputInjectorPtr injector);	

		virtual void SetText(std::wstring* text);				
		virtual int GetCursorPos() const;
		virtual void SetCursorPos(int pos);
		bool IsHighlighting() const { return mHighlightStart != -1; }
		virtual int GetHighlightStart() const { return mHighlightStart; }
		virtual void SelectAll();
	};
}