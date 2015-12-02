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

#include "Container.h"

namespace fb
{
	FB_DECLARE_SMART_PTR(Wnd);
	FB_DECLARE_SMART_PTR(Button);
	FB_DECLARE_SMART_PTR(DropDown);
	class FB_DLL_UI DropDown : public Container
	{
	protected:
		DropDown();		

	public:
		static const int ITEM_HEIGHT;
		
		static DropDownPtr Create();

		// IWinBase
		void OnCreated();
		void OnSizeChanged();
		ComponentType::Enum GetType() const { return ComponentType::DropDown; }
		void GatherVisit(std::vector<UIObject*>& v);
		bool SetProperty(UIProperty::Enum prop, const char* val);
		bool GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly);
		size_t AddDropDownItem(WCHAR* szString);
		// alread added as a child
		size_t AddDropDownItem(WinBasePtr item);
		void ClearDropDownItems();
		size_t GetSelectedIndex() const;
		void SetSelectedIndex(size_t index);
		void SetReservedIndex(size_t index);
		void OnFocusLost();
		bool OnInputFromHandler(IInputInjectorPtr injector);
		void OnParentVisibleChanged(bool show);		
		void ModifyItem(unsigned index, UIProperty::Enum, const char* szString);
		const wchar_t* GetItemString(unsigned index);

	protected:
		void SetCommonProperty(WinBasePtr item, size_t index);
		const static float LEFT_GAP;

	private:
		void OnMouseClick(void* arg);
		void OnItemSelected(void* arg);

		void CloseOptions();
		void SetVisibleDropDownItems(bool visible);

	private:
		int mCursorPos;
		bool mPasswd;
		ButtonWeakPtr mButton;
		std::vector<ButtonWeakPtr> mDropDownItems;
		size_t mCurIdx;
		size_t mReservedIdx;
		WndWeakPtr mHolder;
		int mMaxHeight;
		bool mTriggerEvent;

		static DropDownWeakPtr sCurrentDropDown;
		friend class UIManager;
	};

}