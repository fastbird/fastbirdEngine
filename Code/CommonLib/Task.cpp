#include <CommonLib/StdAfx.h>
#include <CommonLib/Task.h>
#include <CommonLib/TaskScheduler.h>

namespace fastbird
{

Task::Task(bool _AutoDestroy, bool _WaitEvent, volatile long* _ExecCounter)
	: mScheduled(false)
	, mExecuted(false)
	, mAutoDestroy(_AutoDestroy)
	, mIsDependency(false)
	, mIsHashed(false)
	, mIsAdded(false)
	, mExecCounter(_ExecCounter)
	, mSyncCounter(0)
	, mHashNext(NULL)
	, mWaitEvent(0)
{
    assert(!_WaitEvent || !_AutoDestroy);

    static volatile DWORD UniqueTaskID = 0;
    mTaskID = InterlockedIncrement((DWORD*)&UniqueTaskID);

    mWaitEvent = _WaitEvent ? CreateEvent(0, FALSE, FALSE, 0) : NULL;
}

Task::~Task()
{
    if(mWaitEvent)
    {
        CloseHandle(mWaitEvent);
    }
}

void Task::OnExecuted()
{
    if(mExecCounter)
    {
        InterlockedDecrement((DWORD*)mExecCounter);
    }

    if(mWaitEvent)
    {
		SetEvent(mWaitEvent);
    }
	mIsAdded = false;
}

void Task::Trigger(TaskScheduler* pScheduler)
{
    Execute(pScheduler);

	mExecuted = true;

    if(!mIsHashed && IsExecuted())
    {
        OnExecuted();

        if(mAutoDestroy)
        {
            delete this;
        }
    }
    else
    {
		VERIFY(pScheduler->mPendingTasksQueue.Enqueue(this));
    }
}

void Task::Sync()
{
	if (mIsAdded)
		WaitForSingleObject(mWaitEvent, INFINITE);
}

void Task::Reset()
{
	Sync();
	mIsAdded = false;
	mScheduled = false;
	mExecuted = false;
	ResetEvent(mWaitEvent);
}

}