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

#include "stdafx.h"
#include "TaskScheduler.h"
#include "FBCommonHeaders/LockFreeQueue.h"
#include "FBCommonHeaders/LockQueue.h"
#include "FBCommonHeaders/SpinLock.h"
#include "AsyncObjects.h"
#include "WorkerThread.h"
#include "Task.h"
#include "FBCommonHeaders/Helpers.h"
#include "FBSystemLib/System.h"
#include <thread>
using namespace fb;


static const int MAX_NUM_TASK = 2048;
bool sFinalize = false;
namespace fb
{
	TaskScheduler* gTaskScheduler = 0;
}

class TaskScheduler::Impl{
public:
	TaskScheduler* mSelf;
	//std::thread mSchedulerThread;
	// Array of worker threads
	std::vector<WorkerThread*> mWorkerThreads;
	int mNumWorkerThreads;

	// Lock used to check if a scheduling slice is already running (never waits)
	SpinLock<false, false> mSchedulerLock;

	// Hash table of active tasks
	TaskPtr mActiveTasksMap[1024];

	// Internal queues
	LockQueue<TaskPtr> mPendingTasksQueue;
	LockQueue<TaskPtr> mReadyTasksQueue;
	LockFreeQueue<WorkerThread*> mIdleThreadsQueue;

	bool mExiting = false;

	//---------------------------------------------------------------------------
	Impl(TaskScheduler* self, int numThread)
		: mSelf(self)
	{
		gTaskScheduler = self;
		mNumWorkerThreads = (numThread == 0) ? GetNumProcessors() : numThread;
		assert(mNumWorkerThreads > 0);
	}

	~Impl(){
		sFinalize = true;
		bool prepqreQuitIsNotCalled = gTaskScheduler != nullptr;
		if (prepqreQuitIsNotCalled)
		{
			gTaskScheduler = nullptr;
			std::this_thread::sleep_for(std::chrono::microseconds(500));
			for (int i = 0; i < mNumWorkerThreads; i++)
			{
				mWorkerThreads[i]->ForceExit(false);
				mWorkerThreads[i]->SetTask(0);
			}
			std::this_thread::sleep_for(std::chrono::microseconds(500));			
			for (int i = 0; i < mNumWorkerThreads; i++)
			{
				if (!mWorkerThreads[i]->IsRunning())
					delete mWorkerThreads[i];
			}
		}
	}

