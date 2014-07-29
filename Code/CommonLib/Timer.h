#pragma once

#include "SmartPtr.h"

namespace fastbird
{
	class Timer : public ReferenceCounter
	{
	public:
		Timer();	
		~Timer();

	public:
		typedef float TIME_PRECISION;
		typedef unsigned int FRAME_PRECISION; // unsigned int : safe for 828 'days' at 60 frames/sec
											  // unsigned long long : safe for 9749040289 'years' at 60 frames/sec

		void Tick();
		TIME_PRECISION GetTime();
		TIME_PRECISION GetDeltaTime();		
		void Reset();
		__int64 GetTickCount();
		__int64 GetFreq() const{ return mFreq.QuadPart; }
		FRAME_PRECISION GetFrame() const { return mFrames; }


	private:
		LARGE_INTEGER mFreq;
		LARGE_INTEGER mBase;
		TIME_PRECISION mStartTime;
		TIME_PRECISION mDeltaTime;
		TIME_PRECISION mTime;
		FRAME_PRECISION mFrames;
	};
	extern Timer* gpTimer;
}