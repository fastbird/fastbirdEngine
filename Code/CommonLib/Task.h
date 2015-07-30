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
	DWORD mMyHash;
    volatile long* mExecCounter;	// Pointer to a variable that gets decremented when execution is done.
    volatile long mSyncCounter;  // Used to wait for subtasks to complete.
    HANDLE mWaitEvent;       // Event used to wait for a task to complete.
	std::atomic<bool> mExecuted;          // Is this task executed?
	std::atomic<bool> mIsHashed;          // Is this task in the dependencies hashmap?
    bool mScheduled : 1;         // Is this task scheduled?
    bool mAutoDestroy : 1;       // Is this task automatically destroyed after execution?
    bool mIsDependency : 1;      // Is this task a dependency for another task?
    

public:
    Task(bool _AutoDestroy = true, bool _WaitEvent = false, 
		volatile long* _ExecCounter = NULL);
    virtual ~Task();

    virtual void Execute(TaskScheduler* Scheduler)=0;

    bool IsExecuted() const
    {
        return mExecuted && !mSyncCounter;
    }

	// wait to finish
	void Sync();

	// to re add.
	virtual void Reset();
};

}