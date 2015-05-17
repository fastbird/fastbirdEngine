#pragma once

namespace fastbird
{
	class KeyValueDataSet{
	public:

		typedef std::wstring KeyType;
		typedef std::wstring ValueType;
		typedef std::pair<KeyType, ValueType> DataType;


	private:

		std::vector< DataType > mValues;
		std::map<KeyType, ValueType> mMap;

		std::wstring mCharBuffer;
		float mLastTime;


	public:
		KeyValueDataSet();
		void Sort();
		unsigned AddData(const KeyType& key, const ValueType& value);
		DataType* GetData(unsigned i);
		const ValueType& GetData(const KeyType& key);
		unsigned DelData(const KeyType& key);
		void Clear();
		unsigned Size() const;
		unsigned FindNext(char c, unsigned startIndex);

	};
}