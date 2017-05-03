#include "stdafx.h"
#include "TaskProcessor.h"
#include "Task2.h"
#include "FBCommonHeaders/LockQueue.h"
namespace fb {
struct TaskProcessor::ClassData {
	std::vector<std::thread> threads;
	LockQueue<Task2Ptr> tasks;
	std::unordered_map<void*, Task2Ptr> children_waiting_tasks;
	SpinLockWaitSleep children_waiting_tasks_guard;
	std::mutex trigger_mutex;
	std::condition_variable trigger;
	bool quit = false;
};

void TaskThread(TaskProcessor* tp) {
	while (!tp->IsQuit()) {
		std::unique_lock<std::mutex> lock(tp->m->trigger_mutex);
		auto task = tp->m->tasks.Deq();
		if (!task) {
			tp->m->trigger.wait(lock);
			task = tp->m->tasks.Deq();
		}
		lock.unlock();
		while (task) {
			task->Execute(tp);
			task->OnExecuted(tp);		
			
			task = tp->m->tasks.Deq();
		}
	}
}

TaskProcessor::TaskProcessor(int numThreads) 
	:m(new ClassData)
{
	m->threads.reserve(numThreads);
	// create threads
	for (int i = 0; i < numThreads; ++i) {
		m->threads.push_back(std::thread(TaskThread, this));
	}
}

TaskProcessor::~TaskProcessor() {
	PrepareQuit();
	for (auto& t : m->threads) {
		t.join();
	}
}

void TaskProcessor::PrepareQuit() {
	m->quit = true;
	m->trigger.notify_all();
}

bool TaskProcessor::IsQuit() {
	return m->quit;
}

void TaskProcessor::AddTask(Task2Ptr task) {
	{
		std::lock_guard<std::mutex> lock(m->trigger_mutex);
		m->tasks.Enq(task);
	}
	m->trigger.notify_all();
}

void TaskProcessor::CheckChildrenWaitingTasks(void* sync_address){
	EnterSpinLock<SpinLockWaitSleep> lock(m->children_waiting_tasks_guard);
	auto it = m->children_waiting_tasks.find(sync_address);
	if (it != m->children_waiting_tasks.end()) {
		it->second->OnFinish(this);
		m->children_waiting_tasks.erase(it);
	}
	else {
		Logger::Log(FB_ERROR_LOG_ARG, "sync_addresss is invalid.");
	}
}

void TaskProcessor::AddChildrenWaitingTask(void* sync_address, Task2Ptr task) {
	EnterSpinLock<SpinLockWaitSleep> lock(m->children_waiting_tasks_guard);
	m->children_waiting_tasks[sync_address] = task;
}

}