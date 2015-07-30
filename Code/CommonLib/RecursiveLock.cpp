#include <CommonLib/StdAfx.h>
#include <CommonLib/RecursiveLock.h>
namespace fastbird{
	void RecursiveLock::Lock(){
		mMutex.lock();
	}
	void RecursiveLock::Unlock(){
		mMutex.unlock();
	}
}