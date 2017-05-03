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
#include <string>
#include <chrono>
#include <functional>
#include "Types.h"
namespace fb {
	class ProfilerSimple {
		std::string mName;
		std::function<void(const char*, INT64)> mCallbackFunction;

		void* mPtr;
		std::function<void(void*, INT64)> mCallbackFunction2;

		INT64 mStartTick;				
		mutable TIME_PRECISION mPrevDT;

	public:
		ProfilerSimple(std::string name, std::function<void(const char*, INT64)> callback = {})
			: mName(name)
			, mCallbackFunction(callback)
			, mPtr(0)
		{
			using namespace std::chrono;
			mStartTick = duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
		}

		ProfilerSimple(void* ptr, std::function<void(void*, INT64)> callback = {})
			: mPtr(ptr)
			, mCallbackFunction2(callback)
		{
			using namespace std::chrono;
			mStartTick = duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
		}

		~ProfilerSimple()
		{
			if (mCallbackFunction) {
				mCallbackFunction(mName.c_str(), GetDTMicro());
			}
			else if (mCallbackFunction2) {
				mCallbackFunction2(mPtr, GetDTMicro());
			}			
		}

		TIME_PRECISION GetDT() const {
			using namespace std::chrono;
			TIME_PRECISION dt = 
				(duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count() - mStartTick) / (TIME_PRECISION)std::micro::den;
			mPrevDT = (dt + mPrevDT) * .5f;
			return mPrevDT;
		}

		INT64 GetDTMicro() const {
			using namespace std::chrono;
			return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count() - mStartTick;
		}

		const char* GetName() const {
			return mName.c_str();
		}

		void Reset() {
			using namespace std::chrono;
			mStartTick = duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
		}
	};
}
