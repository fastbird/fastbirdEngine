#pragma once
#include <CommonLib/CommonLib.h>
#include <CommonLib/Math/fbMath.h>

namespace fastbird
{

//---------------------------------------------------------------------------
struct FB_CRITICAL_SECTION
{
	FB_CRITICAL_SECTION()
	{
		InitializeCriticalSection(&mCS);
	}

	~FB_CRITICAL_SECTION()
	{
		 DeleteCriticalSection(&mCS);
	}

	LPCRITICAL_SECTION GetWinCS() { return &mCS; }

private:

	CRITICAL_SECTION mCS;
};

//---------------------------------------------------------------------------
inline void EnterCriticalSection(FB_CRITICAL_SECTION& cs)
{
	EnterCriticalSection((LPCRITICAL_SECTION)&cs);
}

//---------------------------------------------------------------------------
inline void LeaveCriticalSection(FB_CRITICAL_SECTION& cs)
{
	LeaveCriticalSection((LPCRITICAL_SECTION)&cs);
}

//---------------------------------------------------------------------------
struct LOCK_CRITICAL_SECTION
{
	LOCK_CRITICAL_SECTION(CRITICAL_SECTION* cs)
	{
		mCS = cs;
		EnterCriticalSection(mCS);
	}

	LOCK_CRITICAL_SECTION(FB_CRITICAL_SECTION* cs)
	{
		mCS = cs->GetWinCS();
		EnterCriticalSection(mCS);
	}
	LOCK_CRITICAL_SECTION(FB_CRITICAL_SECTION& cs)
	{
		mCS = cs.GetWinCS();
		EnterCriticalSection(mCS);
	}
	~LOCK_CRITICAL_SECTION()
	{
		LeaveCriticalSection(mCS);
	}

	LPCRITICAL_SECTION mCS;
};

//---------------------------------------------------------------------------
struct FB_READ_WRITE_CS
{
public:
	FB_READ_WRITE_CS()
		: mNumReaders(0)
	{
		mReaderCleared = CreateEvent(NULL, TRUE, TRUE, NULL);
	}
	~FB_READ_WRITE_CS()
	{
		WaitForSingleObject(mReaderCleared,INFINITE);
		CloseHandle(mReaderCleared);
	}
	void EnterReader()
	{
		EnterCriticalSection(mWriteCS);
		EnterCriticalSection(mReaderCountCS);
		if (++mNumReaders==1)
			ResetEvent(mReaderCleared);
		LeaveCriticalSection(mReaderCountCS);
		LeaveCriticalSection(mWriteCS);
	}
	void LeaveReader()
	{
		EnterCriticalSection(mReaderCountCS);
		if (--mNumReaders==0)
			SetEvent(mReaderCleared);
		LeaveCriticalSection(mReaderCountCS);
	}
	void EnterWriter()
	{
		EnterCriticalSection(mWriteCS);
		WaitForSingleObject(mReaderCleared, INFINITE);
	}
	void LeaveWriter()
	{
		LeaveCriticalSection(mWriteCS);
	}
private:
	FB_CRITICAL_SECTION mWriteCS;
	FB_CRITICAL_SECTION mReaderCountCS;
	int mNumReaders;
	HANDLE mReaderCleared;
};

struct READ_LOCK
{
	READ_LOCK(FB_READ_WRITE_CS& lock)
		: mReadWrite(&lock)
	{
		mReadWrite->EnterReader();
	}
	~READ_LOCK()
	{
		mReadWrite->LeaveReader();
	}

	FB_READ_WRITE_CS* mReadWrite;
};

struct WRITE_LOCK
{
	WRITE_LOCK(FB_READ_WRITE_CS& lock)
		: mReadWrite(&lock)
	{
		mReadWrite->EnterWriter();
	}
	~WRITE_LOCK()
	{
		mReadWrite->LeaveWriter();
	}

	FB_READ_WRITE_CS* mReadWrite;
};

//---------------------------------------------------------------------------
//
// Synchronization base class
//
class SyncBase
{
public:
    virtual ~SyncBase() {}
    virtual void Lock()=0;
    virtual void Unlock()=0;
};

//
// Event
//
class SyncEvent: public SyncBase
{
public:
    virtual void Trigger()=0;
    virtual void Reset()=0;
    virtual bool Wait(DWORD WaitTime = 0xffffffff)=0;
};

SyncEvent* CreateSyncEvent(bool ManualReset=FALSE, char* Name=NULL);

//---------------------------------------------------------------------------
inline void SwitchThread()
{
	Sleep(0);
}

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
    volatile long mLockSem;

public:
    SpinLock()
    : mLockSem(0)
    {}

