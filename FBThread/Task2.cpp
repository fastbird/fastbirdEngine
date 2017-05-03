#include "stdafx.h"
#include "Task2.h"
#include "AsyncObjects.h"
#include "TaskProcessor.h"
namespace fb {
struct Task2::Task2Data {
	SyncEventPtr wait_event;	
	ThreadSafeCounter* children_counter = nullptr;
	ThreadSafeCounter children_counter_source;	
};

Task2::Task2(bool wait)
	: m(new Task2Data)
{
	if (wait) {
		m->wait_event = CreateSyncEvent(true);
	}	
}

void Task2::OnExecuted(TaskProcessor* tp) {
	if (m->children_counter_source == 0) {
		OnFinish(tp);		
	}
	else {
		tp->AddChildrenWaitingTask(&m->children_counter_source, shared_from_this());
	}
}

void Task2::OnFinish(TaskProcessor* tp) {
	if (m->children_counter) {
		auto counter = --(*m->children_counter);
		if (counter == 0) {
			tp->CheckChildrenWaitingTasks(m->children_counter);
		}
	}

	if (m->wait_event)
		m->wait_event->Trigger();
}

void Task2::AddChild(TaskProcessor* tp, Task2Ptr child) {
	child->m->children_counter = &m->children_counter_source;
	++(*child->m->children_counter);
	tp->AddTask(child);
}

void Task2::Wait() {
	if (m->wait_event)
		m->wait_event->Wait();
}

void Task2::Reset() {
	if (m->wait_event) {
		m->wait_event->Trigger();
		m->wait_event->Reset();
	}

	if (m->children_counter_source != 0) {
		Logger::Log(FB_ERROR_LOG_ARG, "children_counter_source is not zero. Rest failed.");
	}
}

}