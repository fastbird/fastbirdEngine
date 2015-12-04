/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#include "stdafx.h"
#include "threads.h"
#include "FBCommonHeaders/platform.h"
#include <thread>
#include <atomic>

namespace fb
{
__declspec(thread) ThreadInfo* GThreadDesc = 0;

void SwitchThread()
{
	std::this_thread::yield();
}

struct ThreadNameInfo
{
	DWORD dwType;     // must be 0x1000
	const char* szName;    // pointer to name (in user address space)
	DWORD dwThreadID; // thread ID (-1 = caller thread)
	DWORD dwFlags;    // reserved for future use, must be zero
};

void SetThreadName(DWORD ThreadID, const char* ThreadName)
{
	ThreadNameInfo Info;

	Info.dwType = 0x1000;
	Info.szName = ThreadName;
	Info.dwThreadID = ThreadID;
	Info.dwFlags = 0;
#if defined(_PLATFORM_WINDOWS_)
	__try
	{
		RaiseException(0x406D1388, 0, sizeof(Info) / sizeof(DWORD), (DWORD*)&Info);
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
	}
#else
	assert(0 && "Not implemented");
#endif
}

static ThreadHandlePtr CreateThreadHandle(Thread* ThreadInstance, int StackSize,
	char* ThreadName)
{
	return ThreadHandlePtr(new ThreadHandle(ThreadInstance, StackSize, ThreadName), [](ThreadHandle* obj){ delete obj; });
}

//---------------------------------------------------------------------------
class ThreadSafeCounter::Impl{
public:
	std::atomic<int> mValue;

	//---------------------------------------------------------------------------
	Impl()
		:mValue(0)
	{
	}

	Impl(int count)
		:mValue(count)
	{
	}

	int operator*() const{
		return mValue;
	}

	int operator++(){
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

ThreadSafeCounter::ThreadSafeCounter()
	: mImpl(new Impl)
{

}
ThreadSafeCounter::ThreadSafeCounter(int _Value)
	: mImpl(new Impl(_Value))
{

}

ThreadSafeCounter::~ThreadSafeCounter(){

}

int ThreadSafeCounter::operator *() const{
	return mImpl->operator*();
}

int ThreadSafeCounter::operator ++(){
	return mImpl->operator++();
}

int ThreadSafeCounter::operator --(){
	return mImpl->operator--();
}

int ThreadSafeCounter::operator +=(int Amount){
	return mImpl->operator+=(Amount);
}

int ThreadSafeCounter::operator -=(int Amount){
	return mImpl->operator-=(Amount);
}

//---------------------------------------------------------------------------
static DWORD __stdcall ThreadProc(LPVOID ThreadPtr)
{
	assert(ThreadPtr);
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
class ThreadHandle::Impl{
public:
	std::thread mThread;

	//---------------------------------------------------------------------------
	Impl(Thread* Thread, int StackSize, char* ThreadName){
		mThread = std::thread(ThreadProc, Thread);
		DWORD ThreadID = GetThreadId(mThread.native_handle());
		SetThreadName(ThreadID, ThreadName);
	}

	HANDLE GetNativeHandle()
	{
		return mThread.native_handle();
	}

	bool IsValid()
	{
		return mThread.joinable();
	}

	void Join()
	{
		if (mThread.joinable())
			mThread.join();
	}
};

//---------------------------------------------------------------------------
ThreadHandle::ThreadHandle(Thread* Thread, int StackSize, char* ThreadName)
	:mImpl(new Impl(Thread, StackSize, ThreadName))
{	
}

ThreadHandle::~ThreadHandle()
{
	Join();
}

HANDLE ThreadHandle::GetNativeHandle()
{
	return mImpl->GetNativeHandle();
}

bool ThreadHandle::IsValid()
{
	return mImpl->IsValid();	
}
	
void ThreadHandle::Join()
{
	return mImpl->Join();
}

//---------------------------------------------------------------------------
class Thread::Impl{
public:
	Thread* mSelf;
	ThreadHandlePtr mThreadHandle;
	ThreadSafeCounter mRunning;
	ThreadSafeCounter mForcedExit;

	Impl(Thread* self)
		:mSelf(self)
		
	{
		mSelf->mThreadDesc = 0;
	}
	~Impl(){
		delete mSelf->mThreadDesc;
	}

	void CreateThread(int StackSize, const char* ThreadName){
		if (mSelf->mThreadDesc)
		{
			Logger::Log(FB_ERROR_LOG_ARG, "Already created.");
			return;
		}
		mSelf->mThreadDesc = new ThreadInfo;
		strcpy_s(mSelf->mThreadDesc->ThreadName, ThreadName);
		mThreadHandle = CreateThreadHandle(mSelf, StackSize, mSelf->mThreadDesc->ThreadName);
		assert(mThreadHandle);
		assert(mThreadHandle->IsValid());
	}

	void RegisterThread(){
		assert(!GThreadDesc);
		assert(mSelf->mThreadDesc);
		if (!mSelf->mThreadDesc)
			return;

		mSelf->mThreadDesc->mThreadID = std::this_thread::get_id();
		GThreadDesc = mSelf->mThreadDesc;
	}

	void StartRun()
	{
		assert(!*mRunning);
		++mRunning;
	}

	void EndRun()
	{
		assert(*mRunning);
		--mRunning;
	}

	bool IsForceExit()
	{
		return *mForcedExit != 0;
	}

	void ForceExit(bool Wait)
	{
		++mForcedExit;

		while (Wait && IsRunning())
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
		if (mThreadHandle)
			mThreadHandle->Join();
	}

	bool IsJoinable(){
		return mThreadHandle && mThreadHandle->IsValid();
	}
};

//---------------------------------------------------------------------------
Thread::Thread()
	:mImpl(new Impl(this))
{

}

Thread::~Thread(){

}

void Thread::CreateThread(int StackSize, const char* ThreadName)
{
	mImpl->CreateThread(StackSize, ThreadName);
}


void Thread::RegisterThread()
{
	mImpl->RegisterThread();
}

// static
void Thread::RegisterThread(const char* ThreadName)
{
	assert(!GThreadDesc);

	GThreadDesc = new (ThreadInfo); // deleting is caller's responsibility.
	strcpy_s(GThreadDesc->ThreadName, ThreadName);
	GThreadDesc->mThreadID = std::this_thread::get_id();
}

void Thread::StartRun(){
	mImpl->StartRun();
}

void Thread::EndRun(){
	mImpl->EndRun();
}

bool Thread::IsForceExit(){
	return mImpl->IsForceExit();
}

void Thread::ForceExit(bool Wait){
	mImpl->ForceExit(Wait);
}

bool Thread::IsRunning(){
	return mImpl->IsRunning();
}

bool Thread::IsJoinable(){
	return mImpl->IsJoinable();
}

void Thread::Join(){
	mImpl->Join();
}

}