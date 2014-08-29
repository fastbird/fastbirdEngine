#pragma once

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
	template<bool Wait, bool Sleep> class SpinLock
	{
		std::atomic<long> mLockSem;

	public:
		SpinLock()
			: mLockSem(0)
		{}

		bool Lock()
		{
			do
			{
				// Atomically swap the lock variable with 1 if it's currently equal to 0
				long expected = 0;
				//on certain machines, and for certain algorithms that check this in a loop, 
				// compare_exchange_weak may lead to significantly better performance than compare_exchange_strong
				if (mLockSem.compare_exchange_weak(expected, 1))
				{
					// We successfully acquired the lock
					return true;
				}

				// To reduce inter-CPU bus traffic, when the lock is not acquired, 
				// loop reading without trying to write anything, until 
				// the value changes. This optimization should be effective on 
				// all CPU architectures that have a cache per CPU.
				while (Wait && mLockSem)
				{
					if (Sleep)
					{
						SwitchThread();
					}
				}
			} while (Wait);

			return false;
		}

		void Unlock()
		{
			mLockSem = 0;
		}
	};
}