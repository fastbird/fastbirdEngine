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
#include "Profiler.h"
#include "FBCommonHeaders/platform.h"
#include <stdarg.h>
#include <fstream>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#endif

namespace fb
{
	static std::ofstream profileLogFile("fb_profile_result.log");
	int Profiler::indent = 0;
	Profiler::MSG_STACK Profiler::msgs;

	static void DebugOutput(const char* format, ...){
		va_list args;
		va_start(args, format);
		char buffer[2048];
		vsprintf_s(buffer, format, args);
		va_end(args);
#if defined(_PLATFORM_WINDOWS_)
		OutputDebugString(buffer);
#else
		assert(0 && "Not implemented");
#endif
	}

	Profiler::Profiler(const char* name)
		: mName(name)
		, mAccumulator(0)
	{
		mStartTick = gpTimer->GetTickCount();
		indent++;
	}

	Profiler::Profiler(const char* name, TIME_PRECISION* accumulator)
		: mName(name)
		, mAccumulator(accumulator)
	{
		mStartTick = gpTimer->GetTickCount();
		indent++;
	}

	void Profiler::SetAccumulator(TIME_PRECISION* p)
	{
		mAccumulator = p;
	}

	Profiler::~Profiler()
	{
		indent--;
		TIME_PRECISION elapsedTime = (gpTimer->GetTickCount() - mStartTick) / (TIME_PRECISION)std::milli::den;
		char sp[10];
		memset(sp, '\t', 10);
		if (indent > 9)
			sp[9] = 0;
		else
			sp[indent] = 0;
		if (indent != 0)
		{
			char buffer[255];
			sprintf_s(buffer, 255, "%s[Profiler] %s takes %f secs.\n", sp, mName.c_str(), elapsedTime);
			msgs.push(buffer);
		}
		else
		{
			DebugOutput("%s[Profiler] %s takes %f secs.\n", sp, mName.c_str(), elapsedTime);
			// print stack
			while (!msgs.empty())
			{
				DebugOutput(msgs.top().c_str());
				if (profileLogFile.is_open()){
					profileLogFile << msgs.top().c_str();
					profileLogFile.flush();

				}
				msgs.pop();
			}
		}
		if (mAccumulator)
			*mAccumulator += elapsedTime;		
	}

	TIME_PRECISION Profiler::GetDt() const{
		return (gpTimer->GetTickCount() - mStartTick) / (TIME_PRECISION)std::milli::den;
	}

	void Profiler::Reset(){
		mStartTick = gpTimer->GetTickCount();
	}

	//-----------------------------------------------------------------------
	// Profile simple
	ProfilerSimple::ProfilerSimple(const char* name)
		:mName(name)
	{
		mStartTick = gpTimer->GetTickCount();
	}

	TIME_PRECISION ProfilerSimple::GetDT() const
	{
		TIME_PRECISION dt = (gpTimer->GetTickCount() - mStartTick) / (TIME_PRECISION)std::milli::den;
		mPrevDT = (dt + mPrevDT) * .5f;
		return mPrevDT;
	}

	const char* ProfilerSimple::GetName() const
	{
		return mName;
	}

	void ProfilerSimple::Reset()
	{
		mStartTick = gpTimer->GetTickCount();
	}
}