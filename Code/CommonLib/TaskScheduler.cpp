#include <CommonLib/StdAfx.h>
#include <CommonLib/TaskScheduler.h>
#include <CommonLib/CommonLib.h>
#include <CommonLib/System.h>
#include <CommonLib/Task.h>
#include <CommonLib/WorkerThread.h>

#define MAX_NUM_QUEUE 65536
namespace fastbird
{
	TaskScheduler* gTaskScheduler = 0;
	SyncEvent* gScheduleSliceEvent = 0;
	void Scheduler()
	{
		while (!gTaskScheduler->IsFinalized())
		{
			gTaskScheduler->_Schedule();
			gScheduleSliceEvent->Wait(66);
		}
	}
TaskScheduler::TaskScheduler(DWORD NumThreads)
	: mFinalize(false)
{
	gTaskScheduler = this;
	mSchedulingSlices = 0;
    for(int i=0; i<ARRAYCOUNT(mActiveTasksMap); i++)
    {
        mActiveTasksMap[i] = NULL;
    }

    mNumWorkerThreads = (NumThreads == 0) ? GetNumProcessors() : NumThreads;
    assert(mNumWorkerThreads > 0);

    mPendingTasksQueue.Init(MAX_NUM_QUEUE);
    mReadyTasksQueue.Init(MAX_NUM_QUEUE);
    mIdleThreadsQueue.Init(mNumWorkerThreads);
	mWorkerThreads.assign(mNumWorkerThreads, 0);

    for(int i=0; i<mNumWorkerThreads; i++)
    {
		mWorkerThreads[i] = FB_NEW(WorkerThread)(this);
		AddIdleWorker(mWorkerThreads[i]);
    }

    fastbird::Log("Task Scheduler initialized using %d worker threads\n", mNumWorkerThreads);

	gScheduleSliceEvent = CreateSyncEvent(false);
	mSchedulerThread = std::thread(Scheduler);
}

TaskScheduler::~TaskScheduler()
{
	Finalize();
	std::this_thread::sleep_for(std::chrono::microseconds(500));
	for(int i=0; i<mNumWorkerThreads; i++)
    {
		mWorkerThreads[i]->ForceExit(false);		
		mWorkerThreads[i]->SetTask(0);
	}
	std::this_thread::sleep_for(std::chrono::microseconds(500));
	gScheduleSliceEvent->Trigger();
	mSchedulerThread.join();
	DeleteSyncEvent(gScheduleSliceEvent);
	gScheduleSliceEvent = 0;
	for(int i=0; i<mNumWorkerThreads; i++)
    {
		if (!mWorkerThreads[i]->IsRunning())
			FB_SAFE_DEL(mWorkerThreads[i]);
	}
}

void TaskScheduler::SchedulerSlice()
{
	if (mFinalize)
		return;
	if (mSchedulerLock.Lock())
	{
		Task* CurTask;
		while (CurTask = mPendingTasksQueue.Dequeue())
		{
			//Log("Task %d Dequeued from pending", CurTask->mTaskID);
			if (CurTask->mScheduled)
			{
				if (!CurTask->mIsHashed)
				{
					// Fast path: the task is fully executed and not hashed
					if (CurTask->IsExecuted())
					{
						CurTask->OnExecuted();
						//Log("Task %d OnExcuted()", CurTask->mTaskID);

						if (CurTask->mAutoDestroy)
						{
							FB_SAFE_DEL(CurTask);
						}
					}
					else
					{
						// Register task in dependencies hashmap
						CHECK(!CurTask->mHashNext);
						DWORD HashBin = (CurTask->mTaskID) &
							(ARRAYCOUNT(mActiveTasksMap) - 1);
						CurTask->mHashNext = mActiveTasksMap[HashBin];
						assert(mActiveTasksMap[HashBin] != CurTask);
						mActiveTasksMap[HashBin] = CurTask;
						CurTask->mIsHashed = true;
						CurTask->mMyHash = HashBin;
						//Log("Task %d Hashed()", CurTask->mTaskID);
					}
				}
			}
			else
			{
				Task** Dependencies = NULL;
				int NumDependencies = CurTask->GetDependencies(Dependencies);
				if (CurTask->mIsDependency || NumDependencies)
				{
					// Register task in dependencies hashmap
					CHECK(!CurTask->mHashNext);
					DWORD HashBin = (CurTask->mTaskID) &
						(ARRAYCOUNT(mActiveTasksMap) - 1);
					CurTask->mHashNext = mActiveTasksMap[HashBin];
					assert(mActiveTasksMap[HashBin] != CurTask);
					CurTask->mIsHashed = true;
					mActiveTasksMap[HashBin] = CurTask;
				}

				if (!NumDependencies)
				{
					// Schedule ready tasks
					ScheduleTask(CurTask);
				}
			}
		}

		// Schedule ready tasks with dependencies
		for (int i = 0; i < ARRAYCOUNT(mActiveTasksMap); i++)
		{
			Task* CurTask = mActiveTasksMap[i];
			Task** PrevLink = &mActiveTasksMap[i];

			while (CurTask)
			{
				CHECK(CurTask->mIsHashed);

				bool UnHashTask = false;
				bool DestroyTask = false;

				if (!CurTask->mScheduled)
				{
					bool TaskIsReady = true;
					Task** Dependencies = NULL;
					int NumDependencies = CurTask->GetDependencies(Dependencies);

					for (int j = 0; j < NumDependencies; j++)
					{
						Task* ChildTask = Dependencies[j];

						DWORD HashBin = (ChildTask->mTaskID) &
							(ARRAYCOUNT(mActiveTasksMap) - 1);
						Task* HashTask = mActiveTasksMap[HashBin];

						while (HashTask)
						{
							if ((HashTask == ChildTask) && !ChildTask->IsExecuted())
							{
								TaskIsReady = false;
								break;
							}

							HashTask = HashTask->mHashNext;
						}
					}

					if (TaskIsReady)
					{
						ScheduleTask(CurTask);
						UnHashTask = !CurTask->mIsDependency;
					}
				}
				else if (CurTask->IsExecuted())
				{
					CurTask->OnExecuted();
					//Log("Hashed Task %d OnExecuted()", CurTask->mTaskID);
					DestroyTask = CurTask->mAutoDestroy;
					UnHashTask = true;
				}

				if (UnHashTask)
				{
					*PrevLink = CurTask->mHashNext;
					CurTask->mIsHashed = false;
					//Log("Hashed Task %d is Unhashed()", CurTask->mTaskID);
				}
				else
				{
					PrevLink = &CurTask->mHashNext;
				}

				Task* NextTask = CurTask->mHashNext;

				if (DestroyTask)
				{
					FB_SAFE_DEL(CurTask);
				}

				CurTask = NextTask;
			}
		}
		mSchedulerLock.Unlock();
	}// if mSchedulerLock
	else
	{
		gScheduleSliceEvent->Trigger();
	}
}

void TaskScheduler::_Schedule()
{
	SchedulerSlice();
}

Task* TaskScheduler::GetNextReadyTask(WorkerThread* Thread)
{
	if (mFinalize)
		return 0;
	LOCK_CRITICAL_SECTION lock(mQueueCS);

    Task* ReadyTask = mReadyTasksQueue.Dequeue();
    if(ReadyTask)
    {
        return ReadyTask;
    }

    return NULL;
}

WorkerThread* TaskScheduler::GetNextIdleThread(Task* t)
{
	if (mFinalize)
		return 0;
	LOCK_CRITICAL_SECTION lock(mQueueCS);

    return mIdleThreadsQueue.Dequeue();
}

void TaskScheduler::ScheduleTask(Task* t)
{
	if (mFinalize)
		return;
    t->mScheduled = true;
    WorkerThread* WorkerThread = GetNextIdleThread(t);
    if(WorkerThread)
    {
        WorkerThread->SetTask(t);		
    }
	else
	{
		AddReadyTask(t);
	}
}

void TaskScheduler::AddTask(Task* NewTask)
{
	if (mFinalize)
		return;
	assert(NewTask);
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
		AddPendingTask(NewTask);
        SchedulerSlice();
    }
}

void TaskScheduler::Finalize()
{
	mFinalize=true;
}

void TaskScheduler::AddIdleWorker(WorkerThread* w)
{
	if (mFinalize)
		return;
	LOCK_CRITICAL_SECTION lock(mQueueCS);
	VERIFY(mIdleThreadsQueue.Enqueue(w));
}

void TaskScheduler::AddReadyTask(Task* t)
{
	if (mFinalize)
		return;
	LOCK_CRITICAL_SECTION lock(mQueueCS);
	VERIFY(mReadyTasksQueue.Enqueue(t));
}

void TaskScheduler::AddPendingTask(Task* t)
{
	if (mFinalize)
		return;
	LOCK_CRITICAL_SECTION lock(mQueueCS);
	VERIFY(mPendingTasksQueue.Enqueue(t));
	//Log("Task %d added to pending", t->mTaskID);
}

}