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
#include "WorkerThread.h"
#include "Task.h"
#include "TaskScheduler.h"
namespace fb
{

	WorkerThread::WorkerThread(TaskScheduler* scheduler)
		: mScheduler(scheduler)
	{
		mTasksEvent = CreateSyncEvent();

		static DWORD ThreadID = 0;
		static char ThreadName[128];
		sprintf_s(ThreadName, "worker_thread_%d", ThreadID++);

		CreateThread(256 * 1024, ThreadName);
	}

	WorkerThread::~WorkerThread()
	{
	}

	void WorkerThread::PrepareQuit() {
		ForceExit(false);
		mTasksEvent->Trigger();
	}

	bool WorkerThread::Run()
	{
#ifdef _PLATFORM_WINDOWS_
		struct Coinit {
			Coinit()
			{
				CoInitializeEx(NULL, COINIT_MULTITHREADED);
			}
			~Coinit()
			{
				CoUninitialize();
			}
		};
		Coinit coinit;
#endif
		mTasksEvent->Wait();

		// Loop executing ready tasks
		while (mCurrentTask)
		{
			// Execute current task
			mCurrentTask->Trigger(mScheduler);

			// Try to get another task
			mCurrentTask = mScheduler->GetNextReadyTask(this);
		}
		mScheduler->AddIdleWorker(this);
		mScheduler->SchedulerSlice();

		return !IsForceExit();
	}

	void WorkerThread::SetTask(TaskPtr t)
	{
		mCurrentTask = t;
		mTasksEvent->Trigger();
	}

}