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
using namespace fb;

//---------------------------------------------------------------------------
FB_CRITICAL_SECTION::FB_CRITICAL_SECTION()
{
}

FB_CRITICAL_SECTION::~FB_CRITICAL_SECTION()
{
}

void FB_CRITICAL_SECTION::Lock()
{
	mMutex.lock();
}

void FB_CRITICAL_SECTION::Unlock()
{
	mMutex.unlock();
}

//---------------------------------------------------------------------------
FB_CRITICAL_SECTION_R::FB_CRITICAL_SECTION_R()
{
}

FB_CRITICAL_SECTION_R::~FB_CRITICAL_SECTION_R()
{
}

void FB_CRITICAL_SECTION_R::Lock()
{
	mMutex.lock();
}

void FB_CRITICAL_SECTION_R::Unlock()
{
	mMutex.unlock();
}

//---------------------------------------------------------------------------
ENTER_CRITICAL_SECTION::ENTER_CRITICAL_SECTION(FB_CRITICAL_SECTION* cs)
	:mCS(cs)
{
	mCS->Lock();
}
ENTER_CRITICAL_SECTION::ENTER_CRITICAL_SECTION(FB_CRITICAL_SECTION& cs)
	:mCS(&cs)
{
	mCS->Lock();
}
ENTER_CRITICAL_SECTION::~ENTER_CRITICAL_SECTION()
{
	mCS->Unlock();
}

ENTER_CRITICAL_SECTION_R::ENTER_CRITICAL_SECTION_R(FB_CRITICAL_SECTION_R* cs)
	:mCS(cs)
{
	mCS->Lock();
}
ENTER_CRITICAL_SECTION_R::ENTER_CRITICAL_SECTION_R(FB_CRITICAL_SECTION_R& cs)
	: mCS(&cs)
{
	mCS->Lock();
}
ENTER_CRITICAL_SECTION_R::~ENTER_CRITICAL_SECTION_R()
{
	mCS->Unlock();
}

//---------------------------------------------------------------------------
FB_READ_WRITE_CS::FB_READ_WRITE_CS()
	: mNumReaders(0)
	, mWriting(false)
{
	mC.notify_all();
}
FB_READ_WRITE_CS::~FB_READ_WRITE_CS()
{
}
void FB_READ_WRITE_CS::EnterReader()
{
	std::unique_lock<std::mutex> lg(mMutex);
	while(mWriting)
		mC.wait(lg);
	++mNumReaders;	
}
void FB_READ_WRITE_CS::LeaveReader()
{
	--mNumReaders;
	if (mNumReaders == 0) {
		mC.notify_one();
	}
}
void FB_READ_WRITE_CS::EnterWriter()
{
	std::unique_lock<std::mutex> lg(mMutex);
	while (mWriting || mNumReaders > 0)
		mC.wait(lg);
	mWriting = true;
}
void FB_READ_WRITE_CS::LeaveWriter()
{
	mWriting = false;
	mC.notify_all();
}

//---------------------------------------------------------------------------
READ_LOCK::READ_LOCK(FB_READ_WRITE_CS& lock)
	: mReadWrite(&lock)
{
	mReadWrite->EnterReader();
}
READ_LOCK::~READ_LOCK()
{
	mReadWrite->LeaveReader();
}

//---------------------------------------------------------------------------
WRITE_LOCK::WRITE_LOCK(FB_READ_WRITE_CS& lock)
	: mReadWrite(&lock)
{
	mReadWrite->EnterWriter();
}
WRITE_LOCK::~WRITE_LOCK()
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