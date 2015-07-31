#pragma once

namespace fastbird{
	class RecursiveLock{
		std::recursive_mutex mMutex;
	public:
		void Lock();
		void Unlock();
	};
}