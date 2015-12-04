/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#include "stdafx.h"
#include "Timer.h"

using namespace std::chrono;
namespace fb
{
	Timer* gpTimer = 0;
	class Timer::Impl{
	public:
		std::chrono::high_resolution_clock::time_point mPreviousTimePoint;
		FRAME_PRECISION mFrames;
		TIME_PRECISION mTime;
		TIME_PRECISION mTimeNotPausable;
		TIME_PRECISION mDeltaTime;
		TIME_PRECISION mDeltaTimeNotPausable;
		bool mPaused;

		//---------------------------------------------------------------------------
		Impl()
			: mFrames(0)
			, mPaused(false){
			Reset();
		}

		void Tick()
		{
			auto now = high_resolution_clock::now();
			std::chrono::duration<TIME_PRECISION> diff = now - mPreviousTimePoint;
			mPreviousTimePoint = now;
			mTimeNotPausable += diff.count();
			mDeltaTimeNotPausable = diff.count();
			mTimeNotPausable += mDeltaTimeNotPausable;

			++mFrames;
			if (mPaused)
				return;

			TIME_PRECISION previousTime = mTime;
			mTime += mDeltaTimeNotPausable;
			mDeltaTime = mDeltaTimeNotPausable;
		}

		void Reset()
		{
			mPreviousTimePoint = high_resolution_clock::now();
			mTime = 0;
			mDeltaTime = 0;
			mTimeNotPausable = 0;
			mDeltaTimeNotPausable = 0;
		}

		TIME_PRECISION GetDeltaTime() const
		{
			return mDeltaTime < 0.4f ? mDeltaTime : 0.4f;
		}

		TIME_PRECISION GetDeltaTimeNotPausable() const
		{
			return mDeltaTimeNotPausable < 0.4f ? mDeltaTimeNotPausable : 0.4f;
		}

		void SetDeltaTime(TIME_PRECISION deltaTime){
			mDeltaTimeNotPausable = deltaTime;
			if (!mPaused)
				mDeltaTime = deltaTime;
		}

		TIME_PRECISION GetTime() const
		{
			return mTime;
		}

		void SetTime(TIME_PRECISION time){
			mTimeNotPausable = time;
			if (!mPaused)
				mTime = time;
		}

		FRAME_PRECISION GetFrame() const{
			return mFrames;
		}

		INT64 GetTickCount() const
		{
			return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
		}

		void Pause()
		{
			mPaused = true;
			mDeltaTime = 0.f;
		}

		void Resume()
		{
			mPaused = false;
		}

		bool IsPaused() const {
			return mPaused;
		}
	};

	//---------------------------------------------------------------------------
	TimerPtr sTimer;
	TimerPtr Timer::Create(){
		auto timer = TimerPtr(new Timer, [](Timer* obj){ delete obj; });
		if (!sTimer){
			sTimer = timer;
			gpTimer = timer.get();
		}
		return timer;
	}

	TimerPtr Timer::GetMainTimer(){
		if (!sTimer){
			Create();
		}
		return sTimer;
	}

	Timer::Timer()
		: mImpl(new Impl)
	{
	}

	Timer::~Timer()
	{
	}

	void Timer::Tick()
	{
		mImpl->Tick();
	}

	void Timer::Reset()
	{
		mImpl->Reset();
	}

	TIME_PRECISION Timer::GetDeltaTime() const
	{
		return mImpl->GetDeltaTime();
	}

	TIME_PRECISION Timer::GetDeltaTimeNotPausable() const
	{
		return mImpl->GetDeltaTimeNotPausable();
	}

	void Timer::SetDeltaTime(TIME_PRECISION deltaTime){
		mImpl->SetDeltaTime(deltaTime);
	}

	TIME_PRECISION Timer::GetTime() const
	{
		return mImpl->GetTime();
	}

	void Timer::SetTime(TIME_PRECISION time){
		mImpl->SetTime(time);
	}

	FRAME_PRECISION Timer::GetFrame() const{
		return mImpl->GetFrame();
	}

	INT64 Timer::GetTickCount() const
	{
		return mImpl->GetTickCount();
	}

	INT64 Timer::GetFrequency() const{
		return std::milli::den;
	}

	void Timer::Pause()
	{
		mImpl->Pause();
	}

	void Timer::Resume()
	{
		mImpl->Resume();
	}

	bool Timer::IsPaused() const{
		return mImpl->IsPaused();
	}
}