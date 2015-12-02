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
#include <vector>
namespace fb{
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

			inline T* operator->() const { return &(*mIterator); }
			inline T& operator*() const { return *mIterator; }
			bool operator==(const IteratorWrapper& other) const
			{
				assert(mBuffer == other.mBuffer);
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
			assert(size > 4);
			mVector.assign(size, T());
			mBegin = mVector.begin();
			mEnd = mBegin;
			mInit = true;
		}

		unsigned size() const
		{
			return mVector.size();
		}

		bool empty() const
		{
			return mVector.empty();
		}

		void DoubleSize()
		{
			VECTOR temp;
			size_t beginIdx = std::distance(mVector.begin(), mBegin);
			size_t endIdx = std::distance(mVector.begin(), mEnd);
			mVector.resize(mVector.size() * 2);
			mBegin = mVector.begin() + beginIdx;
			mEnd = mVector.begin() + endIdx;
		}

		size_t push_back(const value_type& data)
		{
			if (!mInit)
			{
				Init(8);
			}
			(*mEnd) = data;
			size_t idx = std::distance(mVector.begin(), mEnd);
			++mEnd;

			if (mEnd == mVector.end())
			{
				mEnd = mVector.begin();
				if (mBegin == mEnd)
				{
					//Log("Particle circular buffer is too small. some will be deleted.");
					++mBegin;
					if (mBegin == mVector.end())
					{
						assert(0);
						mBegin = mVector.begin();
					}
				}
			}

			return idx;
		}
		IteratorWrapper begin()
		{
			while (1)
			{
				if (!mBegin->IsAvailable() || mBegin == mEnd)
				{
					return IteratorWrapper(mBegin, &mVector);
				}

				++mBegin;
				if (mBegin == mVector.end())
					mBegin = mVector.begin();
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
				return *(mVector.end() - 1);
			}
			else
			{
				return *(mEnd - 1);
			}
		}

		value_type& GetAt(size_t idx)
		{
			return mVector.at(idx);
		}

		VECTOR& GetVector() { return mVector; }
		iterator& GetRawBeginIter() { return mBegin; }
		iterator& GetRawEndIter() { return mEnd; }

	private:
		VECTOR mVector;

		iterator mBegin;
		iterator mEnd;

		bool mInit;
	};
}