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
	FB_DECLARE_SMART_PTR(ImageBox);
	FB_DECLARE_SMART_PTR(TextBox);
	class FB_DLL_UI TextBox : public Container
	{
	protected:
		TextBox();
		~TextBox();

	public:
		static TextBoxPtr Create();

		// IWinBase
		ComponentType::Enum GetType() const { return ComponentType::TextBox; }
		void GatherVisit(std::vector<UIObject*>& v);
		bool SetProperty(UIProperty::Enum prop, const char* val);
		bool GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly);
		void SetText(const wchar_t* szText);
		void SetWNScollingOffset(const Vec2& offset);
		unsigned GetTextBoxHeight() const;
		void SetHwndId(HWindowId hwndId);

	protected:
		const static float LEFT_GAP;
		void OnPosChanged(bool anim);
		void OnSizeChanged();
		void CalcTextWidth(); // for mutiline text

	private:
		int mCursorPos;
		bool mPasswd;
		bool mMatchHeight;
		// strong pointer
		ImageBoxPtr mImage;
		std::string mStrBackImage;
		std::string mStrImageDisplay;
		std::wstring mMultiLineText;
	};

}