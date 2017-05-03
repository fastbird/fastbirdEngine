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
#include "Task.h"
#include "TaskScheduler.h"
#include "threads.h"
#include "AsyncObjects.h"
namespace fb
{
static ThreadSafeCounter counter;
Task::Task(bool _WaitEvent, ThreadSafeCounter* _ExecCounter)
	: mScheduled(false)
	, mExecuted(false)
	, mIsDependency(false)
	, mIsHashed(false)
	, mExecCounter(_ExecCounter)
	, mSyncCounter(0)
	, mHashNext(NULL)
	, mMyHash(-1)
	, mTriggered(false)
{
	static volatile DWORD UniqueTaskID = 0;	
	mTaskID = ++counter;		
	if (_WaitEvent){
		mWaitEvent = CreateSyncEvent(true);
	}
}

Task::~Task()
{
}

void Task::OnExecuted()
{
	if(mExecCounter)
	{
		mExecCounter->operator--();
	}

	if(mWaitEvent)
	{
		mWaitEvent->Trigger();
	}
}

void Task::Trigger(TaskScheduler* pScheduler)
{
	mTriggered = true;
	Execute(pScheduler);
	mExecuted = true;

  if(!mIsHashed && IsExecuted())
  {
		OnExecuted();
  }
  else
  {
		pScheduler->AddPendingTask(this->shared_from_this());		
		pScheduler->SchedulerSlice();
  }	
}

int Task::GetDependencies(TaskPtr Dependencies[])
{
	Dependencies = NULL;
	return 0;
}

void Task::Sync()
{
	if (mWaitEvent)
		mWaitEvent->Wait();	
}

void Task::Reset()
{
	Sync();
	mScheduled = false;
	mExecuted = false;
	mTriggered = false;
	if (mWaitEvent)
		mWaitEvent->Reset();
}

bool Task::IsExecuted() const
{
	return mExecuted && *mSyncCounter==0;
}

bool Task::IsTriggered() const{
	return mTriggered;
}
}