#pragma once

#include "SmartPtr.h"

namespace fastbird
{
	class Timer : public ReferenceCounter
	{
	public:
		Timer();	
		~Timer();

	protected:
		virtual void FinishSmartPtr();

	public:
		typedef float TIME_PRECISION;
		typedef unsigned int FRAME_PRECISION; // unsigned int : safe for 828 'days' at 60 frames/sec
											  // unsigned long long : safe for 9749040289 'years' at 60 frames/sec

		void Tick();
		TIME_PRECISION GetTime();
		TIME_PRECISION GetDeltaTime();
		TIME_PRECISION GetDeltaTimeNotPausable();
		void Reset();
		__int64 GetTickCount();
		__int64 GetFreq() const{ return mFreq.QuadPart; }
		FRAME_PRECISION GetFrame() const { return mFrames; }

		void Pause();
		void Resume();
		bool IsPause() const { return mPaused; }


	private:
		LARGE_INTEGER mFreq;
		LARGE_INTEGER mBase;
		TIME_PRECISION mStartTime;
		TIME_PRECISION mDeltaTime;
		TIME_PRECISION mDeltaTimeNotPausable;
		TIME_PRECISION mTimeNotPausable;
		TIME_PRECISION mTime;
		FRAME_PRECISION mFrames;
		bool mPaused;
	};
	extern Timer* gpTimer;
}