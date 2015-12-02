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
#include "UIPropTypes.h"
#include "UIProperty.h"

namespace fb
{
	// NOT USING
	template <class T>
	class UIProp
	{
	public:
		UIProp(UIProperty::Enum propname, UIPropTypes::Enum type, T defaultValue)
			: mPropName(propname)
			, mType(type)
			, mValue(defaultValue)
			, mDefaultValue(defaultValue)
		{
		}
		const T& GetValue() const { return mValue; }
		const char* GetStrValue() const { return mStrValue.c_str(); }
		
		void SetValue(const T& value){
			mVlaue = value;
		}
		void SetStrValue(const char* strValue)	{
			if (!strValue)
				return;

			mStrValue = strValue;
			switch (mType)
			{
			case UIPropTypes::Bool:
			{
				mValue = StringConverter::ParseBool(strValue);
			}
			case UIPropTypes::Int:
			{
				mValue = StringConverter::ParseInt(strValue);
			}
			case UIPropTypes::Color:
			{
				mValue = StringMathConverter::ParseColor(strValue);
			}
			case UIPropTypes::Vec2I:
			{
				mValue = StringMathConverter::ParseVec2I(strValue);
			}
			case UIPropTypes::Float:
			{
				mValue = StringConverter::ParseReal(strValue);
			}
			case UIPropTypes::String:
			{
				mValue = mStrValue;
			}
			default:
				assert(0);
			}
		}

		bool IsDefault() const {
			return mValue == mDefaultValue;
		}



	private:
		UIProperty::Enum mPropName;
		UIPropTypes::Enum mType;
		T mValue;
		T mDefaultValue;

		std::string mStrValue;

	};
}