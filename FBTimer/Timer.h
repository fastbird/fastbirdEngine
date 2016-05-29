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

#pragma once
#include "FBCommonHeaders/platform.h"
#include <memory>
#include <chrono>
namespace fb
{
	FB_DECLARE_SMART_PTR(Timer);
	class FB_DLL_TIMER Timer
	{
		FB_DECLARE_PIMPL(Timer);
		Timer();

	public:
		static TimerPtr Create();
		static TimerPtr GetMainTimer();		
		~Timer();		

		void Tick();
		void Reset();
		/**Get current game time in seconds.
		Calling this function in the same frame will return
		the same value.
		*/
		TIME_PRECISION GetTime() const;
		/** Set game time in seconds.
		If you are handling time manually rather than using Timer::Tick() function, 
		use this function to set the current game time.
		*/
		void SetTime(TIME_PRECISION time);
		TIME_PRECISION GetDeltaTime() const;
		TIME_PRECISION GetDeltaTimeNotPausable() const;		
		/** Set delta time in secs.
		If you are handling time manually rather than using Timer::Tick() function, 
		use this function to set the delta time.
		*/
		void SetDeltaTime(TIME_PRECISION deltaTime);
		FRAME_PRECISION GetFrame() const;
		/// milliseconds
		INT64 GetTickCount() const;
		/// seconds from epoch 00:00:00 UTC, Thursday, 1, January, 1970
		time_t GetPosixTime() const;
		INT64 GetFrequency() const;

		void Pause();
		void Resume();
		bool IsPaused() const;
		
	};
	/// only declaration. you need to define it.
	extern Timer* gpTimer;
}