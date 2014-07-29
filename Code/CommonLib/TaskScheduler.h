#pragma once
#include <CommonLib/threads.h>

namespace fastbird
{

class TaskScheduler
{
	static const int MAX_NUM_QUEUE = 65536;
    friend class Task;
    friend class WorkerThread;

    // Array of worker threads
    WorkerThread** mWorkerThreads;
    int mNumWorkerThreads;

    // Lock used to check if a scheduling slice is already running (never waits)
    SpinLock<false, false> mSchedulerLock;

    // Lock used to keep the pending tasks and idle threads queues consistent 
	// (rarely spins more than a couple of times)
    SpinLock<true, false> mQueuesLock;

    // Hash table of active tasks
    Task* mActiveTasksMap[1024];

    // Internal queues
    LockFreeQueue<Task> mPendingTasksQueue;
    LockFreeQueue<Task> mReadyTasksQueue;
    LockFreeQueue<WorkerThread> mIdleThreadsQueue;

    // Used to maintain the number of scheduling slices requested
    volatile long mSchedulingSlices;

    Task* GetNextReadyTask(WorkerThread* Thread);
    WorkerThread* GetNextIdleThread(Task* Task);

    void ScheduleTask(Task* Task);
    void SchedulerSlice();

	bool mFinalize;

public:
    TaskScheduler(DWORD NumThreads = 0);
	~TaskScheduler();

    void AddTask(Task* NewTask);
	void Finalize();
};

}