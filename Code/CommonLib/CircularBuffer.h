#pragma once
namespace fastbird
{

struct ICircularData
{
public:
	virtual bool IsAvailable() const = 0; 
};

// T should inherited from ICircularData
template<class T>
class CircularBuffer
{
public:
	typedef T value_type;
	typedef std::vector<value_type> VECTOR;
	typedef typename VECTOR::iterator iterator;

	struct IteratorWrapper
	{
		IteratorWrapper(iterator it, VECTOR* buffer)
			:mIterator(it)
			, mBuffer(buffer)
		{
		}

		IteratorWrapper operator=(const IteratorWrapper& other)
		{
			assert(mBuffer == other.mBuffer)
			mIterator = other.mIterator;
		}
		
		inline T* operator->() const { return &(*mIterator);}
		inline T& operator*() const { return *mIterator;}
		bool operator==(const IteratorWrapper& other) const
		{
			return mIterator == other.mIterator;
		}

		bool operator!=(const IteratorWrapper& other) const
		{
			return !operator==(other);
		}

		IteratorWrapper& operator++() // prefix ++
		{
			++mIterator;
			if (mIterator == mBuffer->end())
			{
				mIterator = mBuffer->begin();
			}

			return *this;
		}



	private:
		iterator mIterator;
		VECTOR* mBuffer;
	};
	CircularBuffer()
		: mInit(false)
	{
		mBegin = mVector.begin();
		mEnd = mVector.end();
	}

	void Init(size_t size)
	{
		assert(size>4);
		mVector.assign(size, T());
		mBegin = mVector.begin();
		mEnd = mBegin;
		mInit = true;
	}

	void push_back(const value_type& data)
	{
		if (!mInit)
		{
			Init(8);
		}
		(*mEnd) = data;
		mEnd++;
		
		if (mEnd==mVector.end())
		{
			mEnd = mVector.begin();
			assert(mBegin != mEnd); // buffer is too small.
			if (mBegin==mEnd)
			{
				mBegin++;
				if (mBegin == mVector.end())
				{
					mBegin = mVector.begin();
				}
			}
		}
	}
	IteratorWrapper begin()
	{
		while(1)
		{
			if (mBegin == mEnd || !mBegin->IsAvailable())
				return IteratorWrapper(mBegin, &mVector);
			mBegin++;
			if (mBegin==mVector.end())
				mBegin=mVector.begin();
		}
	}

	IteratorWrapper end()
	{
		return IteratorWrapper(mEnd, &mVector);
	}

	value_type& back()
	{
		if (mEnd == mVector.begin())
		{
			return *(mVector.end()-1);
		}
		else
		{
			return *(mEnd-1);
		}
	}

private:
	size_t mNext;
	
	VECTOR mVector;

	iterator mBegin;
	iterator mEnd;

	bool mInit;
};

}