	void Init(){		
		for (int i = 0; i<ARRAYCOUNT(mActiveTasksMap); i++)
		{
			mActiveTasksMap[i] = NULL;
		}		

		mWorkerThreads.assign(mNumWorkerThreads, 0);

		for (int i = 0; i<mNumWorkerThreads; i++)
		{
			mWorkerThreads[i] = new WorkerThread(mSelf);
			AddIdleWorker(mWorkerThreads[i]);
		}

		Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("Task Scheduler initialized using %d worker threads\n", mNumWorkerThreads).c_str());
		//mSchedulerThread = std::thread(Scheduler);
	}
	//---------------------------------------------------------------------------
	// Public
	//---------------------------------------------------------------------------
	void SetWorkerPriority(int priority) {
		for (auto it : mWorkerThreads) {
			it->SetPriority(priority);
		}
	}

	void AddTask(TaskPtr NewTask){
		if (sFinalize || mExiting)
			return;		
		assert(NewTask);
		TaskPtr* Dependencies = NULL;
		int NumDependencies = NewTask->GetDependencies(Dependencies);

		// Recursivly add dependant tasks
		for (int i = 0; i<NumDependencies; i++)
		{
			Dependencies[i]->mIsDependency = true;
			AddTask(Dependencies[i]);
		}

		if (!NumDependencies && !NewTask->mIsDependency)
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
		constexpr size_t halfnum = MAX_NUM_TASK / 2;
		auto count = mReadyTasksQueue.GetCount();
		if (count >= MAX_NUM_TASK) {
			Logger::Log(FB_ERROR_LOG_ARG, "TaskScheduler is full.");
		}
		else if (count >= halfnum) {
			Logger::Log(FB_ERROR_LOG_ARG, "TaskScheduler is half-full.");
		}
	}

	bool IsFull() const
	{
		return mPendingTasksQueue.GetCount() + mReadyTasksQueue.GetCount() > MAX_NUM_TASK;
	}

	bool IsHalfFull() const {
		return mPendingTasksQueue.GetCount() + mReadyTasksQueue.GetCount() > MAX_NUM_TASK / 2;
	}

	size_t GetNumTasks() const {
		return mPendingTasksQueue.GetCount() + mReadyTasksQueue.GetCount();
	}

	void PrepareQuit() {
		mExiting = true; 
		gTaskScheduler = 0;

		std::this_thread::sleep_for(std::chrono::microseconds(500));
		for (int i = 0; i<mNumWorkerThreads; i++)
		{
			mWorkerThreads[i]->PrepareQuit();
			mWorkerThreads[i]->Join();
		}
		std::this_thread::sleep_for(std::chrono::microseconds(500));		
		for (int i = 0; i<mNumWorkerThreads; i++)
		{
			if (!mWorkerThreads[i]->IsRunning())
				delete mWorkerThreads[i];
		}
		mPendingTasksQueue.Clear();
		mReadyTasksQueue.Clear();
	}

	//---------------------------------------------------------------------------
	// Called by WorkerThread or Task
	//---------------------------------------------------------------------------
	void AddIdleWorker(WorkerThread* w){
		if (sFinalize || mExiting)
			return;		
		mIdleThreadsQueue.Enq(w);
	}

	void AddPendingTask(TaskPtr t){
		if (sFinalize || mExiting)
			return;		
		mPendingTasksQueue.Enq(t);
	}

	void SchedulerSlice(){
		if (sFinalize || mExiting)
			return;
		if (mSchedulerLock.Lock())
		{
			TaskPtr CurTask;
			while (CurTask = mPendingTasksQueue.Deq())
			{				
				if (CurTask->mScheduled)
				{
					if (!CurTask->mIsHashed)
					{						
						if (CurTask->IsExecuted())
						{
							CurTask->OnExecuted();							
							CurTask = 0;
						}
						else
						{
							assert(!CurTask->mHashNext);
							DWORD HashBin = (CurTask->mTaskID) &
								(ARRAYCOUNT(mActiveTasksMap) - 1);
							CurTask->mHashNext = mActiveTasksMap[HashBin];
							assert(mActiveTasksMap[HashBin] != CurTask);
							mActiveTasksMap[HashBin] = CurTask;
							CurTask->mIsHashed = true;
							CurTask->mMyHash = HashBin;							
						}
					}
				}
				else // if not (CurTask->mScheduled)
				{
					TaskPtr* Dependencies = NULL;
					int NumDependencies = CurTask->GetDependencies(Dependencies);
					if (CurTask->mIsDependency || NumDependencies)
					{						
						assert(!CurTask->mHashNext);
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
				TaskPtr CurTask = mActiveTasksMap[i];
				TaskPtr* PrevLink = &mActiveTasksMap[i];

				while (CurTask)
				{
					assert(CurTask->mIsHashed);
					bool UnHashTask = false;

					if (!CurTask->mScheduled)
					{
						bool TaskIsReady = true;
						TaskPtr* Dependencies = NULL;
						int NumDependencies = CurTask->GetDependencies(Dependencies);

						for (int j = 0; j < NumDependencies; j++)
						{
							TaskPtr ChildTask = Dependencies[j];

							DWORD HashBin = (ChildTask->mTaskID) &
								(ARRAYCOUNT(mActiveTasksMap) - 1);
							TaskPtr HashTask = mActiveTasksMap[HashBin];

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

					TaskPtr NextTask = CurTask->mHashNext;

					CurTask = NextTask;
				}
			}
			mSchedulerLock.Unlock();
		}// if mSchedulerLock
	}

	TaskPtr GetNextReadyTask(WorkerThread* Thread){
		if (sFinalize || mExiting)
			return 0;		
		auto nextReadyTask = mReadyTasksQueue.Deq();
		return nextReadyTask;
	}

	//---------------------------------------------------------------------------
	// Internal
	//---------------------------------------------------------------------------
	WorkerThread* GetNextIdleThread(TaskPtr t){
		if (sFinalize || mExiting)
			return 0;		

		return mIdleThreadsQueue.Deq();
	}

	void AddReadyTask(TaskPtr t){
		if (sFinalize || mExiting)
			return;
		
		mReadyTasksQueue.Enq(t);
	}

	void ScheduleTask(TaskPtr t){
		if (sFinalize || mExiting)
			return;
		t->mScheduled = true;
		WorkerThread* WorkerThread = GetNextIdleThread(t);
		if (WorkerThread)
		{
			WorkerThread->SetTask(t);			
		}
		else
		{
			AddReadyTask(t);
		}
	}
};

TaskScheduler* gpTaskSchedularRaw = 0;
TaskSchedulerWeakPtr gpTaskSchedular;

TaskSchedulerPtr TaskScheduler::Create(int numThreads){
	auto p = TaskSchedulerPtr(new TaskScheduler(numThreads), [](TaskScheduler* obj){ delete obj; });
	// The first created schedular is the main.
	if (gpTaskSchedular.expired()) {
		gpTaskSchedular = p;
		gpTaskSchedularRaw = p.get();
	}
	return p;
}

TaskScheduler& TaskScheduler::GetInstance()
{
	return *gpTaskSchedularRaw;
}

bool TaskScheduler::HasInstance() {
	return !gpTaskSchedular.expired();
}

TaskScheduler::TaskScheduler(int NumThreads)
	: mImpl(new Impl(this, NumThreads))
{
	mImpl->Init();
}

TaskScheduler::~TaskScheduler(){
	Logger::Log(FB_DEFAULT_LOG_ARG, "TaskScheduler Deleted");
}

void TaskScheduler::SetWorkerPriority(int priority) {
	mImpl->SetWorkerPriority(priority);
}

void TaskScheduler::AddTask(TaskPtr NewTask){
	mImpl->AddTask(NewTask);
}

bool TaskScheduler::IsFull() const
{
	return mImpl->IsFull();
}

bool TaskScheduler::IsHalfFull() const {
	return mImpl->IsHalfFull();
}

size_t TaskScheduler::GetNumTasks() const {
	return mImpl->GetNumTasks();
}

void TaskScheduler::PrepareQuit()
{
	mImpl->PrepareQuit();
}

bool TaskScheduler::_IsFinalized() const{
	return sFinalize;
}

void TaskScheduler::_Schedule(){
	mImpl->SchedulerSlice();
}

void TaskScheduler::AddIdleWorker(WorkerThread* w){
	mImpl->AddIdleWorker(w);
}

void TaskScheduler::AddPendingTask(TaskPtr t){
	mImpl->AddPendingTask(t);
}

void TaskScheduler::SchedulerSlice(){
	mImpl->SchedulerSlice();
}

TaskPtr TaskScheduler::GetNextReadyTask(WorkerThread* thread){
	return mImpl->GetNextReadyTask(thread);
}