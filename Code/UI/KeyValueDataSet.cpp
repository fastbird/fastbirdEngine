#include <UI/StdAfx.h>
#include <UI/KeyValueDataSet.h>

namespace fastbird
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
			Error(FB_DEFAULT_DEBUG_ARG, FormatString("Data is not found with key %s", key.c_str()));
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