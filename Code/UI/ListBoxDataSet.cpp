#include <UI/StdAfx.h>
#include <UI/ListBoxDataSet.h>
#include <UI/ListBoxData.h>

namespace fastbird
{
	ListBoxDataSet::ListBoxDataSet(unsigned num)
		: mLastTime(0)
		, mNumCols(num+1) // including id
	{
		mData.reserve(64);
	}

	ListBoxDataSet::~ListBoxDataSet()
	{
		Clear();
	}

	void ListBoxDataSet::Sort(unsigned colIndex)
	{
		colIndex += 1; // because of the unique key.

		if_assert_fail(colIndex < mNumCols)
			return;
		std::sort(mData.begin(), mData.end(), [&](ListBoxData* left, ListBoxData* right)->bool{
			return wcscmp(left[colIndex].GetText(), right[colIndex].GetText()) < 0;
		}
		);
	}

	unsigned ListBoxDataSet::AddPropertyListData(const std::wstring& uniqueKey, const std::wstring& visualKey, const std::wstring& value)
	{
		// this is for property list.
		assert(mNumCols == 3);
		unsigned index = 0;
		auto it = mMap.find(uniqueKey);
		if (it == mMap.end())
		{
			mData.push_back(FB_ARRNEW(ListBoxData, mNumCols));
			auto& back = mData.back();
			back[0].SetDataType(ListItemDataType::String);
			back[0].SetText(uniqueKey.c_str());

			back[1].SetDataType(ListItemDataType::String);
			back[1].SetText(visualKey.c_str());
			
			back[2].SetDataType(ListItemDataType::TextField);
			back[2].SetText(value.c_str());
			index = mData.size() - 1;
		}
		else
		{
			for (auto& it : mData)
			{
				if (wcscmp(it[0].GetText(), uniqueKey.c_str()) == 0){
					it[1].SetText(visualKey.c_str());
					it[2].SetText(value.c_str());
					break;
				}
				
				++index;
			}
		}
		mMap[uniqueKey] = value;
		return index;
	}

	unsigned ListBoxDataSet::FindRowIndexWithKey(const std::wstring& uniqueKey){
		unsigned row = 0;
		for (auto& cols : mData){
			if (wcscmp(cols[0].GetText(), uniqueKey.c_str()) == 0){
				return row;
			}
			++row;
		}
		return -1;
	}

	unsigned ListBoxDataSet::FindRowIndexWithKey(unsigned uniqueKey){
		unsigned row = 0;
		for (auto& cols : mData){
			if (cols[0].GetKey()== uniqueKey){
				return row;
			}
			++row;
		}
		return -1;
	}
	
	unsigned ListBoxDataSet::InsertData(const std::wstring& uniqueKey){
		auto row = FindRowIndexWithKey(uniqueKey);
		if (row != -1){
			return row;
		}
		mData.push_back(FB_ARRNEW(ListBoxData, mNumCols));
		auto& cols = mData.back();
		cols[0].SetDataType(ListItemDataType::String);
		cols[0].SetText(uniqueKey.c_str());
		return mData.size() - 1;
	}

	unsigned ListBoxDataSet::InsertData(unsigned uniqueKey){
		auto row = FindRowIndexWithKey(uniqueKey);
		if (row != -1){
			return row;
		}
		mData.push_back(FB_ARRNEW(ListBoxData, mNumCols));
		auto& cols = mData.back();
		cols[0].SetKey(uniqueKey);
		return mData.size() - 1;
	}

	unsigned ListBoxDataSet::InsertEmptyData(){
		mData.push_back(FB_ARRNEW(ListBoxData, mNumCols));
		auto& cols = mData.back();
		return mData.size() - 1;
	}

	void ListBoxDataSet::SetData(const std::wstring& uniqueKey, unsigned colIndex, const wchar_t* string, ListItemDataType::Enum type)
	{
		colIndex += 1; // due to the unique key
		if_assert_fail(colIndex < mNumCols)
			return;

		unsigned row = FindRowIndexWithKey(uniqueKey);
		if (row == -1)
		{
			Error(FB_DEFAULT_DEBUG_ARG, 
				FormatString("Cannot find the data set with the key %s", WideToAnsi(uniqueKey.c_str()))
				);
			return;
		}
		auto cols = mData[row];
		cols[colIndex].SetText(string);
		cols[colIndex].SetDataType(type);
	}

	void ListBoxDataSet::SetData(const std::wstring& uniqueKey, unsigned colIndex, bool checked){
		colIndex += 1; // due to the unique key
		if_assert_fail(colIndex < mNumCols)
			return;

		unsigned row = FindRowIndexWithKey(uniqueKey);
		if (row == -1)
		{
			Error(FB_DEFAULT_DEBUG_ARG, 
				FormatString("Cannot find the data set with the key %s", WideToAnsi(uniqueKey.c_str()))
				);
			return;
		}
		auto cols = mData[row];
		cols[colIndex].SetChecked(checked);
	}

