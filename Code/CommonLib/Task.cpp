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
	, mExecCounter(_ExecCounter)
	, mSyncCounter(0)
	, mHashNext(NULL)
	, mMyHash(-1)
	, mTriggered(false)
{
    assert(!_WaitEvent || !_AutoDestroy);

    static volatile DWORD UniqueTaskID = 0;
    mTaskID = InterlockedIncrement((DWORD*)&UniqueTaskID);

	mWaitEvent = _WaitEvent ? CreateEvent(0, TRUE, FALSE, 0) : INVALID_HANDLE_VALUE;
}

Task::~Task()
{
	if (mWaitEvent != INVALID_HANDLE_VALUE)
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

    if(mWaitEvent!=INVALID_HANDLE_VALUE)
    {
		SetEvent(mWaitEvent);
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

        if(mAutoDestroy)
        {
			FB_DELETE(this);
        }
    }
    else
    {
		pScheduler->AddPendingTask(this);
		pScheduler->SchedulerSlice();
    }
}

void Task::Sync()
{
	if (mWaitEvent != INVALID_HANDLE_VALUE)
		WaitForSingleObject(mWaitEvent, INFINITE);
}

void Task::Reset()
{
	Sync();
	mScheduled = false;
	mExecuted = false;
	mTriggered = false;
	if (mWaitEvent != INVALID_HANDLE_VALUE)
		ResetEvent(mWaitEvent);
}

}