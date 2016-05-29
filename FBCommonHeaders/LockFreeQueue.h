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
#include <atomic>
namespace fb
{
	//---------------------------------------------------------------------------
	// type must have
	//   1. a copy constructor 
	//   2. a trivial assignment operator 
	//   3. a trivial destructor 
	// std::shared_ptr cannot be the 'type'
	template<class type>
	class LockFreeQueue
	{
	public:
		class Node
		{
		public:
			type mValue;
			std::atomic<Node*> mNext;

			Node(type value)
				: mValue(value)
			{
				mNext = nullptr;
			}
		};

		std::atomic<Node*> mHead;
		std::atomic<Node*> mTail;
		std::atomic<int> mCount = 0;

		LockFreeQueue()
		{
			mHead = new Node(type());
			mTail = mHead.load();			
		}

		~LockFreeQueue()
		{
			Clear();
			delete mHead.load();
		}

		void Clear() {
			while (mHead.load())
			{
				auto first = mHead.load();
				auto next = first->mNext.load();
				if (mHead.compare_exchange_strong(first, next))
					delete first;
			}
			mHead = new Node(type());
			mTail = mHead.load();
		}

		void Enq(type value)
		{
			Node* node = new Node(value);			
			while (true)
			{
				Node* last = mTail.load(std::memory_order_relaxed);
				Node* next = last->mNext.load(std::memory_order_relaxed);
				// Testing shared varialbes one more is helpful to mitigate bus contention.
				// This technique is called test-and-test-and-set, TTAS.
				if (last == mTail)
				{
					if (next == nullptr)
					{
						if (last->mNext.compare_exchange_strong(next, node, std::memory_order_relaxed))
						{
							// from prior senctence to this, it is not atomic.
							// every other method call must be prepared to encounter a half-finiched enq() call,
							// and to finish the job.
							// This is called "helping" technique.
							mTail.compare_exchange_weak(last, node, std::memory_order_relaxed);
							++mCount;
							return;
						}
					}
					else
					{
						mTail.compare_exchange_weak(last, next, std::memory_order_relaxed);
					}
				}
			}
		}

		type Deq()
		{
			while (true)
			{
				Node* first = mHead.load(std::memory_order_relaxed);
				Node* last = mTail.load(std::memory_order_relaxed);
				Node* next = first->mNext.load(std::memory_order_relaxed);
				if (first == mHead)
				{
					if (first == last)
					{
						if (next == nullptr)
							return nullptr; // empty

						mTail.compare_exchange_weak(last, next, std::memory_order_relaxed);
					}
					else
					{						
						type value = next->mValue;
						if (mHead.compare_exchange_strong(first, next, std::memory_order_relaxed))
						{							
							--mCount;
							delete first;							
							return value;
						}
					}
				}
			}
		}

		type SeeCur()
		{
			return mHead.load()->mNext->mValue;
		}

		int GetCount() const {
			return mCount;
		}
	};
}