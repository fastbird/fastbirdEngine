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
			mNumElem = data.size();
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

		ValueType Advance(unsigned dist)
		{
			if (dist >= mNumElem)
			{
				mCurrent = mEnd;
			}
			else
			{
				mCurrent += dist;
			}			
		}

	private:
		Iterator mBegin;
		Iterator mEnd;
		Iterator mCurrent;
		unsigned mNumElem;
	};
}