	void ListBoxDataSet::SetData(const std::wstring& uniqueKey, unsigned colIndex, ITexture* texture){
		colIndex += 1; // due to the unique key
		if_assert_fail(colIndex < mNumCols)
			return;

		unsigned row = FindRowIndexWithKey(uniqueKey);
		if (row == -1)
		{
			Error(FB_DEFAULT_DEBUG_ARG,
				FormatString("Cannot find the data set with the key %s", WideToAnsi(uniqueKey.c_str()))
				);
			return;
		}
		auto cols = mData[row];
		cols[colIndex].SetTexture(texture);
	}

	// string or texture path
	void ListBoxDataSet::SetData(unsigned uniqueKey, unsigned colIndex, const wchar_t* string, ListItemDataType::Enum type){
		colIndex += 1; // due to the unique key
		if_assert_fail(colIndex < mNumCols)
			return;

		unsigned row = FindRowIndexWithKey(uniqueKey);
		if (row == -1)
		{
			Error(FB_DEFAULT_DEBUG_ARG, 
				FormatString("Cannot find the data set with the key %u", uniqueKey)
				);
			return;
		}
		auto cols = mData[row];
		cols[colIndex].SetText(string);
		cols[colIndex].SetDataType(type);
	}

	// checkbox
	void ListBoxDataSet::SetData(unsigned uniqueKey, unsigned colIndex, bool checked){
		colIndex += 1; // due to the unique key
		if_assert_fail(colIndex < mNumCols)
			return;

		unsigned row = FindRowIndexWithKey(uniqueKey);
		if (row == -1)
		{
			Error(FB_DEFAULT_DEBUG_ARG,
				FormatString("Cannot find the data set with the key %u", uniqueKey)
				);
			return;
		}
		auto cols = mData[row];
		cols[colIndex].SetChecked(checked);
	}

	void ListBoxDataSet::SetData(unsigned uniqueKey, unsigned colIndex, ITexture* texture){
		colIndex += 1; // due to the unique key
		if_assert_fail(colIndex < mNumCols)
			return;

		unsigned row = FindRowIndexWithKey(uniqueKey);
		if (row == -1)
		{
			Error(FB_DEFAULT_DEBUG_ARG,
				FormatString("Cannot find the data set with the key %u", uniqueKey)
				);
			return;
		}
		auto cols = mData[row];
		cols[colIndex].SetTexture(texture);
	}

	void ListBoxDataSet::SetData(const Vec2I& indexRowCol, const wchar_t* string, ListItemDataType::Enum type){
		unsigned rowIndex = indexRowCol.x;
		unsigned colIndex = indexRowCol.y+1;

		if (rowIndex >= mData.size())
			return;
		if (colIndex >= mNumCols)
			return;
		auto cols = mData[rowIndex];
		cols[colIndex].SetText(string);
		cols[colIndex].SetDataType(type);
	}

	void ListBoxDataSet::SetData(const Vec2I& indexRowCol, bool checked){
		unsigned rowIndex = indexRowCol.x;
		unsigned colIndex = indexRowCol.y+1;

		if (rowIndex >= mData.size())
			return;
		if (colIndex >= mNumCols)
			return;
		auto cols = mData[rowIndex];
		cols[colIndex].SetChecked(checked);
	}

	void ListBoxDataSet::SetData(const Vec2I& indexRowCol, ITexture* texture){
		unsigned rowIndex = indexRowCol.x;
		unsigned colIndex = indexRowCol.y + 1;

		if (rowIndex >= mData.size())
			return;
		if (colIndex >= mNumCols)
			return;
		auto cols = mData[rowIndex];
		cols[colIndex].SetTexture(texture);
	}

	void ListBoxDataSet::SetData(const Vec2I& indexRowCol, int number){
		unsigned rowIndex = indexRowCol.x;
		unsigned colIndex = indexRowCol.y + 1;

		if (rowIndex >= mData.size())
			return;
		if (colIndex >= mNumCols)
			return;
		auto cols = mData[rowIndex];
		cols[colIndex].SetNumber(number);
	}

	ListBoxData* ListBoxDataSet::GetData(unsigned index){
		if (index < mData.size())
			return mData[index] + 1; // excluding id column.
		else 
			return 0;
	}

	const std::wstring& ListBoxDataSet::GetValueWithKey(const std::wstring& uniqueKey)
	{
		static std::wstring dummy;
		auto it = mMap.find(uniqueKey);
		if (it != mMap.end()) {
			return it->second;
		}
		else {
			Error(FB_DEFAULT_DEBUG_ARG, FormatString("Data is not found with key %s", uniqueKey.c_str()));
			return dummy;
		}
	}

