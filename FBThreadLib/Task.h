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

#pragma once
#include <atomic>
#include "threads.h"
namespace fb
{
FB_DECLARE_SMART_PTR(SyncEvent);
FB_DECLARE_SMART_PTR(Task);
class Task : std::enable_shared_from_this<Task>
{
    friend class TaskScheduler;
    friend class WorkerThread;

protected:
	
  void Trigger(TaskScheduler* Scheduler);
	virtual int GetDependencies(TaskPtr Dependencies[]);

    // Called by the scheduler when the task is fully executed.
    virtual void OnExecuted();

    unsigned mTaskID;              // Internal task-id, used for task hashing.
    TaskPtr mHashNext;            // Internal hashmap next element pointer.
	unsigned mMyHash;	
	ThreadSafeCounter* mExecCounter;	// Pointer to a variable that gets decremented when execution is done.
	ThreadSafeCounter mSyncCounter;  // Used to wait for subtasks to complete.
	SyncEventPtr mWaitEvent;       // Event used to wait for a task to complete.
	std::atomic<bool> mExecuted;          // Is this task executed?
	std::atomic<bool> mIsHashed;          // Is this task in the dependencies hashmap?
  bool mScheduled : 1;         // Is this task scheduled?
  bool mIsDependency : 1;      // Is this task a dependency for another task?
	bool mTriggered : 1;
    

public:
	Task(bool _WaitEvent = false, ThreadSafeCounter* _ExecCounter = NULL);
	virtual ~Task();
  virtual void Execute(TaskScheduler* Scheduler)=0;

	bool IsExecuted() const;
	bool IsTriggered() const;

	// wait to finish
	void Sync();
	// to re add.
	virtual void Reset();
};

}