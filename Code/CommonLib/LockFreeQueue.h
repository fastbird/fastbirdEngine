#pragma once
#include <CommonLib/AsyncObjects.h>
namespace fastbird
{
	//
	// A fixed-size, lockfree queue.
	//
	//---------------------------------------------------------------------------
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

		LockFreeQueue()
		{
			mHead = new Node(type());
			mTail = mHead.load();
		}

		~LockFreeQueue()
		{
			while (mHead.load())
			{
				Node* first = mHead.load();
				mHead = first->mNext.load();
				delete first;
			}
		}

		void Enq(type value)
		{
			Node* node = new Node(value);
			while (true)
			{
				Node* last = mTail.load();
				Node* next = last->mNext.load();
				// Testing shared varialbes one more is helpful to mitigate bus contention.
				// This technique is called test-and-test-and-set, TTAS.
				if (last == mTail)
				{
					if (next == nullptr)
					{
						if (last->mNext.compare_exchange_strong(next, node))
						{
							// from prior senctence to this, it is not atomic.
							// every other method call must be prepared to encounter a half-finiched enq() call,
							// and to finish the job.
							// This is called "helping" technique.
							mTail.compare_exchange_weak(last, node);
							return;
						}
					}
					else
					{
						mTail.compare_exchange_weak(last, next);
					}
				}
			}
		}

		type Deq()
		{
			while (true)
			{
				Node* first = mHead.load();
				Node* last = mTail.load();
				Node* next = first->mNext.load();
				if (first == mHead)
				{
					if (first == last)
					{
						if (next == nullptr)
							return 0; // empty

						mTail.compare_exchange_weak(last, next);
					}
					else
					{
						type value = next->mValue;
						if (mHead.compare_exchange_strong(first, next))
						{
							delete first;
							return value;
						}
					}
				}
			}
		}

		bool TryDeq(type& outData)
		{
			Node* first = mHead.load();
			Node* last = mTail.load();
			Node* next = first->mNext.load();
			if (first == mHead)
			{
				if (first == last)
				{
					if (next == nullptr)
						return false;

					mTail.compare_exchange_weak(last, next);
				}
				else
				{
					type value = next->mValue;
					if (mHead.compare_exchange_strong(first, next))
					{
						delete first;
						outData = value;
						return true;
					}
				}
			}
			return false;
		}

		type SeeCur()
		{
			return mHead.load()->mNext->mValue;
		}
	};
}