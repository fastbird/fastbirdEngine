#pragma once

namespace fastbird
{

class Task
{
    friend class TaskScheduler;
    friend class WorkerThread;

protected:
    void Trigger(TaskScheduler* Scheduler);

    virtual int GetDependencies(Task**& Dependencies)
    {
        Dependencies = NULL;
        return 0;
    }

    // Called by the scheduler when the task is fully executed.
    virtual void OnExecuted();

    DWORD mTaskID;              // Internal task-id, used for task hashing.
    Task* mHashNext;            // Internal hashmap next element pointer.
    volatile long* mExecCounter;	// Pointer to a variable that gets decremented when execution is done.
    volatile long mSyncCounter;  // Used to wait for subtasks to complete.
    HANDLE mWaitEvent;       // Event used to wait for a task to complete.
    bool mScheduled : 1;         // Is this task scheduled?
    bool mExecuted : 1;          // Is this task executed?
    bool mAutoDestroy : 1;       // Is this task automatically destroyed after execution?
    bool mIsDependency : 1;      // Is this task a dependency for another task?
    bool mIsHashed : 1;          // Is this task in the dependencies hashmap?
	bool mIsAdded : 1;			 // Is added to scheduler

public:
    Task(bool _AutoDestroy = true, bool _WaitEvent = false, 
		volatile long* _ExecCounter = NULL);
    virtual ~Task();

    virtual void Execute(TaskScheduler* Scheduler)=0;

    bool IsExecuted() const
    {
        return mExecuted && !mSyncCounter;
    }

	void SetAdded(bool added) { mIsAdded = added; }
	bool IsAdded() const
	{
		return mIsAdded;
	}

	// wait to finish
	void Sync();

	// to re add.
	void Reset();
};

}