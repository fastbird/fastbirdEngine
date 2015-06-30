#pragma once
#include <UI/ListItemDataType.h>

namespace fastbird
{
	class ListBoxData;
	class ITexture;
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
		void SetData(const std::wstring& uniqueKey, unsigned colIndex,  ITexture* texture);

		// string or texture path
		void SetData(unsigned uniqueKey, unsigned colIndex, const wchar_t* string, ListItemDataType::Enum type);
		// checkbox
		void SetData(unsigned uniqueKey, unsigned colIndex, bool checked);
		void SetData(unsigned uniqueKey, unsigned colIndex, ITexture* texture);

		void SetData(const Vec2I& indexRowCol, const wchar_t* string, ListItemDataType::Enum type);
		void SetData(const Vec2I& indexRowCol, bool checked);
		void SetData(const Vec2I& indexRowCol, ITexture* texture);
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