#pragma once
#include <string>
#include <sstream>

#include <CommonLib/threads.h>
namespace fastbird
{
	class IDGenerator
	{
	public:
		IDGenerator(const IDGenerator& rhs)
			: mPrefix(rhs.mPrefix)
			, mNext(rhs.mNext)
		{
			InitializeCriticalSection(&mCriticalSection);
		}

		IDGenerator(const char* prefix)
			: mPrefix(prefix)
			, mNext(1)
		{
			InitializeCriticalSection(&mCriticalSection);
		}
		~IDGenerator()
		{
			DeleteCriticalSection(&mCriticalSection);
		}

		std::string Generate()
		{
			LOCK_CRITICAL_SECTION lock(&mCriticalSection);
			std::ostringstream s;
			s << mPrefix << mNext++;
			return s.str();
		}
		void Reset()
		{
			LOCK_CRITICAL_SECTION lock(&mCriticalSection);
			mNext = 1ULL;
		}
		void SetNext(unsigned long long int val)
		{
			LOCK_CRITICAL_SECTION lock(&mCriticalSection);
			mNext = val;
		}
		unsigned long long int GetNext()
		{
			// 64-bit may not be atomic read
			LOCK_CRITICAL_SECTION lock(&mCriticalSection);
			return mNext;
		}

	protected:
		std::string mPrefix;
		unsigned long long int mNext;
		CRITICAL_SECTION mCriticalSection;
	};
};