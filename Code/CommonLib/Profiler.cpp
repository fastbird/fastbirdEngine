#include <CommonLib/StdAfx.h>
#include "Profiler.h"
#include "Timer.h"
#include "Debug.h"

namespace fastbird
{
	std::fstream profileLogFile("profile_result.txt", std::fstream::out);
	int Profiler::indent = 0;
	Profiler::MSG_STACK Profiler::msgs;

	Profiler::Profiler(const char* name, float* accumulator)
		: mName(name)
		, mAccumulator(accumulator)
	{
		mStartTick = gpTimer->GetTickCount();
		indent++;
	}

	void Profiler::SetAccumulator(float* p)
	{
		mAccumulator = p;
	}

	Profiler::~Profiler()
	{
		indent--;
		float elapsedTime = (float)(gpTimer->GetTickCount() - mStartTick) / (float)gpTimer->GetFreq();
		char sp[10];
		memset(sp, '\t', 10);
		if (indent > 9)
			sp[9] = 0;
		else
			sp[indent] = 0;
		if (indent != 0)
		{
			char buffer[255];
			sprintf_s(buffer, 255, "%s[Profiler] %s takes %f secs.", sp, mName.c_str(), elapsedTime);
			msgs.push(buffer);
		}
		else
		{
			DebugOutput("%s[Profiler] %s takes %f secs.", sp, mName.c_str(), elapsedTime);
			// print stack
			while (!msgs.empty())
			{
				DebugOutput(msgs.top().c_str());
				assert(profileLogFile.is_open());
				profileLogFile << msgs.top().c_str();
				msgs.pop();
			}
		}
		if (mAccumulator)
			*mAccumulator += elapsedTime;
	}

	float Profiler::GetDt(){
		return (float)(gpTimer->GetTickCount() - mStartTick) / (float)gpTimer->GetFreq();
	}

	void Profiler::Reset(){
		mStartTick = gpTimer->GetTickCount();
	}

	//-----------------------------------------------------------------------
	// Profile simple
	ProfilerSimple::ProfilerSimple(const wchar_t* name)
		:mName(name)
	{
		mStartTick = gpTimer->GetTickCount();
	}

	float ProfilerSimple::GetDT()
	{
		float dt = (float)(gpTimer->GetTickCount() - mStartTick) / (float)gpTimer->GetFreq();
		mPrevDT = (dt + mPrevDT)* .5f;
		return mPrevDT;
	}

	const wchar_t* ProfilerSimple::GetName() const
	{
		return mName;
	}

	void ProfilerSimple::Reset()
	{
		mStartTick = gpTimer->GetTickCount();
	}
}