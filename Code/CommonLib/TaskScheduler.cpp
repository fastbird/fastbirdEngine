#include <CommonLib/StdAfx.h>
#include <CommonLib/TaskScheduler.h>
#include <CommonLib/CommonLib.h>
#include <CommonLib/System.h>
#include <CommonLib/Task.h>
#include <CommonLib/WorkerThread.h>

namespace fastbird
{

TaskScheduler::TaskScheduler(DWORD NumThreads)
	: mFinalize(false)
{
    mSchedulingSlices = 0;

    for(int i=0; i<ARRAYCOUNT(mActiveTasksMap); i++)
    {
        mActiveTasksMap[i] = NULL;
    }

    mNumWorkerThreads = (NumThreads == 0) ? GetNumProcessors() : NumThreads;
    assert(mNumWorkerThreads > 0);

    mWorkerThreads = (WorkerThread**)malloc(mNumWorkerThreads * sizeof(WorkerThread**));

    mPendingTasksQueue.Init(MAX_NUM_QUEUE);
    mReadyTasksQueue.Init(MAX_NUM_QUEUE);
    mIdleThreadsQueue.Init(mNumWorkerThreads);

    for(int i=0; i<mNumWorkerThreads; i++)
    {
        mWorkerThreads[i] = new WorkerThread(this);
        VERIFY(mIdleThreadsQueue.Enqueue(mWorkerThreads[i]));
    }

    fastbird::Log("Task Scheduler initialized using %d worker threads\n", mNumWorkerThreads);
}

TaskScheduler::~TaskScheduler()
{
	for(int i=0; i<mNumWorkerThreads; i++)
    {
		mWorkerThreads[i]->ForceExit(false);
		mWorkerThreads[i]->SetTask(0);		
	}
	Sleep(100);
	for(int i=0; i<mNumWorkerThreads; i++)
    {
		if (!mWorkerThreads[i]->IsRunning())
			SAFE_DELETE(mWorkerThreads[i]);
	}
	free(mWorkerThreads);
}

void TaskScheduler::SchedulerSlice()
{
	if (mFinalize)
		return;
    InterlockedIncrement(&mSchedulingSlices);

    if(mSchedulerLock.Lock())
    {
        while(mSchedulingSlices)
        {
            // Schedule waiting tasks
            Task* CurTask;
            while(CurTask = mPendingTasksQueue.Dequeue())
            {
                if(CurTask->mScheduled)
                {
                    if(!CurTask->mIsHashed)
                    {
                        // Fast path: the task is fully executed and not hashed
                        if(CurTask->IsExecuted())
                        {
                            CurTask->OnExecuted();

                            if(CurTask->mAutoDestroy)
                            {
                                delete CurTask;
                            }
                        }
                        else
                        {
                            // Register task in dependencies hashmap
                            CHECK(!CurTask->mHashNext);
                            DWORD HashBin = (CurTask->mTaskID) & 
								(ARRAYCOUNT(mActiveTasksMap) - 1);
                            CurTask->mHashNext = mActiveTasksMap[HashBin];
                            CurTask->mIsHashed = true;
                            mActiveTasksMap[HashBin] = CurTask;
                        }
                    }
                }
                else
                {
                    Task** Dependencies = NULL;
                    int NumDependencies = CurTask->GetDependencies(Dependencies);

                    if(CurTask->mIsDependency || NumDependencies)
                    {
                        // Register task in dependencies hashmap
                        CHECK(!CurTask->mHashNext);
                        DWORD HashBin = (CurTask->mTaskID) & 
							(ARRAYCOUNT(mActiveTasksMap) - 1);
                        CurTask->mHashNext = mActiveTasksMap[HashBin];
                        CurTask->mIsHashed = true;
                        mActiveTasksMap[HashBin] = CurTask;
                    }

                    if(!NumDependencies)
                    {
                        // Schedule ready tasks
                        ScheduleTask(CurTask);
                    }
                }
            }

            // Schedule ready tasks with dependencies
            for(int i=0; i<ARRAYCOUNT(mActiveTasksMap); i++)
            {
                Task* CurTask = mActiveTasksMap[i];
                Task** PrevLink = &mActiveTasksMap[i];

                while(CurTask)
                {
                    CHECK(CurTask->mIsHashed);

                    bool UnHashTask = false;
                    bool DestroyTask = false;

                    if(!CurTask->mScheduled)
                    {
                        bool TaskIsReady = true;
                        Task** Dependencies = NULL;
                        int NumDependencies = CurTask->GetDependencies(Dependencies);

                        for(int j=0; j<NumDependencies; j++)
                        {
                            Task* ChildTask = Dependencies[j];

                            DWORD HashBin = (ChildTask->mTaskID) & 
								(ARRAYCOUNT(mActiveTasksMap) - 1);
                            Task* HashTask = mActiveTasksMap[HashBin];

                            while(HashTask)
                            {
                                if((HashTask == ChildTask) && !ChildTask->IsExecuted())
                                {
                                    TaskIsReady = false;
                                    break;
                                }

                                HashTask = HashTask->mHashNext;
                            }
                        }

                        if(TaskIsReady)
                        {
                            ScheduleTask(CurTask);
                            UnHashTask = !CurTask->mIsDependency;
                        }
                    }
                    else if(CurTask->IsExecuted())
                    {
                        CurTask->OnExecuted();
                        DestroyTask = CurTask->mAutoDestroy;
                        UnHashTask = true;
                    }

                    if(UnHashTask)
                    {
                        *PrevLink = CurTask->mHashNext;
                        CurTask->mIsHashed = false;
                    }
                    else
                    {
                        PrevLink = &CurTask->mHashNext;
                    }

                    Task* NextTask = CurTask->mHashNext;

                    if(DestroyTask)
                    {
                        delete CurTask;
                    }

                    CurTask = NextTask;
                }
            }

            InterlockedDecrement(&mSchedulingSlices);
        }

        mSchedulerLock.Unlock();
    }
}

Task* TaskScheduler::GetNextReadyTask(WorkerThread* Thread)
{
	if (mFinalize)
		return 0;
    mQueuesLock.Lock();

    Task* ReadyTask = mReadyTasksQueue.Dequeue();

    if(ReadyTask)
    {
        mQueuesLock.Unlock();
        return ReadyTask;
    }

    VERIFY(mIdleThreadsQueue.Enqueue(Thread));

    mQueuesLock.Unlock();
    return NULL;
}

WorkerThread* TaskScheduler::GetNextIdleThread(Task* Task)
{
	if (mFinalize)
		return 0;
    mQueuesLock.Lock();

    WorkerThread* IdleThread = mIdleThreadsQueue.Dequeue();

    if(IdleThread)
    {
        mQueuesLock.Unlock();
        return IdleThread;
    }

    VERIFY(mReadyTasksQueue.Enqueue(Task));

    mQueuesLock.Unlock();
    return NULL;
}

void TaskScheduler::ScheduleTask(Task* Task)
{
	if (mFinalize)
		return;
    Task->mScheduled = true;

    WorkerThread* WorkerThread = GetNextIdleThread(Task);

    if(WorkerThread)
    {
        WorkerThread->SetTask(Task);
    }
}

void TaskScheduler::AddTask(Task* NewTask)
{
	if (mFinalize)
		return;
	assert(NewTask);
	NewTask->SetAdded(true);
    Task** Dependencies = NULL;
    int NumDependencies = NewTask->GetDependencies(Dependencies);

    // Recursivly add dependant tasks
    for(int i=0; i<NumDependencies; i++)
    {
        Dependencies[i]->mIsDependency = true;
        AddTask(Dependencies[i]);
    }

    if(!NumDependencies && !NewTask->mIsDependency)
    {
        // The task is ready to execute; schedule it right away
        ScheduleTask(NewTask);
    }
    else
    {
        // Since the task has dependencies, we register it internally.
        VERIFY(mPendingTasksQueue.Enqueue(NewTask));

        SchedulerSlice();
    }
}

void TaskScheduler::Finalize()
{
	mFinalize=true;
}

}