    bool Lock()
    {
        do
        {
            // Atomically swap the lock variable with 1 if it's currently equal to 0
            if(!InterlockedCompareExchange(&mLockSem, 1, 0))
            {
                // We successfully acquired the lock
                return true;
            }

            // To reduce inter-CPU bus traffic, when the lock is not acquired, 
			// loop reading without trying to write anything, until 
            // the value changes. This optimization should be effective on 
			// all CPU architectures that have a cache per CPU.
            while(Wait && mLockSem)
            {
                if(Sleep)
                {
                    SwitchThread();
                }
            }
        }
        while(Wait);

        return false;
    }

    void Unlock()
    {
        mLockSem = 0;
    }
};

//
// A fixed-size, lockfree queue.
//
//---------------------------------------------------------------------------
template<class tType> class LockFreeQueue
{
private:
    struct LFQNode
    {
		struct ElemKey
        {
            tType* Elem;
            DWORD Key;
        };

        union Data
        {
            ElemKey mElemKey;
			long long mValue;
        };

		Data mData;

        LFQNode()
        {}

        LFQNode(tType* _Elem, DWORD _Key)
        {
			mData.mElemKey.Elem = _Elem;
			mData.mElemKey.Key = _Key;
		}
    };

    int mSize;
    LFQNode* mRealBuffer;
    LFQNode* mBuffer;
    volatile long mReadIndex;
    volatile long mWriteIndex;
       
public:
    LockFreeQueue()
    : mSize(0)
	, mBuffer(NULL)
	, mReadIndex(0)
	, mWriteIndex(0)
    {}

    ~LockFreeQueue()
    {
        if(mRealBuffer)
        {
            free((void*)mRealBuffer);
        }
    }

    void Init(int _Size)
    {
        mSize = GetNextPowerOfTwo(_Size);
        mRealBuffer = (LFQNode*)malloc(sizeof(LFQNode) * mSize + 128);
        assert(mRealBuffer);
        mBuffer = (LFQNode*)(((DWORD)mRealBuffer + 127) & ~127);
        assert(!((DWORD)mBuffer & 127));
            
        for(int i=0; i<mSize; i++)
        {
            mBuffer[i].mData.mElemKey.Elem= NULL;
            mBuffer[i].mData.mElemKey.Key = i;
        }
    }

    bool Enqueue(tType* Elem)
    {
        assert(Elem);
            
        LFQNode CurNode(NULL, 0);
        LFQNode NewNode(Elem, 0);        

        while((mWriteIndex - mReadIndex) < mSize)
        {
            const int CurWriteIndex = mWriteIndex;
            const int WriteBucket = (CurWriteIndex & (mSize - 1));

            CurNode.mData.mElemKey.Key= CurWriteIndex;
            NewNode.mData.mElemKey.Key= CurWriteIndex;

            if(InterlockedCompareExchange64(&mBuffer[WriteBucket].mData.mValue, 
				NewNode.mData.mValue, CurNode.mData.mValue) == CurNode.mData.mValue)
            {
                InterlockedIncrement(&mWriteIndex);
                return true;
            }
        }

        return false;
    }

    tType* Dequeue()
    {
        while(mReadIndex != mWriteIndex)
        {
            const int CurReadIndex = mReadIndex;
            const int ReadBucket = (CurReadIndex & (mSize - 1));
            const LFQNode CurNode((tType*)mBuffer[ReadBucket].mData.mElemKey.Elem, CurReadIndex);

            if(CurNode.mData.mElemKey.Elem)
            {
                const LFQNode NewNode(NULL, CurReadIndex + mSize);

                if(InterlockedCompareExchange64(&mBuffer[ReadBucket].mData.mValue, 
					NewNode.mData.mValue, CurNode.mData.mValue) == CurNode.mData.mValue)
                {
                    InterlockedIncrement(&mReadIndex);
                    return (tType*)CurNode.mData.mElemKey.Elem;
                }
            }
        }

        return NULL;
    }
};

//---------------------------------------------------------------------------
class ThreadSafeCounter
{
protected:
    long mValue;

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
        return InterlockedIncrement(&mValue);
    }

    int operator --()
    {
        return InterlockedDecrement(&mValue);
    }

    int operator +=(int Amount)
    {
        return InterlockedExchangeAdd(&mValue, Amount);
    }

    int operator -=(int Amount)
    {
        return InterlockedExchangeAdd(&mValue, -Amount);
    }
};

//---------------------------------------------------------------------------
struct ThreadHandle
{
    virtual bool IsValid()=0;
};

//---------------------------------------------------------------------------
struct ThreadNameInfo
{
    DWORD dwType;     // must be 0x1000
    LPCSTR szName;    // pointer to name (in user address space)
    DWORD dwThreadID; // thread ID (-1 = caller thread)
    DWORD dwFlags;    // reserved for future use, must be zero
};

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

    // Interface
    virtual bool Init() { return true; }
    virtual bool Run() = 0;
    virtual void Exit() {}
};


}

#include <CommonLib/TaskScheduler.h>
#include <CommonLib/Task.h>