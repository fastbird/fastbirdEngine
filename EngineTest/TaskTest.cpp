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
#include "TaskTest.h"
#include "FBThread/TaskScheduler.h"
#include "FBThread/Task.h"
using namespace fb;

std::atomic<size_t> gNumExecuted = 0;
class MyTask : public Task {
public:
	MyTask(ThreadSafeCounter* _ExecCounter = nullptr) 
		: Task(false, _ExecCounter)
	{
		if (mExecCounter) {
			_ExecCounter->operator++();
		}
	}
	void Execute(TaskScheduler* Scheduler) OVERRIDE {
		if (!mExecCounter) {
			// 5 children tasks
			for (int i = 0; i < 5; ++i) {
				auto t = std::make_shared<MyTask>(&mSyncCounter);
				Scheduler->AddTask(t);
			}
		}
		double v = 1;
		for (int i = 1; i < 10000; ++i) {
			v += Random(-1.f, 1.f);
		}		
		Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("[%u]V= %f; num executed = %u", std::this_thread::get_id(), v, ++gNumExecuted).c_str());
	}
};
class TaskTest::Impl {
public:
	

	Impl() {
		for (int i = 0; i < 5000; ++i) {
			auto t = std::make_shared<MyTask>();
			TaskScheduler::GetInstance().AddTask(t);
		}
	}

	~Impl() {

	}

};

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(TaskTest);
TaskTest::TaskTest()
	: mImpl(new Impl)
{

}

TaskTest::~TaskTest() {

}