#pragma once
#include "FBCommonHeaders/Types.h"
namespace fb {
	class ThreadSafeCounter;
	class TaskProcessor;
	FB_DECLARE_SMART_PTR(Task2);
	class FB_DLL_THREAD Task2 : public std::enable_shared_from_this<Task2> {
		friend class TaskProcessor;
		friend void TaskThread(TaskProcessor* tp);		

		void OnExecuted(TaskProcessor* tp);
		void OnFinish(TaskProcessor* tp);

	protected:
		struct Task2Data;
		Task2Data* m;
		void AddChild(TaskProcessor* tp, Task2Ptr child);		

	public:
		Task2(bool wait = false);
		virtual void Execute(TaskProcessor* tp) = 0;		
		
		void Wait();
		void Reset();
		
	};
}
