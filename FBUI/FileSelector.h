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

#include "Wnd.h"

namespace fb
{

FB_DECLARE_SMART_PTR(StaticText);
FB_DECLARE_SMART_PTR(TextField);
FB_DECLARE_SMART_PTR(ListBox);
FB_DECLARE_SMART_PTR(Button);
FB_DECLARE_SMART_PTR(FileSelector);
class FB_DLL_UI FileSelector : public Wnd
{
protected:
	FileSelector();
	~FileSelector();

public:
	static FileSelectorPtr Create();

	virtual void OnCreated();

	// IWinBase
	virtual ComponentType::Enum GetType() const { return ComponentType::FileSelector; }
	virtual void GatherVisit(std::vector<UIObject*>& v);

	// Event
	void OnOK(void* pButton);
	void OnCancel(void* pButton);
	void OnListClick(void* pList);
	void OnListDoubleClick(void* pList);
	void OnDriveClick(void* pButton);

	// IFileSelector
	void SetTitle(const wchar_t* szTitle);
	void ListFiles(const char* folder, const char* filter);
	std::string GetFile() const { return mFile;}


private:
	std::string mFile;
	std::string mFolder;
	std::string mFilter;;
	StaticTextWeakPtr mStaticText;
	std::vector<ButtonWeakPtr> mDriveButtons;
	TextFieldWeakPtr mFileTextField;
	ListBoxWeakPtr mListBox;
	ButtonWeakPtr mOKButton;
	ButtonWeakPtr mCancelButton;
};

}