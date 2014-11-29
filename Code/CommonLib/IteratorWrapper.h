#pragma once

namespace fastbird
{
	template<typename T>
	class IteratorWrapper
	{
	public:
		typedef typename T::value_type ValueType;
		typedef typename T::iterator Iterator;
		IteratorWrapper(T& data)
		{
			mCurrent = mBegin = data.begin();
			mEnd = data.end();
		}

		ValueType GetType()
		{
			return *mBegin;
		}

		bool HasMoreElement() const
		{
			return  (mCurrent != mEnd);
		}
		ValueType GetNext()
		{
			return *mCurrent++;
		}

	private:
		Iterator mBegin;
		Iterator mEnd;
		Iterator mCurrent;
	};
}