#include <CommonLib/StdAfx.h>
#include "Timer.h"

namespace fastbird
{
	Timer* gpTimer = 0;

	Timer::Timer()
		: mFrames(0)
		, mPaused(false)
	{
		bool support = QueryPerformanceFrequency(&mFreq)!=0;
		if(!support)
		{
			mFreq.QuadPart = 0;
			std::cerr << "PerformanceCounter is not supported.";
		}
		Reset();
	}

	Timer::~Timer()
	{
	}

	void Timer::Tick()
	{
		if (mPaused)
			return;
		++mFrames;
		float previousTime = mTime;
		mTime = GetTickCount() / (TIME_PRECISION)mFreq.QuadPart - mStartTime;
		mDeltaTime = mTime - previousTime;
	}

	Timer::TIME_PRECISION Timer::GetDeltaTime()
	{
		return mDeltaTime < 0.4f ? mDeltaTime : 0.4f;
	}

	Timer::TIME_PRECISION Timer::GetTime()
	{
		return mTime;
	}

	void Timer::Reset()
	{
		QueryPerformanceCounter( &mBase );
		mStartTime = mBase.QuadPart / (TIME_PRECISION)mFreq.QuadPart;
		mTime = 0;
		mDeltaTime = 0;
	}

	__int64 Timer::GetTickCount()
	{
		LARGE_INTEGER newCount;
		QueryPerformanceCounter(&newCount);
		return newCount.QuadPart;
	}

	void Timer::Pause()
	{
		mPaused = true;
		mDeltaTime = 0.f;
	}

	void Timer::Resume()
	{
		mPaused = false;
	}
}