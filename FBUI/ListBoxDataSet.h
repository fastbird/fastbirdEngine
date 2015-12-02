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
#include "ListItemDataType.h"

namespace fb
{
	FB_DECLARE_SMART_PTR(Texture);
	class ListBoxData;
	class ListBoxDataSet{
	public:
		typedef std::pair<std::wstring, std::wstring> KeyValueDataType;
	private:
		// vector of mData[]
		std::vector<ListBoxData*> mData;
		unsigned mNumCols;

		// key-value;
		std::map<std::wstring, std::wstring> mMap;

		std::wstring mCharBuffer;
		float mLastTime;


	public:
		ListBoxDataSet(unsigned numCols);
		~ListBoxDataSet();
		void Sort(unsigned colIndex);
		
		// for property list
		unsigned AddPropertyListData(const std::wstring& uniqueKey, const std::wstring& visualKey, const std::wstring& value);
		
		unsigned InsertData(const std::wstring& uniqueKey);
		unsigned InsertData(unsigned uniqueKey);
		unsigned InsertEmptyData();
		bool ModifyKey(unsigned row, unsigned key);

		// string or texture path
		void SetData(const std::wstring& uniqueKey, unsigned colIndex, const wchar_t* string, ListItemDataType::Enum type);		
		// checkbox
		void SetData(const std::wstring& uniqueKey, unsigned colIndex, bool checked);
		void SetData(const std::wstring& uniqueKey, unsigned colIndex,  TexturePtr texture);

		// string or texture path
		void SetData(unsigned uniqueKey, unsigned colIndex, const wchar_t* string, ListItemDataType::Enum type);
		// checkbox
		void SetData(unsigned uniqueKey, unsigned colIndex, bool checked);
		void SetData(unsigned uniqueKey, unsigned colIndex, TexturePtr texture);

		void SetData(const Vec2I& indexRowCol, const wchar_t* string, ListItemDataType::Enum type);
		void SetData(const Vec2I& indexRowCol, bool checked);
		void SetData(const Vec2I& indexRowCol, TexturePtr texture);
		void SetData(const Vec2I& indexRowCol, int number); // numeric Up Down
		void SetData(const Vec2I& indexRowCol, float number); // horizontal gauge

		
		unsigned FindRowIndexWithKey(const std::wstring& uniqueKey);
		unsigned FindRowIndexWithKey(unsigned uniqueKey);
		
		ListBoxData* GetData(unsigned i);		
		const std::wstring& GetValueWithKey(const std::wstring& uniqueKey);

		const wchar_t* GetStringKey(unsigned rowIndex) const;
		unsigned GetUnsignedKey(unsigned rowIndex) const;

		unsigned DelDataWithKey(const std::wstring& uniqueKey);
		unsigned DelDataWithKey(unsigned uniqueKey);
		unsigned DelDataWithIndex(unsigned index);

		void Clear();
		unsigned Size() const;
		unsigned FindNext(unsigned colIndex, char c, unsigned curIndex);

		void SwapData(unsigned index0, unsigned index1);

		bool FindNextFocus(unsigned& rowIndex, unsigned& colIndex);
		bool FindPrevFocus(unsigned& rowIndex, unsigned& colIndex);
	};
}