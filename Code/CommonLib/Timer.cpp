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

	void Timer::FinishSmartPtr(){
		FB_DELETE(this);
	}

	void Timer::Tick()
	{
		mDeltaTimeNotPausable = (GetTickCount() / (TIME_PRECISION)mFreq.QuadPart - mStartTime) - mTimeNotPausable;
		mTimeNotPausable += mDeltaTimeNotPausable;
		
		if (mPaused)
			return;
		++mFrames;
		float previousTime = mTime;
		mTime += mDeltaTimeNotPausable;
		mDeltaTime = mDeltaTimeNotPausable;
	}

	Timer::TIME_PRECISION Timer::GetDeltaTime()
	{
		return mDeltaTime < 0.4f ? mDeltaTime : 0.4f;
	}

	Timer::TIME_PRECISION Timer::GetDeltaTimeNotPausable()
	{
		return mDeltaTimeNotPausable < 0.4f ? mDeltaTimeNotPausable : 0.4f;
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
		mTimeNotPausable = 0;
		mDeltaTimeNotPausable = 0;
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