	unsigned ListBoxDataSet::DelDataWithKey(const std::wstring& uniqueKey)
	{
		auto itMap = mMap.find(uniqueKey);
		if (itMap == mMap.end())
			return -1;

		mMap.erase(itMap);
		unsigned index = 0;
		for (auto it = mData.begin(); it != mData.end(); ++it)
		{
			auto cols = *it;
			if (wcscmp(cols[0].GetText(), uniqueKey.c_str()) == 0)
			{
				FB_SAFE_DEL(cols);
				mData.erase(it);
				return index;
			}
			++index;
		}
		return -1;
	}

	unsigned ListBoxDataSet::DelDataWithKey(unsigned uniqueKey){
		unsigned index = 0;
		for (auto it = mData.begin(); it != mData.end(); ++it)
		{
			auto cols = *it;
			if (cols[0].GetKey()== uniqueKey)
			{
				FB_SAFE_DEL(cols);
				mData.erase(it);
				return index;
			}
			++index;
		}
		return -1;
	}

	unsigned ListBoxDataSet::DelDataWithIndex(unsigned index)
	{
		if (index >= mData.size())
		{
			Error(FB_DEFAULT_DEBUG_ARG, "Out of index");
			return -1;
		}
		auto it = mData.begin() + index;
		auto cols = *it;
		FB_SAFE_DEL(cols);
		mData.erase(it);
		return index;
	}

	void ListBoxDataSet::Clear()
	{
		ClearWithSwap(mMap);
		for (auto data : mData)
		{
			FB_ARRDELETE(data);
		}
		ClearWithSwap(mData);
	}

	unsigned ListBoxDataSet::Size() const
	{
		return mData.size();
	}

	unsigned ListBoxDataSet::FindNext(unsigned colIndex, char c, unsigned curIndex)
	{
		if (mData.empty())
			return -1;
		colIndex += 1;
		assert(colIndex < mNumCols);
		if (colIndex >= mNumCols)
			return -1;

		auto cols = mData[0];
		if (cols[colIndex].GetDataType() != ListItemDataType::String)
			return -1;

		if (gpTimer->GetTime() - mLastTime > 2.f)
		{
			mCharBuffer.clear();
			mLastTime = gpTimer->GetTime();
		}
		
		mCharBuffer.push_back(AnsiToWide(c));
		ToUpperCase(mCharBuffer);
		unsigned num = mData.size();
		for (unsigned i = curIndex; i < num; ++i){
			auto cols = mData[i];
			std::wstring s = cols[colIndex].GetText();
			ToUpperCase(s);
			DeleteValuesInVector(s, L'_');
			if (s.size() >= mCharBuffer.size() && s.compare(0, mCharBuffer.size(), mCharBuffer) == 0)
			{
				return i;
			}
		}
		if (curIndex != 0)
		{
			for (unsigned i = 0; i < curIndex - 1; ++i)
			{
				auto cols = mData[i];
				std::wstring s = cols[colIndex].GetText();
				ToUpperCase(s);
				DeleteValuesInVector(s, L'_');
				if (s.size() >= mCharBuffer.size() && s.compare(0, mCharBuffer.size(), mCharBuffer) == 0)
				{
					return i;
				}
			}
		}
		return -1;
	}

	void ListBoxDataSet::SwapData(unsigned index0, unsigned index1) {
		if (index0 >= mData.size() || index1 >= mData.size())
			return;

		std::swap(mData[index0], mData[index1]);
	}

	const wchar_t* ListBoxDataSet::GetStringKey(unsigned index) const{
		if (index >= mData.size())
			return L"";

		return mData[index][0].GetText();
	}

	unsigned ListBoxDataSet::GetUnsignedKey(unsigned index) const{
		if (index >= mData.size())
			return -1;

		return mData[index][0].GetKey();
	}

	bool ListBoxDataSet::FindNextFocus(unsigned& rowIndex, unsigned& colIndex){
		unsigned initialRowIndex = rowIndex;
		unsigned initialColIndex = colIndex;
		unsigned tryCount = mData.size();
		do{
			++colIndex;
			if (colIndex >= mNumCols-1){
				colIndex = 0;
				++rowIndex;
				if (rowIndex >= mData.size()){
					rowIndex = 0;
				}
			}
			if (rowIndex == initialRowIndex && colIndex == initialColIndex)
				return false;

			if (colIndex == 0 || mData[rowIndex][colIndex+1].CanHaveFocus())
				return true;
		} while (--tryCount);
		return false;
	}

	bool ListBoxDataSet::FindPrevFocus(unsigned& rowIndex, unsigned& colIndex){
		unsigned initialRowIndex = rowIndex;
		unsigned initialColIndex = colIndex;
		unsigned tryCount = mData.size();
		do{
			--colIndex;
			if (colIndex == -1){
				colIndex = mNumCols-1-1;
				--rowIndex;
				if (rowIndex == -1){
					rowIndex = mData.size()-1;
				}
			}
			if (rowIndex == initialRowIndex && colIndex == initialColIndex)
				return false;

			if (colIndex==0 || mData[rowIndex][colIndex+1].CanHaveFocus())
				return true;
		} while (--tryCount);
		return false;
	}
}