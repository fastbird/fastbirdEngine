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

#include "StdAfx.h"
#include "ListBoxData.h"

namespace fb{
	ListBoxData::ListBoxData(ListItemDataType::Enum type, const wchar_t* text, bool checked)
		: mDataType(type)
		, mChecked(checked)
		, mKey(-1)
	{
		mNumber.i = 0;
		if (text)
			mText = text;
	}

	ListBoxData::ListBoxData()
		: mDataType(ListItemDataType::Unknown)
		, mChecked(false)
		, mKey(-1)
	{

	}

	ListBoxData::~ListBoxData()
	{

	}

	bool ListBoxData::IsTextData() const
	{
		return mDataType == ListItemDataType::String ||
			mDataType == ListItemDataType::TextField ||
			mDataType == ListItemDataType::TexturePath ||
			mDataType == ListItemDataType::TextureRegion;
	}

	bool ListBoxData::CanHaveFocus() const{
		return mDataType == ListItemDataType::TextField || mDataType == ListItemDataType::CheckBox;
	}

	void ListBoxData::SetTexture(TexturePtr texture)
	{
		mTexture = texture;
		mDataType = ListItemDataType::TexturePath;
	}

	void ListBoxData::SetInt(int number){
		mNumber.i = number;
		mDataType = ListItemDataType::NumericUpDown;
	}

	void ListBoxData::SetFloat(float f){
		mNumber.f = f;
		mDataType = ListItemDataType::HorizontalGauge;
	}
}