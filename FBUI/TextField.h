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

#include "WinBase.h"
#include "FBInputManager/ITextManipulatorObserver.h"

namespace fb
{
FB_DECLARE_SMART_PTR(PropertyList);
FB_DECLARE_SMART_PTR(ListBox);
FB_DECLARE_SMART_PTR(TextField);
class FB_DLL_UI TextField : public WinBase, public ITextManipulatorObserver
{
protected:
	TextField();
	~TextField();

public:
	static TextFieldPtr Create();

	// IWinBase
	ComponentType::Enum GetType() const { return ComponentType::TextField; }
	void GatherVisit(std::vector<UIObject*>& v);
	bool OnInputFromHandler(IInputInjectorPtr injector);
	void SetText(const wchar_t* szText);

	void OnFocusLost();
	void OnFocusGain();

	void SetPasswd(bool passwd);

	// ITextManipulatorListener
	void OnCursorPosChanged(TextManipulator* mani);
	void OnTextChanged(TextManipulator* mani);
	PropertyListPtr IsInPropertyList() const;
	ListBoxPtr IsInListBox() const;
	void SelectAll();

	void OnClicked(void* arg);
	void OnDoubleClicked(void* arg);
	
	

protected:
	const static float LEFT_GAP;
	void SetUseBorder(bool use);
	void OnPosChanged(bool anim);
	void OnSizeChanged();

private:
	bool mPasswd;
	int mCursorOffset;
};

}