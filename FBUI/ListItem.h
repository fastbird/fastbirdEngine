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
	FB_DECLARE_SMART_PTR(CheckBox);	
	FB_DECLARE_SMART_PTR(ListItem);
	class FB_DLL_UI ListItem : public Container
	{
	protected:
		ListItem();
		~ListItem();

	public:
		static ListItemPtr Create();
		static const size_t INVALID_INDEX;

		// IWinBase
		ComponentType::Enum GetType() const { return ComponentType::ListItem; }
		void GatherVisit(std::vector<UIObject*>& v);

		// Own
		void SetRowIndex(size_t index);
		size_t GetRowIndex() const { return mRowIndex; }

		void SetColIndex(size_t index) { mColIndex = index; }
		size_t GetColIndex() const { return mColIndex; }

		Vec2I GetRowCol() const { return Vec2I(mRowIndex, mColIndex); }

		CheckBoxPtr ListItem::GetCheckBox() const;

		void SetBackColor(const char* backColor);
		void SetNoBackground(bool noBackground);
		const char* GetBackColor() const { return mBackColor.c_str(); }
		bool GetNoBackground() const { return mNoBackground; }

		void OnFocusLost();
		void OnFocusGain();

		void SetMerged(bool m){ mMerged = m; }
		bool GetMerged() const { return mMerged; }		
		void OnMouseHover(void* arg);
		void OnMouseIn(void* arg);
		void OnMouseOut(void* arg);
		void RegisterMouseHoverEvent();

		//bool OnInputFromHandler(IInputInjectorPtr injector);

	protected:
		const static float LEFT_GAP;

		size_t mRowIndex;
		size_t mColIndex;

		// backup values
		std::string mBackColor;
		bool mNoBackground;
		bool mMerged;
	};
}