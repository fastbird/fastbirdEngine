#pragma once
#include <CommonLib/AsyncObjects.h>
#include <CommonLib/SpinLock.h>

namespace fastbird
{
	//---------------------------------------------------------------------------
	// SpinLocks are efficient if threads are only likely to be blocked for 
	// a short period of time, as they avoid overhead from operating
	// system process re-scheduling or context switching. However, spinlocks become 
	// wasteful if held for longer durations, both preventing other threads from 
	// running and requiring re-scheduling. The longer a lock is held by a thread, 
	// the greater the risk that it will be interrupted by the O/S scheduler while holding 
	// the lock. If this happens, other threads will be left "spinning" (repeatedly trying 
	// to acquire the lock), while the thread holding the lock is not making progress 
	// towards releasing it. The result is a semi-deadlock until the thread holding 
	// the lock can finish and release it. This is especially true on a single-processor 
	// system, where each waiting thread of the same priority is likely to waste its 
	// quantum (allocated time where a thread can run) spinning until the thread that 
	// holds the lock is finally finished.
	template<bool Wait, bool Sleep>
	class RecursiveSpinLock
	{
	protected:
		SpinLock<Wait, Sleep> mGuard;
		long mLockLevel;
		std::thread::id mThreadId;


	public:
		RecursiveSpinLock()
			: mLockLevel(0)
		{}

		bool Lock()
		{
			do
			{
				if (!mGuard.Lock())
					return false;

				// Atomically swap the lock variable with 1 if it's currently equal to 0
				long expected = 0;
				//on certain machines, and for certain algorithms that check this in a loop, 
				// compare_exchange_weak may lead to significantly better performance than compare_exchange_strong				
				if (mLockLevel==0)
				{
					mLockLevel = 1;
					mThreadId = std::this_thread::get_id();
					// We successfully acquired the lock
					mGuard.Unlock();
					return true;
				}
				if (mThreadId == std::this_thread::get_id()){
					++mLockLevel;
					mGuard.Unlock();
					return true;
				}
				mGuard.Unlock();

				// To reduce inter-CPU bus traffic, when the lock is not acquired, 
				// loop reading without trying to write anything, until 
				// the value changes. This optimization should be effective on 
				// all CPU architectures that have a cache per CPU.
				while (Wait && mLockLevel)
				{
					if (Sleep)
					{
						std::this_thread::yield(); // SwitchThread
					}
				}
			} while (Wait);

			return false;
		}

		void Unlock()
		{
			--mLockLevel;
			assert(mLockLevel >= 0);
			//long expected = mLockLevel.load();
			//long desired = expected - 1;
			//while (!mLockLevel.compare_exchange_weak(expected, desired))
			//	/*nothing to do*/;

			//if (std::this_thread::get_id() == mDebugThread){
			//	--mDebugLockCounter;
			//}
		}
	};
}