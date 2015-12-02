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
#include "UIProperty.h"
#include "FBCommonHeaders/IteratorWrapper.h"
namespace fb
{
	FB_DECLARE_SMART_PTR(WinBase);
	class IUIEditor
	{
	public:
		typedef std::vector<WinBasePtr> Comps;
		virtual void OnComponentSelected(WinBasePtr comp) {}
		virtual void OnComponentDeselected(WinBasePtr comp) {}
		virtual void OnComponentDeleted(WinBasePtr comp){}
		virtual WinBasePtr GetCurSelected() const { return 0; }
		virtual unsigned GetNumCurEditing() const { return 0; }
		virtual WinBasePtr GetCurSelected(unsigned index) const { return 0; }
		virtual IteratorWrapper<Comps> GetSelectedComps() = 0;
		virtual void OnCancelComponent() {}
		virtual void OnPosSizeChanged(){}
		virtual void DrawFocusBoxes(){}
		virtual void TryToDeleteCurComp() {}
		virtual HWindowId GetHwndId() const { return -1; }
		virtual void BackupSizePos(){}
		virtual void RestoreSizePos(){}

		// process for Ctrl + left, right, up, down. and Ctrl+keypad 5
		virtual void ProcessKeyInput(){}

		virtual void Save(){}
		
	};
}