#pragma once
#include <CommonLib/threads.h>
using namespace fastbird;
class QuickSortTask : public Task
{
	int* Data;
	int Start;
	int End;

	static FORCEINLINE void swap(int *a, int *b)
	{
		int t = *a;
		*a = *b;
		*b = t;
	}

	static void qsort(int* Data, int Start, int End)
	{
		if (End > Start + 1)
		{
			int piv = Data[Start], l = Start + 1, r = End;
			while (l < r)
			{
				if (Data[l] <= piv)
					l++;
				else
					swap(&Data[l], &Data[--r]);
			}

			swap(&Data[--l], &Data[Start]);

			qsort(Data, Start, l);
			qsort(Data, r, End);
		}
	}

public:
	QuickSortTask(int* _Data, int _Start, int _End, volatile long* ExecCounter)
		: Task(!!ExecCounter, !ExecCounter, ExecCounter),
		Data(_Data),
		Start(_Start),
		End(_End)
	{
		if (ExecCounter)
		{
			InterlockedIncrement(ExecCounter);
		}
	}

	~QuickSortTask()
	{}

	void Execute(TaskScheduler* Parent)
	{
		if ((End - Start) < 65536)
		{
			// handle in one thread
			qsort(Data, Start, End);
		}
		else
		{
			int piv = Data[Start], l = Start + 1, r = End;

			// group values with pivot. left is for smaller or equal to pivot, right side is for bigger.
			while (l < r)
			{
				if (Data[l] <= piv)
					l++;
				else
					swap(&Data[l], &Data[--r]);
			}

			swap(&Data[--l], &Data[Start]);

			// left side is available for sort.
			if (l > Start + 1)
			{
				QuickSortTask* SubTask0 = new QuickSortTask(Data, Start, l, mExecCounter ? mExecCounter : &mSyncCounter);

				if (Parent)
				{
					// Parallel version
					Parent->AddTask(SubTask0);
				}
				else
				{
					// Serial version
					SubTask0->Execute(NULL);
				}
			}

			if (End > r + 1)
			{
				QuickSortTask* SubTask1 = new QuickSortTask(Data, r, End, mExecCounter ? mExecCounter : &mSyncCounter);

				if (Parent)
				{
					// Parallel version
					Parent->AddTask(SubTask1);
				}
				else
				{
					// Serial version
					SubTask1->Execute(NULL);
				}
			}
		}
	}
};
