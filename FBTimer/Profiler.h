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
#include <stack>
#include "FBCommonHeaders/platform.h"
#include "Timer.h"
namespace fb
{
	class FB_DLL_TIMER Profiler
	{
		static int indent;
		typedef std::stack<std::string> MSG_STACK;
		static  MSG_STACK msgs;
		std::string mName;
		INT64 mStartTick;
		TIME_PRECISION* mAccumulator;		

	public:
		Profiler(const char* name);
		Profiler(const char* name, TIME_PRECISION* accumulator);
		~Profiler();

		void SetAccumulator(TIME_PRECISION* p);
		TIME_PRECISION GetDt() const;
		void Reset();	
	};

	class FB_DLL_TIMER ProfilerSimple
	{
		const char* mName;
		INT64 mStartTick;
		mutable TIME_PRECISION mPrevDT;

	public:
		ProfilerSimple(const char* name);
		TIME_PRECISION GetDT() const;
		const char* GetName() const;
		void Reset();		
	};
}