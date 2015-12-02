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
#include "ListBox.h"

namespace fb{
	FB_DECLARE_SMART_PTR(PropertyList);
	class FB_DLL_UI PropertyList : public ListBox
	{
		unsigned mFocusRow;

	protected:
		PropertyList();
		~PropertyList();

	public:
		static PropertyListPtr Create();
		
		void OnCreated();
		ComponentType::Enum GetType() const { return ComponentType::PropertyList; }
		
		unsigned InsertItem(const wchar_t* key, const wchar_t* value);
		unsigned ModifyItem(const wchar_t* key, const wchar_t* value);

		const wchar_t* GetValue(const wchar_t* key);

		bool GetCurKeyValue(std::string& key, std::string& value);		

		void MoveFocusToEdit(unsigned index);
		void MoveLine(bool applyInput, bool next);
		void RemoveHighlight(unsigned index);
		void MoveFocusToKeyItem();

		//void VisualizeData(unsigned index);

		void SetFocusRow(unsigned row){ mFocusRow = row; }
		unsigned GetFocusRow() const { return mFocusRow; }
		
	};
}