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
#include "AsyncObjects.h"
#include "FBCommonHeaders/SpinLock.h"
using namespace fb;

//---------------------------------------------------------------------------
CriticalSection::CriticalSection()
{
}

CriticalSection::~CriticalSection()
{
}

void CriticalSection::Lock()
{
	mMutex.lock();
}

void CriticalSection::Unlock()
{
	mMutex.unlock();
}

//---------------------------------------------------------------------------
RecursiveCriticalSection::RecursiveCriticalSection()
{
}

RecursiveCriticalSection::~RecursiveCriticalSection()
{
}

void RecursiveCriticalSection::Lock()
{
	mMutex.lock();
}

void RecursiveCriticalSection::Unlock()
{
	mMutex.unlock();
}

//---------------------------------------------------------------------------
ENTER_CRITICAL_SECTION::ENTER_CRITICAL_SECTION(CriticalSection* cs)
	:mCS(cs)
{
	if (mCS)
		mCS->Lock();
}
ENTER_CRITICAL_SECTION::ENTER_CRITICAL_SECTION(CriticalSection& cs)
	:mCS(&cs)
{
	mCS->Lock();
}
ENTER_CRITICAL_SECTION::~ENTER_CRITICAL_SECTION()
{
	if (mCS)
		mCS->Unlock();
}

ENTER_CRITICAL_SECTION_R::ENTER_CRITICAL_SECTION_R(RecursiveCriticalSection* cs)
	:mCS(cs)
{
	if (mCS)
		mCS->Lock();
}
ENTER_CRITICAL_SECTION_R::ENTER_CRITICAL_SECTION_R(RecursiveCriticalSection& cs)
	: mCS(&cs)
{
	mCS->Lock();
}
ENTER_CRITICAL_SECTION_R::~ENTER_CRITICAL_SECTION_R()
{
	if (mCS)
		mCS->Unlock();
}

//---------------------------------------------------------------------------
ReadWriteCS::ReadWriteCS()
	: mNumReaders(0)
	, mWriting(false)
{
	mC.notify_all();
}
ReadWriteCS::~ReadWriteCS()
{
}
void ReadWriteCS::EnterReader()
{
	using namespace std::chrono_literals;
	std::unique_lock<std::mutex> lg(mMutex);
	while(mWriting)
		mC.wait_for(lg, 100ms);
	++mNumReaders;	
}
void ReadWriteCS::LeaveReader()
{
	--mNumReaders;
	if (mNumReaders == 0) {
		mC.notify_all();
	}
}
void ReadWriteCS::EnterWriter()
{
	using namespace std::chrono_literals;
	std::unique_lock<std::mutex> lg(mMutex);
	while (mWriting || mNumReaders > 0)
		mC.wait_for(lg, 100ms);
	mWriting = true;
}
void ReadWriteCS::LeaveWriter()
{
	mWriting = false;
	mC.notify_all();
}

//---------------------------------------------------------------------------
ReadLock::ReadLock(ReadWriteCS& lock)
	: mReadWrite(&lock)
{
	mReadWrite->EnterReader();
}
ReadLock::~ReadLock()
{
	mReadWrite->LeaveReader();
}

//---------------------------------------------------------------------------
WriteLock::WriteLock(ReadWriteCS& lock)
	: mReadWrite(&lock)
{
	mReadWrite->EnterWriter();
}
WriteLock::~WriteLock()
{
	mReadWrite->LeaveWriter();
}

#if defined(_PLATFORM_WINDOWS_)
class SyncEventWin : public SyncEvent
{
protected:
	HANDLE EventHandle;

public:
	SyncEventWin(bool ManualReset, char* Name)
	{
		
		EventHandle = CreateEventA(NULL, ManualReset, 0, Name);
		assert(EventHandle);
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
};
#else
#endif

SyncEventPtr fb::CreateSyncEvent(bool ManualReset, char* Name)
{
#if defined(_PLATFORM_WINDOWS_)
	return SyncEventPtr(new SyncEventWin(ManualReset, Name), [](SyncEventWin* obj){ delete obj; });
#else
	assert(0 && " Not implemented.");
	return 0;
#endif
}

//---------------------------------------------------------------------------
class ThreadSafeCounter::Impl {
public:
	std::atomic<int> mValue;
	mutable SpinLockWaitNoSleep mLock;

	//---------------------------------------------------------------------------
	Impl()
		:mValue(0)
	{
	}

	Impl(int count)
		:mValue(count)
	{
	}

	int operator*() const {
		//EnterSpinLock<SpinLockWaitNoSleep> lock(mLock);
		return mValue;
	}

	int operator++() {
		//EnterSpinLock<SpinLockWaitNoSleep> lock(mLock);
		return ++mValue;
	}

	int operator --()
	{
		//EnterSpinLock<SpinLockWaitNoSleep> lock(mLock);
		auto value = --mValue;
		return value;
	}

	int operator +=(int Amount)
	{
		//EnterSpinLock<SpinLockWaitNoSleep> lock(mLock);
		mValue += Amount;
		return mValue;
	}

	int operator -=(int Amount)
	{
		//EnterSpinLock<SpinLockWaitNoSleep> lock(mLock);
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

ThreadSafeCounter::~ThreadSafeCounter() {

}

int ThreadSafeCounter::operator *() const {
	return mImpl->operator*();
}

int ThreadSafeCounter::operator ++() {
	return mImpl->operator++();
}

int ThreadSafeCounter::operator --() {
	return mImpl->operator--();
}

int ThreadSafeCounter::operator +=(int Amount) {
	return mImpl->operator+=(Amount);
}

int ThreadSafeCounter::operator -=(int Amount) {
	return mImpl->operator-=(Amount);
}

ThreadSafeCounter::operator int() const {
	return mImpl->mValue;
}