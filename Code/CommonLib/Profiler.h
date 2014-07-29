#pragma once
#include <stack>
namespace fastbird
{
	class Profiler
	{
	public:
		Profiler(const char* name, float* accumulator=0);
		void SetAccumulator(float* p);
		virtual ~Profiler();

	protected:
		static int indent;
		typedef std::stack<std::string> MSG_STACK;
		static  MSG_STACK msgs;
		std::string mName;
		__int64 mStartTick;
		float* mAccumulator;
		
	};
}