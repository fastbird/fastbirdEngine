#include <CommonLib/StdAfx.h>
#include <CommonLib/threads.h>

namespace fastbird
{

__declspec(thread) ThreadInfo* GThreadDesc = NULL;

struct ThreadHandleWin: ThreadHandle
{
    HANDLE ThreadHandle;

    ThreadHandleWin(Thread* Thread, int StackSize, char* ThreadName)
    {
        DWORD ThreadID;
        ThreadHandle = ::CreateThread(NULL, StackSize, ThreadProc, Thread, 0, &ThreadID);

        if(ThreadHandle)
        {
            SetThreadName(ThreadID, ThreadName);
        }
    }

    ~ThreadHandleWin()
    {
        if(ThreadHandle)
        {
            CloseHandle(ThreadHandle);
        }
    }

    bool IsValid()
    {
        return (ThreadHandle != NULL);
    }

    static DWORD __stdcall ThreadProc(LPVOID ThreadPtr)
    {
	    CHECK(ThreadPtr);
        Thread* ThreadInstance = (Thread*)ThreadPtr;

        ThreadInstance->StartRun();
        ThreadInstance->RegisterThread();

        DWORD ReturnCode = 0;

        if(ThreadInstance->Init())
        {
            while(!ThreadInstance->IsForceExit() && ThreadInstance->Run())
            {
            }

            ThreadInstance->Exit();
        }

        ThreadInstance->EndRun();
        return ReturnCode;
    }
};

static ThreadHandle* CreateThreadHandle(Thread* ThreadInstance, int StackSize, 
	char* ThreadName)
{
    return new ThreadHandleWin(ThreadInstance, StackSize, ThreadName);
}

Thread::~Thread()
{
    CHECK(!IsRunning());

    delete mThreadDesc;        
    delete mThreadHandle;
}

void Thread::CreateThread(int StackSize, char* ThreadName)
{
    mThreadDesc = new ThreadInfo;
    strcpy(mThreadDesc->ThreadName, ThreadName);

    mThreadHandle = CreateThreadHandle(this, StackSize, mThreadDesc->ThreadName);

    CHECK(mThreadHandle);
    CHECK(mThreadHandle->IsValid());
}

void Thread::RegisterThread()
{
    CHECK(!GThreadDesc);
    CHECK(mThreadDesc);

    mThreadDesc->ThreadID = GetCurrentThreadId();
    GThreadDesc = mThreadDesc;
}

void Thread::RegisterThread(char* ThreadName)
{
    CHECK(!GThreadDesc);

    GThreadDesc = new ThreadInfo;
    strcpy(GThreadDesc->ThreadName, ThreadName);
    GThreadDesc->ThreadID = GetCurrentThreadId();
}


// Synchronization
class SyncEventWin: public SyncEvent
{
protected:
    HANDLE EventHandle;

public:
    SyncEventWin(bool ManualReset, char* Name)
    {
        EventHandle = CreateEventA(NULL, ManualReset, 0, Name);
        CHECK(EventHandle);
    }

    ~SyncEventWin()
    {
        CloseHandle(EventHandle);
    }

    void Trigger()
    {
        SetEvent(EventHandle);
    }

    void Reset()
    {
        ResetEvent(EventHandle);
    }

    bool Wait(DWORD WaitTime)
    {
        return WaitForSingleObject(EventHandle, WaitTime) == WAIT_OBJECT_0;
    }

    void Lock()
    {
        WaitForSingleObject(EventHandle, INFINITE);
    }

	void Unlock()
	{
	}
};

SyncEvent* CreateSyncEvent(bool ManualReset, char* Name)
{
    return new SyncEventWin(ManualReset, Name);
}

}