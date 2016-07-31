#pragma once
#include <queue>
#include "SpinLock.h"
namespace fb {
	template<class type>
	class LockQueue {
		std::queue<type> mQueue;
		mutable SpinLockWaitSleep mSpinLock;

	public:
		void Clear() {
			EnterSpinLock<SpinLockWaitSleep> lock(mSpinLock);
			while (!mQueue.empty())
				mQueue.pop();
		}

		void Enq(type& value) {
			EnterSpinLock<SpinLockWaitSleep> lock(mSpinLock);
			mQueue.push(value);
		}

		void Enq(type&& value) {
			EnterSpinLock<SpinLockWaitSleep> lock(mSpinLock);
			mQueue.push(std::move(value));
		}

		type Deq() {
			type val;
			{
				EnterSpinLock<SpinLockWaitSleep> lock(mSpinLock);
				if (mQueue.empty())
					return val;
				val = mQueue.front();
				mQueue.pop();
			}
			return val;
		}

		type SeeCur() {
			type val;
			{
				EnterSpinLock<SpinLockWaitSleep> lock(mSpinLock);
				if (mQueue.empty())
					return val;
				val = mQueue.front();
			}
			return val;
		}

		int GetCount() const {
			EnterSpinLock<SpinLockWaitSleep> lock(mSpinLock);
			return mQueue.size();
		}
		
	};
}