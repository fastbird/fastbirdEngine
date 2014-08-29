#pragma once
#include <CommonLib/threads.h>
#include <CommonLib/SpinLock.h>
#include <CommonLib/LockFreeQueue.h>
namespace fastbird
{

class TaskScheduler
{
    friend class Task;
    friend class WorkerThread;
	
	std::thread mSchedulerThread;
    // Array of worker threads
    std::vector<WorkerThread*> mWorkerThreads;
    int mNumWorkerThreads;

    // Lock used to check if a scheduling slice is already running (never waits)
    SpinLock<false, false> mSchedulerLock;

    // Hash table of active tasks
    Task* mActiveTasksMap[1024];

    // Internal queues
    LockFreeQueue<Task> mPendingTasksQueue;
    LockFreeQueue<Task> mReadyTasksQueue;
    LockFreeQueue<WorkerThread> mIdleThreadsQueue;

	FB_CRITICAL_SECTION mQueueCS;

    // Used to maintain the number of scheduling slices requested
    volatile long mSchedulingSlices;

    Task* GetNextReadyTask(WorkerThread* Thread);
    WorkerThread* GetNextIdleThread(Task* t);
	void AddIdleWorker(WorkerThread* w);
	void AddReadyTask(Task* t);
	void AddPendingTask(Task* t);

    void ScheduleTask(Task* t);
	void SchedulerSlice();

	bool mFinalize;

public:
    TaskScheduler(DWORD NumThreads = 0);
	~TaskScheduler();

    void AddTask(Task* NewTask);
	void Finalize();

	bool IsFinalized() { return mFinalize; }
	// don't need to call this explicitly.
	void _Schedule();
};

}