#include <CommonLib/StdAfx.h>
#include <CommonLib/threads.h>
#include <CommonLib/CommonLib.h>

namespace fastbird
{

	__declspec(thread) ThreadInfo* GThreadDesc = NULL;

	//---------------------------------------------------------------------------
	static DWORD __stdcall ThreadProc(LPVOID ThreadPtr)
	{
		CHECK(ThreadPtr);
		Thread* ThreadInstance = (Thread*)ThreadPtr;

		ThreadInstance->StartRun();
		ThreadInstance->RegisterThread();

		DWORD ReturnCode = 0;

		if (ThreadInstance->Init())
		{
			while (!ThreadInstance->IsForceExit() && ThreadInstance->Run())
			{
			}

			ThreadInstance->Exit();
		}

		ThreadInstance->EndRun();
		return ReturnCode;
	}

	//---------------------------------------------------------------------------
	ThreadHandle::ThreadHandle(Thread* Thread, int StackSize, char* ThreadName)
	{
		mThread = std::thread(ThreadProc, Thread);
		DWORD ThreadID = GetThreadId(mThread.native_handle());
		SetThreadName(ThreadID, ThreadName);
	}
	ThreadHandle::~ThreadHandle()
	{
		Join();
	}

	HANDLE ThreadHandle::GetNativeHandle()
	{
		return mThread.native_handle();
	}

	bool ThreadHandle::IsValid()
	{
		return mThread.joinable();
	}
	
	void ThreadHandle::Join()
	{
		mThread.join();
	}

	//---------------------------------------------------------------------------
	static ThreadHandle* CreateThreadHandle(Thread* ThreadInstance, int StackSize,
		char* ThreadName)
	{
		return FB_NEW(ThreadHandle)(ThreadInstance, StackSize, ThreadName);
	}

	//---------------------------------------------------------------------------
	Thread::~Thread()
	{
		CHECK(!IsRunning());

		FB_SAFE_DEL(mThreadDesc);
		FB_SAFE_DEL(mThreadHandle);
	}

	//---------------------------------------------------------------------------
	void Thread::CreateThread(int StackSize, char* ThreadName)
	{
		mThreadDesc = FB_NEW(ThreadInfo);
		strcpy(mThreadDesc->ThreadName, ThreadName);

		mThreadHandle = CreateThreadHandle(this, StackSize, mThreadDesc->ThreadName);

		CHECK(mThreadHandle);
		CHECK(mThreadHandle->IsValid());
	}

	//---------------------------------------------------------------------------
	void Thread::RegisterThread()
	{
		CHECK(!GThreadDesc);
		CHECK(mThreadDesc);
		if (!mThreadDesc)
			return;

		mThreadDesc->ThreadID = GetCurrentThreadId();
		GThreadDesc = mThreadDesc;
	}

	//---------------------------------------------------------------------------
	void Thread::RegisterThread(char* ThreadName)
	{
		CHECK(!GThreadDesc);

		GThreadDesc = FB_NEW(ThreadInfo);
		strcpy(GThreadDesc->ThreadName, ThreadName);
		GThreadDesc->ThreadID = GetCurrentThreadId();
	}
}