#pragma once
#include <CommonLib/threads.h>

namespace fastbird
{
class Task;
class TaskScheduler;
class WorkerThread: public Thread
{
protected:
    Task* mCurrentTask;          // Current executing task.
    SyncEvent* mTasksEvent;      // Event used when waiting for a task.
    TaskScheduler* mScheduler;          // Scheduler owning this worker thread.

	FB_CRITICAL_SECTION mTaskCS;

public:
    WorkerThread(TaskScheduler* scheduler);
	~WorkerThread();

    void SetTask(Task* t);
	virtual bool Run();
};

}