#pragma once
#include <CommonLib/CommonLib.h>
#include <CommonLib/Math/fbMath.h>

namespace fastbird
{

//---------------------------------------------------------------------------
inline void SwitchThread()
{
	std::this_thread::yield();
}



//---------------------------------------------------------------------------
class ThreadSafeCounter
{
protected:
    std::atomic<long> mValue;

    ThreadSafeCounter(const ThreadSafeCounter&) {}
    void operator =(const ThreadSafeCounter&) {}

public:
    ThreadSafeCounter()
    : mValue(0)
    {}

    ThreadSafeCounter(int _Value)
    : mValue(_Value)
    {}

    int operator *()
    {
        return mValue;
    }

    int operator ++()
    {
		return ++mValue;
    }

    int operator --()
    {
		return --mValue;
    }

    int operator +=(int Amount)
    {
		mValue += Amount;
		return mValue;
    }

    int operator -=(int Amount)
    {
		mValue -= Amount;
		return mValue;
    }
};

//---------------------------------------------------------------------------
struct ThreadNameInfo
{
    DWORD dwType;     // must be 0x1000
    LPCSTR szName;    // pointer to name (in user address space)
    DWORD dwThreadID; // thread ID (-1 = caller thread)
    DWORD dwFlags;    // reserved for future use, must be zero
};

//---------------------------------------------------------------------------
inline void SetThreadName(DWORD ThreadID, const char* ThreadName)
{
    ThreadNameInfo Info;

    Info.dwType = 0x1000;
    Info.szName = ThreadName;
    Info.dwThreadID = ThreadID;
    Info.dwFlags = 0;

    __try
    {
        RaiseException(0x406D1388, 0, sizeof(Info) / sizeof(DWORD), (DWORD*)&Info);
    }
    __except(EXCEPTION_CONTINUE_EXECUTION)
    {
    }
}

//---------------------------------------------------------------------------
struct ThreadInfo
{
    char ThreadName[128];
    DWORD ThreadID;

    ThreadInfo()
    : ThreadID(0xffffffff)
    {}
};

extern __declspec(thread) ThreadInfo* GThreadDesc;

class Thread;
struct ThreadHandle
{
public:
	ThreadHandle(Thread* Thread, int StackSize, char* ThreadName);
	~ThreadHandle();
	bool IsValid();
	HANDLE GetNativeHandle();
	void Join();
private:
	std::thread mThread;
};
//---------------------------------------------------------------------------
class Thread
{
protected:
    ThreadHandle* mThreadHandle;
    ThreadSafeCounter mRunning;
    ThreadSafeCounter mForcedExit;

public:
    ThreadInfo* mThreadDesc;

    Thread()
    : mThreadDesc(NULL)
    {}

    virtual ~Thread();

    void CreateThread(int StackSize, char* ThreadName);
    void RegisterThread();
    static void RegisterThread(char* ThreadName);

    void StartRun()
    {
        CHECK(!*mRunning);
        ++mRunning;
    }

    void EndRun()
    {
        CHECK(*mRunning);
        --mRunning;
    }

    bool IsForceExit()
    {
        return !!*mForcedExit;
    }

    void ForceExit(bool Wait)
    {
        ++mForcedExit;

        while(Wait && IsRunning())
        {
            SwitchThread();
        }
    }

    bool IsRunning()
    {
        return (*mRunning != 0);
    }

	void Join()
	{
		assert(mThreadHandle);
		mThreadHandle->Join();
	}

    // Interface
    virtual bool Init() { return true; }
    virtual bool Run() = 0;
    virtual void Exit() {}
};


}

#include <CommonLib/TaskScheduler.h>
#include <CommonLib/Task.h>
#include <CommonLib/AsyncObjects.h>
#include <CommonLib/LockFreeQueue.h>
#include <CommonLib/SpinLock.h>
#include <CommonLib/RecursiveLock.h>
#include <CommonLib/RecursiveSpinLock.h>