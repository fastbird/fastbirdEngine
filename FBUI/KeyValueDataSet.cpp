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
#include "KeyValueDataSet.h"

namespace fb
{
	KeyValueDataSet::KeyValueDataSet()
		: mLastTime(0)
	{
	}

	void KeyValueDataSet::Sort()
	{
		std::sort(mValues.begin(), mValues.end(), [](const DataType& left, const DataType&right)->bool{
			return left.first.compare(right.first) < 0;
		}
		);
	}

	unsigned KeyValueDataSet::AddData(const KeyType& key, const ValueType& value)
	{
		unsigned index = 0;
		auto it = mMap.find(key);
		if (it == mMap.end())
		{
			mValues.push_back(std::make_pair(key, value));
			index = mValues.size() - 1;
		}
		else
		{
			for (auto& it : mValues)
			{
				if (it.first == key)
				{
					it.second = value;
					break;
				}
				++index;
			}
		}
		mMap[key] = value;
		return index;
	}

	KeyValueDataSet::DataType* KeyValueDataSet::GetData(unsigned i){
		if (i < mValues.size())
			return &mValues[i];
		else 
			return 0;
	}

	const KeyValueDataSet::ValueType& KeyValueDataSet::GetData(const KeyType& key)
	{
		static ValueType dummy;
		auto it = mMap.find(key);
		if (it != mMap.end()) {
			return it->second;
		}
		else {
			Error(FB_ERROR_LOG_ARG, FormatString("Data is not found with key %s", key.c_str()));
			return dummy;
		}
	}

	unsigned KeyValueDataSet::DelData(const KeyType& key)
	{
		auto itMap = mMap.find(key);
		if (itMap == mMap.end())
			return -1;

		mMap.erase(itMap);
		unsigned index = 0;
		for (auto it = mValues.begin(); it != mValues.end(); ++it)
		{
			if (it->first == key)
			{
				mValues.erase(it);
				return index;
			}
			++index;
		}
		return -1;
	}

	void KeyValueDataSet::Clear()
	{
		ClearWithSwap(mMap);
		ClearWithSwap(mValues);
	}

	unsigned KeyValueDataSet::Size() const
	{
		return mValues.size();
	}

	unsigned KeyValueDataSet::FindNext(char c, unsigned startIndex)
	{
		if (gpTimer->GetTime() - mLastTime > 1.f)
		{
			mCharBuffer.clear();
			mLastTime = gpTimer->GetTime();
		}
		
		mCharBuffer.push_back(AnsiToWide(c));
		ToUpperCase(mCharBuffer);
		unsigned num = mValues.size();
		for (unsigned i = startIndex; i < num; ++i){
			std::wstring s = mValues[i].first;
			DeleteValuesInVector(s, L'_');
			if (s.size() >= mCharBuffer.size() && s.compare(0, mCharBuffer.size(), mCharBuffer) == 0)
			{
				return i;
			}
		}
		for (unsigned i = 0; i < startIndex; ++i)
		{
			std::wstring s = mValues[i].first;
			DeleteValuesInVector(s, L'_');
			if (s.size() >= mCharBuffer.size() && s.compare(0, mCharBuffer.size(), mCharBuffer) == 0)
			{
				return i;
			}
		}
		return -1;
	}


}