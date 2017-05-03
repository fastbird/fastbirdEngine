#pragma once
namespace fb {	
	FB_DECLARE_SMART_PTR(Task2);
	FB_DECLARE_SMART_PTR(TaskProcessor);
	class FB_DLL_THREAD TaskProcessor {
		friend void TaskThread(TaskProcessor* taskProcessor);
		friend class Task2;
		struct ClassData;
		ClassData *m;
		
		void CheckChildrenWaitingTasks(void* sync_address);
		void AddChildrenWaitingTask(void* sync_address, Task2Ptr task);
	public:
		TaskProcessor(int numThreads);
		~TaskProcessor();
		void PrepareQuit();
		bool IsQuit();
		void AddTask(Task2Ptr task);		
	};
}
